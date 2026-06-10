#include "D3D11/D3D11EditorOverlayRenderer.h"
#include "D3D11/D3D11DynamicRHI.h"
#include "D3D11/D3D11MeshPassRenderer.h"
#include "Math/MathConstants.h"
#include "Math/MathUtility.h"
#include "SceneView.h"
#include "Viewport/EditorViewportClient.h"
#include <cassert>
#include <cmath>

namespace
{
    constexpr float FullAlpha = 1.0f;

    const FColor GridMinorLineColor(0.15f, 0.15f, 0.15f, FullAlpha);
    const FColor GridMajorLineColor(0.70f, 0.70f, 0.70f, FullAlpha);

    const FColor WorldXAxisColor(1.0f, 0.0f, 0.0f, FullAlpha);
    const FColor WorldYAxisColor(0.0f, 1.0f, 0.0f, FullAlpha);
    const FColor WorldZAxisColor(0.0f, 0.0f, 1.0f, FullAlpha);

    const FColor GizmoXAxisColor(1.0f, 0.15f, 0.15f, FullAlpha);
    const FColor GizmoYAxisColor(0.15f, 1.0f, 0.15f, FullAlpha);
    const FColor GizmoZAxisColor(0.25f, 0.55f, 1.0f, FullAlpha);

    constexpr INT GridDepthBias = 16;
} // namespace

namespace
{
    FColor BoostColor(const FColor &InColor, float InBoost)
    {
        return FColor((std::min)(1.0f, InColor.R + InBoost), (std::min)(1.0f, InColor.G + InBoost),
                      (std::min)(1.0f, InColor.B + InBoost), InColor.A);
    }

    FColor ResolveAxisColor(const FGizmoState &InGizmoState, EGizmoAxis InAxis,
                            const FColor &InBase)
    {
        if (InGizmoState.ActiveAxis == InAxis)
        {
            return BoostColor(InBase, 0.45f);
        }

        if (InGizmoState.HighlightedAxis == InAxis)
        {
            return BoostColor(InBase, 0.25f);
        }

        return InBase;
    }
} // namespace

namespace
{
    constexpr int32 AxisRefineSteps = 24;
    constexpr float AxisInitialSearchLength = 1000.0f;

    FVector FindAxisVisibilityBoundary(const FSceneView &View, const FVector &Origin,
                                       const FVector &AxisDirection, float VisibleT,
                                       float NonVisibleT)
    {
        FVector Dir = AxisDirection;
        Dir.Normalize();

        float VisibleBoundT = VisibleT;
        float NonVisibleBoundT = NonVisibleT;

        // Origin + Dir * VisibleBoundT    는 visible
        // Origin + Dir * NonVisibleBoundT 는 non-visible
        for (int32 Step = 0; Step < AxisRefineSteps; ++Step)
        {
            const float MidT = 0.5f * (VisibleBoundT + NonVisibleBoundT);

            if (View.IsWorldPointVisible(Origin + Dir * MidT))
            {
                VisibleBoundT = MidT;
            }
            else
            {
                NonVisibleBoundT = MidT;
            }
        }

        return Origin + Dir * VisibleBoundT;
    }

    FVector FindVisibleEndOfAxis(const FSceneView &View, const FVector &Origin,
                                 const FVector &AxisDirection)
    {
        FVector Dir = AxisDirection;
        Dir.Normalize();

        float VisibleT = 0.0f;
        float NonVisibleT = AxisInitialSearchLength;

        // visible / non-visible 구간 찾기
        for (int32 Expand = 0; Expand < 12; ++Expand)
        {
            if (!View.IsWorldPointVisible(Origin + Dir * NonVisibleT))
            {
                break;
            }

            VisibleT = NonVisibleT;
            NonVisibleT *= 2.0f;
        }

        return FindAxisVisibilityBoundary(View, Origin, Dir, VisibleT, NonVisibleT);
    }

    FVector FindVisibleStartOfAxis(const FSceneView &View, const FVector &Origin,
                                   const FVector &AxisDirection)
    {
        FVector Dir = AxisDirection;
        Dir.Normalize();

        if (View.IsWorldPointVisible(Origin))
        {
            return Origin;
        }

        float NonVisibleT = 0.0f;
        float VisibleT = AxisInitialSearchLength;

        // non-visible / visible 구간 찾기
        for (int32 Expand = 0; Expand < 12; ++Expand)
        {
            if (View.IsWorldPointVisible(Origin + Dir * VisibleT))
            {
                break;
            }

            NonVisibleT = VisibleT;
            VisibleT *= 2.0f;
        }

        return FindAxisVisibilityBoundary(View, Origin, Dir, VisibleT, NonVisibleT);
    }

    void GetGizmoAxes(const FGizmoState &InGizmoState, FVector &OutX, FVector &OutY, FVector &OutZ)
    {
        OutX = FVector(1.0f, 0.0f, 0.0f);
        OutY = FVector(0.0f, 1.0f, 0.0f);
        OutZ = FVector(0.0f, 0.0f, 1.0f);

        if (InGizmoState.Space == EGizmoSpace::Local)
        {
            const float YawRad = InGizmoState.Rotation.Yaw * Math::Pi / 180.0f;
            const float PitchRad = InGizmoState.Rotation.Pitch * Math::Pi / 180.0f;
            const float RollRad = InGizmoState.Rotation.Roll * Math::Pi / 180.0f;

            const float SY = std::sin(YawRad);
            const float CY = std::cos(YawRad);
            const float SP = std::sin(PitchRad);
            const float CP = std::cos(PitchRad);
            const float SR = std::sin(RollRad);
            const float CR = std::cos(RollRad);

            OutX = FVector(CY * CR + SY * SP * SR, -CP * SR, -SY * CR + CY * SP * SR);
            OutY = FVector(CY * SR - SY * SP * CR, CP * CR, -SY * SR - CY * SP * CR);
            OutZ = FVector(CP * SY, SP, CP * CY);
        }
    }
} // namespace

struct FDebugLineVertex
{
    FVector Position;
    FColor  Color;
};

struct FDebugTriangleVertex
{
    FVector Position;
    FColor  Color;
};

void FD3D11EditorOverlayRenderer::Initialize(FD3D11DynamicRHI       *InRHI,
                                             FD3D11MeshPassRenderer *InMeshRenderer)
{
    RHI = InRHI;
    MeshRenderer = InMeshRenderer;

    D3D11_DEPTH_STENCIL_DESC DepthDesc = {};
    DepthDesc.DepthEnable = FALSE;
    DepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    DepthDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    DepthDesc.StencilEnable = FALSE;

    HRESULT Hr = RHI->GetDevice()->CreateDepthStencilState(&DepthDesc, &OverlayDepthDisabledState);
    assert(SUCCEEDED(Hr));

    D3D11_RASTERIZER_DESC RasterizerDesc = {};
    RasterizerDesc.FillMode = D3D11_FILL_SOLID;
    RasterizerDesc.CullMode = D3D11_CULL_BACK;
    RasterizerDesc.FrontCounterClockwise = FALSE;
    RasterizerDesc.DepthBias = GridDepthBias;
    RasterizerDesc.DepthBiasClamp = 0.0f;
    RasterizerDesc.SlopeScaledDepthBias = 1.0f;
    RasterizerDesc.DepthClipEnable = TRUE;
    RasterizerDesc.ScissorEnable = FALSE;
    RasterizerDesc.MultisampleEnable = FALSE;
    RasterizerDesc.AntialiasedLineEnable = FALSE;

    Hr = RHI->GetDevice()->CreateRasterizerState(&RasterizerDesc, &GridDepthBiasedRasterizerState);
    assert(SUCCEEDED(Hr));

    /** 미리 메모리 할당 */
    CreateLineBuffer(CurrentMaxLineNum);
    CreateTriangleBuffer(CurrentMaxTriangleNum);
}

void FD3D11EditorOverlayRenderer::Shutdown()
{
    MeshRenderer = nullptr;
    RHI = nullptr;

    if (OverlayDepthDisabledState)
    {
        OverlayDepthDisabledState->Release();
        OverlayDepthDisabledState = nullptr;
    }

    if (GridDepthBiasedRasterizerState)
    {
        GridDepthBiasedRasterizerState->Release();
        GridDepthBiasedRasterizerState = nullptr;
    }

    if (LineBuffer)
    {
        LineBuffer->Release();
        LineBuffer = nullptr;
    }

    if (TriangleBuffer)
    {
        TriangleBuffer->Release();
        TriangleBuffer = nullptr;
    }
}

void FD3D11EditorOverlayRenderer::DrawGrid(const FSceneView &View, float HalfExtent, float Spacing)
{
    const int32 LineCount = static_cast<int32>((HalfExtent * 2.0f) / Spacing);

    for (int32 Index = 0; Index <= LineCount; ++Index)
    {
        const float   Coord = -HalfExtent + Index * Spacing;
        const bool    bMajorLine = (static_cast<int32>(std::fabs(Coord)) % 500) == 0;
        const FColor &LineColor = bMajorLine ? GridMajorLineColor : GridMinorLineColor;

        DrawLine(View, FVector(-HalfExtent, 0.f, Coord), FVector(HalfExtent, 0.f, Coord),
                 LineColor, true);

        DrawLine(View, FVector(Coord, 0.f, -HalfExtent), FVector(Coord, 0.f, HalfExtent),
                 LineColor, true);
    }
}

void FD3D11EditorOverlayRenderer::DrawWorldAxes(const FSceneView &View)
{
    const FVector Origin(0.0f, 0.0f, 0.0f);

    const FVector XAxis(1.0f, 0.0f, 0.0f);
    const FVector YAxis(0.0f, 1.0f, 0.0f);
    const FVector ZAxis(0.0f, 0.0f, 1.0f);

    constexpr float AxisRadius = 2.0f;

    const bool bOriginVisible = View.IsWorldPointVisible(Origin);

    if (bOriginVisible)
    {
        const FVector XEnd = FindVisibleEndOfAxis(View, Origin, XAxis);
        const FVector YEnd = FindVisibleEndOfAxis(View, Origin, YAxis);
        const FVector ZEnd = FindVisibleEndOfAxis(View, Origin, ZAxis);

        DrawCylinderAxis(View, Origin, XAxis, (XEnd - Origin).Length(), AxisRadius,
                         WorldXAxisColor);
        DrawCylinderAxis(View, Origin, YAxis, (YEnd - Origin).Length(), AxisRadius,
                         WorldYAxisColor);
        DrawCylinderAxis(View, Origin, ZAxis, (ZEnd - Origin).Length(), AxisRadius,
                         WorldZAxisColor);
    }
    else
    {
        const FVector XStart = FindVisibleStartOfAxis(View, Origin, XAxis);
        const FVector YStart = FindVisibleStartOfAxis(View, Origin, YAxis);
        const FVector ZStart = FindVisibleStartOfAxis(View, Origin, ZAxis);

        const FVector XEnd = FindVisibleEndOfAxis(View, XStart, XAxis);
        const FVector YEnd = FindVisibleEndOfAxis(View, YStart, YAxis);
        const FVector ZEnd = FindVisibleEndOfAxis(View, ZStart, ZAxis);

        DrawCylinderAxis(View, XStart, XAxis, (XEnd - XStart).Length(), AxisRadius,
                         WorldXAxisColor);
        DrawCylinderAxis(View, YStart, YAxis, (YEnd - YStart).Length(), AxisRadius,
                         WorldYAxisColor);
        DrawCylinderAxis(View, ZStart, ZAxis, (ZEnd - ZStart).Length(), AxisRadius,
                         WorldZAxisColor);
    }
}

void FD3D11EditorOverlayRenderer::DrawTransformGizmo(const FSceneView  &View,
                                                     const FGizmoState &GizmoState)
{
    if (!GizmoState.bVisible)
    {
        return;
    }

    switch (GizmoState.Mode)
    {
    case EGizmoMode::Translate:
        DrawTranslationGizmo(View, GizmoState);
        break;

    case EGizmoMode::Rotate:
        DrawRotationGizmo(View, GizmoState);
        break;

    case EGizmoMode::Scale:
        DrawScaleGizmo(View, GizmoState);
        break;

    default:
        break;
    }
}

void FD3D11EditorOverlayRenderer::Draw(const FSceneView &View, const FGizmoState &GizmoState)
{
    ID3D11DeviceContext     *Context = RHI->GetDeviceContext();
    D3D11_MAPPED_SUBRESOURCE Mapped;
    UINT                     Stride, Offset;
    FMatrix                  World;

    ID3D11DepthStencilState *PrevDepthState = nullptr;
    UINT                     PrevStencilRef = 0;
    Context->OMGetDepthStencilState(&PrevDepthState, &PrevStencilRef);

    ID3D11RasterizerState *PrevRasterizerState = nullptr;
    Context->RSGetState(&PrevRasterizerState);

    MeshRenderer->PrepareShader();

    /*
        Line Draw
    */
    if (Lines.empty())
    {
        // do nothing
    }
    else
    {
        const uint32 LineVertexNum = Lines.size() * 2;

        // 버퍼 크기 체크 (필요하면 재생성)
        if (LineVertexNum > CurrentMaxLineNum * 2)
        {
            CreateLineBuffer(RoundUpToPowerOfTwo(LineVertexNum));
        }

        auto DrawLineBatch = [&](bool bUseDepthBias, ID3D11DepthStencilState *DepthState,
                                 UINT StencilRef, ID3D11RasterizerState *RasterizerState)
        {
            uint32 BatchLineCount = 0;
            for (const FDebugLine &Line : Lines)
            {
                if (Line.bUseDepthBias == bUseDepthBias)
                {
                    ++BatchLineCount;
                }
            }

            if (BatchLineCount == 0)
            {
                return;
            }

            Context->Map(LineBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped);

            FDebugLineVertex *Dest = (FDebugLineVertex *)Mapped.pData;

            for (const FDebugLine &Line : Lines)
            {
                if (Line.bUseDepthBias != bUseDepthBias)
                {
                    continue;
                }

                Dest->Position = Line.Start;
                Dest->Color = Line.Color;
                Dest++;
                Dest->Position = Line.End;
                Dest->Color = Line.Color;
                Dest++;
            }

            Context->Unmap(LineBuffer, 0);

            Stride = sizeof(FDebugLineVertex);
            Offset = 0;

            Context->IASetVertexBuffers(0, 1, &LineBuffer, &Stride, &Offset);
            Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

            World.SetIdentity();

            MeshRenderer->UpdateConstant(World.GetTranspose(), View.GetViewMatrix().GetTranspose(),
                                         View.GetProjectionMatrix().GetTranspose());

            Context->OMSetDepthStencilState(DepthState, StencilRef);
            Context->RSSetState(RasterizerState);
            Context->Draw(BatchLineCount * 2, 0);
        };

        DrawLineBatch(true, PrevDepthState, PrevStencilRef, GridDepthBiasedRasterizerState);
        DrawLineBatch(false, OverlayDepthDisabledState, 0, PrevRasterizerState);

        Lines.clear();
    }

    Context->OMSetDepthStencilState(OverlayDepthDisabledState, 0);
    Context->RSSetState(PrevRasterizerState);

    /*
        Triangle Draw
    */

    if (Triangles.empty())
    {
        // do nothing
    }
    else
    {
        const uint32 TriangleVertexNum = Triangles.size() * 3;

        if (TriangleVertexNum > CurrentMaxTriangleNum * 3)
        {
            CreateTriangleBuffer(RoundUpToPowerOfTwo(TriangleVertexNum));
        }

        Context->Map(TriangleBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped);

        FDebugTriangleVertex *TriangleDest = (FDebugTriangleVertex *)Mapped.pData;

        for (const FDebugTriangle &Triangle : Triangles)
        {
            for (int32 i = 0; i < 3; i++)
            {
                TriangleDest->Position = Triangle.V[i];
                TriangleDest->Color = Triangle.Color[i];
                TriangleDest++;
            }
        }

        Context->Unmap(TriangleBuffer, 0);

        Stride = sizeof(FDebugTriangleVertex);
        Offset = 0;

        Context->IASetVertexBuffers(0, 1, &TriangleBuffer, &Stride, &Offset);
        Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        World.SetIdentity();

        MeshRenderer->UpdateConstant(World.GetTranspose(), View.GetViewMatrix().GetTranspose(),
                                     View.GetProjectionMatrix().GetTranspose());

        Context->Draw(TriangleVertexNum, 0);

        Triangles.clear();
    }
    Context->OMSetDepthStencilState(PrevDepthState, PrevStencilRef);
    Context->RSSetState(PrevRasterizerState);
    if (PrevRasterizerState)
        PrevRasterizerState->Release();
    if (PrevDepthState)
        PrevDepthState->Release();
}

void FD3D11EditorOverlayRenderer::DrawTranslationGizmo(const FSceneView  &View,
                                                       const FGizmoState &GizmoState)
{
    const FVector Origin = GizmoState.Origin;

    constexpr float ShaftLength = 90.0f;
    constexpr float ShaftRadius = 1.8f;
    constexpr float ConeHeight = 24.0f;
    constexpr float ConeRadius = 5.0f;

    const FColor XColor = ResolveAxisColor(GizmoState, EGizmoAxis::X, GizmoXAxisColor);
    const FColor YColor = ResolveAxisColor(GizmoState, EGizmoAxis::Y, GizmoYAxisColor);
    const FColor ZColor = ResolveAxisColor(GizmoState, EGizmoAxis::Z, GizmoZAxisColor);

    FVector XAxis, YAxis, ZAxis;
    GetGizmoAxes(GizmoState, XAxis, YAxis, ZAxis);

    DrawCylinderAxis(View, Origin, XAxis, ShaftLength, ShaftRadius, XColor);
    DrawConeAxis(View, Origin + XAxis * (ShaftLength + ConeHeight), XAxis, ConeHeight, ConeRadius,
                 XColor);

    DrawCylinderAxis(View, Origin, YAxis, ShaftLength, ShaftRadius, YColor);
    DrawConeAxis(View, Origin + YAxis * (ShaftLength + ConeHeight), YAxis, ConeHeight, ConeRadius,
                 YColor);

    DrawCylinderAxis(View, Origin, ZAxis, ShaftLength, ShaftRadius, ZColor);
    DrawConeAxis(View, Origin + ZAxis * (ShaftLength + ConeHeight), ZAxis, ConeHeight, ConeRadius,
                 ZColor);
}

void FD3D11EditorOverlayRenderer::DrawRotationGizmo(const FSceneView  &View,
                                                    const FGizmoState &GizmoState)
{
    const FVector Origin = GizmoState.Origin;

    constexpr float Radius = 72.0f;
    constexpr int32 Segments = 64;

    const FColor XColor = ResolveAxisColor(GizmoState, EGizmoAxis::X, GizmoXAxisColor);
    const FColor YColor = ResolveAxisColor(GizmoState, EGizmoAxis::Y, GizmoYAxisColor);
    const FColor ZColor = ResolveAxisColor(GizmoState, EGizmoAxis::Z, GizmoZAxisColor);

    FVector XAxis, YAxis, ZAxis;
    GetGizmoAxes(GizmoState, XAxis, YAxis, ZAxis);

    DrawRing(View, Origin, XAxis, Radius, Segments, XColor);
    DrawRing(View, Origin, YAxis, Radius, Segments, YColor);
    DrawRing(View, Origin, ZAxis, Radius, Segments, ZColor);
}

void FD3D11EditorOverlayRenderer::DrawScaleGizmo(const FSceneView  &View,
                                                 const FGizmoState &GizmoState)
{
    const FVector Origin = GizmoState.Origin;

    constexpr float ShaftLength = 90.0f;
    constexpr float ShaftRadius = 1.8f;
    constexpr float HandleLength = 10.0f;
    constexpr float HandleRadius = 4.0f;

    const FColor XColor = ResolveAxisColor(GizmoState, EGizmoAxis::X, GizmoXAxisColor);
    const FColor YColor = ResolveAxisColor(GizmoState, EGizmoAxis::Y, GizmoYAxisColor);
    const FColor ZColor = ResolveAxisColor(GizmoState, EGizmoAxis::Z, GizmoZAxisColor);

    FVector XAxis, YAxis, ZAxis;
    GetGizmoAxes(GizmoState, XAxis, YAxis, ZAxis);

    DrawScaleHandle(View, Origin, XAxis, ShaftLength, ShaftRadius, HandleLength, HandleRadius,
                    XColor);

    DrawScaleHandle(View, Origin, YAxis, ShaftLength, ShaftRadius, HandleLength, HandleRadius,
                    YColor);

    DrawScaleHandle(View, Origin, ZAxis, ShaftLength, ShaftRadius, HandleLength, HandleRadius,
                    ZColor);
}

void FD3D11EditorOverlayRenderer::DrawCylinderAxis(const FSceneView &View, const FVector &Origin,
                                                   const FVector &Direction, float Length,
                                                   float Radius, const FColor &Color)
{
    constexpr int32 Segments = 16;

    FVector Dir = Direction;
    Dir.Normalize();

    FVector Tangent, Bitangent;
    BuildPerpendicularBasis(Dir, Tangent, Bitangent);

    const FVector BaseCenter = Origin;
    const FVector TopCenter = Origin + Dir * Length;

    for (int32 Index = 0; Index < Segments; ++Index)
    {
        const float T0 =
            (2.0f * Math::Pi * static_cast<float>(Index)) / static_cast<float>(Segments);
        const float T1 =
            (2.0f * Math::Pi * static_cast<float>(Index + 1)) / static_cast<float>(Segments);

        const FVector Radial0 = Tangent * std::cos(T0) + Bitangent * std::sin(T0);
        const FVector Radial1 = Tangent * std::cos(T1) + Bitangent * std::sin(T1);

        const FVector P0 = BaseCenter + Radial0 * Radius;
        const FVector P1 = BaseCenter + Radial1 * Radius;
        const FVector P2 = TopCenter + Radial0 * Radius;
        const FVector P3 = TopCenter + Radial1 * Radius;

        DrawTriangle(View, P0, P2, P1, Color);
        DrawTriangle(View, P1, P2, P3, Color);
    }
}

void FD3D11EditorOverlayRenderer::DrawConeAxis(const FSceneView &View, const FVector &Apex,
                                               const FVector &Direction, float Height, float Radius,
                                               const FColor &Color)
{
    constexpr int32 Segments = 16;

    FVector Dir = Direction;
    Dir.Normalize();

    FVector Tangent, Bitangent;
    BuildPerpendicularBasis(Dir, Tangent, Bitangent);

    const FVector BaseCenter = Apex - Dir * Height;

    for (int32 Index = 0; Index < Segments; ++Index)
    {
        const float T0 =
            (2.0f * Math::Pi * static_cast<float>(Index)) / static_cast<float>(Segments);
        const float T1 =
            (2.0f * Math::Pi * static_cast<float>(Index + 1)) / static_cast<float>(Segments);

        const FVector Radial0 = Tangent * std::cos(T0) + Bitangent * std::sin(T0);
        const FVector Radial1 = Tangent * std::cos(T1) + Bitangent * std::sin(T1);

        const FVector P0 = BaseCenter + Radial0 * Radius;
        const FVector P1 = BaseCenter + Radial1 * Radius;

        DrawTriangle(View, Apex, P0, P1, Color);
    }
}

void FD3D11EditorOverlayRenderer::DrawRing(const FSceneView &View, const FVector &Center,
                                           const FVector &Normal, float Radius, int32 Segments,
                                           const FColor &Color)
{
    FVector Tangent, Bitangent;
    BuildPerpendicularBasis(Normal, Tangent, Bitangent);

    constexpr float RingThickness = 3.0f;

    for (int32 Index = 0; Index < Segments; ++Index)
    {
        const float T0 =
            (2.0f * Math::Pi * static_cast<float>(Index)) / static_cast<float>(Segments);
        const float T1 =
            (2.0f * Math::Pi * static_cast<float>(Index + 1)) / static_cast<float>(Segments);

        const FVector P0 = Center + (Tangent * std::cos(T0) + Bitangent * std::sin(T0)) * Radius;
        const FVector P1 = Center + (Tangent * std::cos(T1) + Bitangent * std::sin(T1)) * Radius;

        FVector Delta = P1 - P0;
        float   Len = Delta.Length();
        if (Len > 0.0001f)
        {
            // 선분 대신 원통형 조각을 연결해서 그림
            DrawCylinderAxis(View, P0, Delta, Len, RingThickness, Color);
        }
    }
}

void FD3D11EditorOverlayRenderer::DrawScaleHandle(const FSceneView &View, const FVector &Origin,
                                                  const FVector &Direction, float ShaftLength,
                                                  float ShaftRadius, float HandleLength,
                                                  float HandleRadius, const FColor &Color)
{
    FVector Dir = Direction;
    Dir.Normalize();

    DrawCylinderAxis(View, Origin, Dir, ShaftLength, ShaftRadius, Color);

    const FVector HandleOrigin = Origin + Dir * ShaftLength;
    DrawCylinderAxis(View, HandleOrigin, Dir, HandleLength, HandleRadius, Color);
}

void FD3D11EditorOverlayRenderer::DrawLine(const FSceneView &View, const FVector &Start,
                                           const FVector &End, const FColor &Color,
                                           bool bUseDepthBias)
{
    Lines.push_back(FDebugLine(Start, End, Color, bUseDepthBias));
}

void FD3D11EditorOverlayRenderer::DrawTriangle(const FSceneView &View, const FVector &A,
                                               const FVector &B, const FVector &C,
                                               const FColor &Color)
{
    FVector V[3];
    FColor  Colors[3];

    V[0] = A;
    V[1] = B;
    V[2] = C;

    Colors[0] = Color;
    Colors[1] = Color;
    Colors[2] = Color;

    Triangles.push_back(FDebugTriangle(V, Colors));
}

void FD3D11EditorOverlayRenderer::BuildPerpendicularBasis(const FVector &Direction,
                                                          FVector       &OutTangent,
                                                          FVector       &OutBitangent) const
{
    FVector Dir = Direction;
    Dir.Normalize();

    FVector Up = (std::fabs(Dir.Y) < 0.99f) ? FVector(0.0f, 1.0f, 0.0f) : FVector(0.0f, 0.0f, 1.0f);

    OutTangent = Dir.Cross(Up);
    OutTangent.Normalize();

    OutBitangent = Dir.Cross(OutTangent);
    OutBitangent.Normalize();
}

bool FD3D11EditorOverlayRenderer::CreateLineBuffer(uint32 NewMaxLineNum)
{
    if (LineBuffer)
    {
        LineBuffer->Release();
        LineBuffer = nullptr;
    }

    const uint32 LineVertexNum = NewMaxLineNum * 2;

    D3D11_BUFFER_DESC Desc = {};
    Desc.Usage = D3D11_USAGE_DYNAMIC;
    Desc.ByteWidth = sizeof(FDebugLineVertex) * LineVertexNum;
    Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if (FAILED(RHI->GetDevice()->CreateBuffer(&Desc, nullptr, &LineBuffer)))
        return false;

    CurrentMaxLineNum = NewMaxLineNum;
    return true;
}

bool FD3D11EditorOverlayRenderer::CreateTriangleBuffer(uint32 NewMaxTriangleNum)
{
    if (TriangleBuffer)
    {
        TriangleBuffer->Release();
        TriangleBuffer = nullptr;
    }

    const uint32 TriangleVertexNum = NewMaxTriangleNum * 3;

    D3D11_BUFFER_DESC Desc = {};
    Desc.Usage = D3D11_USAGE_DYNAMIC;
    Desc.ByteWidth = sizeof(FDebugTriangleVertex) * TriangleVertexNum;
    Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if (FAILED(RHI->GetDevice()->CreateBuffer(&Desc, nullptr, &TriangleBuffer)))
        return false;

    CurrentMaxTriangleNum = NewMaxTriangleNum;
    return true;
}
