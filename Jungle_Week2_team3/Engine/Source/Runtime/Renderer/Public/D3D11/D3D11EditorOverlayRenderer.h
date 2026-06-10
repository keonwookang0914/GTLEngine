#pragma once

#include "Containers/Array.h"
#include "HAL/Platform.h"
#include "Math/Vector.h"
#include "Math/Vector4.h"
#include "Viewport/EditorViewportState.h"
#include <d3d11.h>

class FD3D11DynamicRHI;
class FD3D11MeshPassRenderer;
class FSceneView;
struct FGizmoState;
struct FDebugLineVertex;

struct FColor
{
    float R, G, B, A;

    FColor(float InR = 0.0f, float InG = 0.0f, float InB = 0.0f, float InA = 1.0f)
        : R(InR), G(InG), B(InB), A(InA)
    {
    }
};

struct FDebugLine
{
    FVector Start;
    FVector End;
    FColor  Color;
    bool    bUseDepthBias = false;

    FDebugLine(const FVector &InStart, const FVector &InEnd, const FColor &InColor,
               bool bInUseDepthBias = false)
        : Start(InStart), End(InEnd), Color(InColor), bUseDepthBias(bInUseDepthBias)
    {
    }
};

struct FDebugTriangle
{
    FVector V[3];
    FColor  Color[3];

    FDebugTriangle(FVector InV[3], FColor InColor[3])
    {
        for (int32 i = 0; i < 3; i++)
        {
            V[i] = InV[i];
            Color[i] = InColor[i];
        }
    }
};

class FD3D11EditorOverlayRenderer
{
  public:
    void Initialize(FD3D11DynamicRHI *InRHI, FD3D11MeshPassRenderer *InMeshRenderer);
    void Shutdown();

    void DrawGrid(const FSceneView &View, float HalfExtent, float Spacing);
    void DrawWorldAxes(const FSceneView &View);

    void DrawTransformGizmo(const FSceneView &View, const FGizmoState &GizmoState);

    void Draw(const FSceneView &View, const FGizmoState &GizmoState);

  private:
    void DrawTranslationGizmo(const FSceneView &View, const FGizmoState &GizmoState);
    void DrawRotationGizmo(const FSceneView &View, const FGizmoState &GizmoState);
    void DrawScaleGizmo(const FSceneView &View, const FGizmoState &GizmoState);

    void DrawCylinderAxis(const FSceneView &View, const FVector &Origin, const FVector &Direction,
                          float Length, float Radius, const FColor &Color);

    void DrawConeAxis(const FSceneView &View, const FVector &Apex, const FVector &Direction,
                      float Height, float Radius, const FColor &Color);

    void DrawRing(const FSceneView &View, const FVector &Center, const FVector &Normal,
                  float Radius, int32 Segments, const FColor &Color);

    void DrawScaleHandle(const FSceneView &View, const FVector &Origin, const FVector &Direction,
                         float ShaftLength, float ShaftRadius, float HandleLength,
                         float HandleRadius, const FColor &Color);

  private:
    void DrawLine(const FSceneView &View, const FVector &Start, const FVector &End,
                  const FColor &Color, bool bUseDepthBias = false);

    void DrawTriangle(const FSceneView &View, const FVector &A, const FVector &B, const FVector &C,
                      const FColor &Color);

    void BuildPerpendicularBasis(const FVector &Direction, FVector &OutTangent,
                                 FVector &OutBitangent) const;

    bool CreateLineBuffer(uint32 NewMaxLineNum);
    bool CreateTriangleBuffer(uint32 NewMaxTriangleNum);

  private:
    FD3D11DynamicRHI       *RHI = nullptr;
    FD3D11MeshPassRenderer *MeshRenderer = nullptr;

    ID3D11DepthStencilState *OverlayDepthDisabledState = nullptr;
    ID3D11RasterizerState   *GridDepthBiasedRasterizerState = nullptr;

    ID3D11Buffer      *LineBuffer = nullptr;
    TArray<FDebugLine> Lines;
    uint32             CurrentMaxLineNum = 1024;

    ID3D11Buffer          *TriangleBuffer = nullptr;
    TArray<FDebugTriangle> Triangles;
    uint32                 CurrentMaxTriangleNum = 1024;
};