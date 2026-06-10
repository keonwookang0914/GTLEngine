#include "D3D11/D3D11PickingPass.h"
#include "D3D11/D3D11DynamicRHI.h"
#include "EditorRenderState.h"
#include "Scene.h"
#include "SceneView.h"
#include "StaticMeshResources.h"
#include "StaticMeshSceneProxy.h"
#include <Math/MathConstants.h>
#include <cmath>
#include <d3dcompiler.h>

namespace
{
    constexpr uint32 GizmoPickIdX = 0xFF000001u;
    constexpr uint32 GizmoPickIdY = 0xFF000002u;
    constexpr uint32 GizmoPickIdZ = 0xFF000003u;
    constexpr uint32 GizmoPickMask = 0x80000000u;
    constexpr uint32 NoHitUUID = 0xFFFFFFFFu;

    FStaticMeshBuildVertex MakeVertex(const FVector &P)
    {
        FStaticMeshBuildVertex V = {};
        V.x = P.X;
        V.y = P.Y;
        V.z = P.Z;
        V.r = 1.0f;
        V.g = 1.0f;
        V.b = 1.0f;
        V.a = 1.0f;
        return V;
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

void FD3D11PickingPass::RequestPick(FScene *InScene, const FSceneView *InSceneView, int32 MouseX,
                                    int32 MouseY, const FGizmoState &InGizmoState)
{
    if (!PickingTexture || !PickingReadbackPixel || !PickingFence || !RHI)
        return;

    ID3D11DeviceContext  *DeviceContext = RHI->GetDeviceContext();
    const D3D11_VIEWPORT &Viewport = RHI->GetViewport();

    if (MouseX < 0 || MouseX >= (int32)Viewport.Width || MouseY < 0 ||
        MouseY >= (int32)Viewport.Height)
    {
        LastPickedId = 0;
        bPickRequestPending = false;
        return;
    }

    RenderPickingPass(InScene, InSceneView, InGizmoState);

    D3D11_BOX srcBox = {};
    srcBox.left = (UINT)MouseX;
    srcBox.right = (UINT)MouseX + 1;
    srcBox.top = (UINT)MouseY;
    srcBox.bottom = (UINT)MouseY + 1;
    srcBox.front = 0;
    srcBox.back = 1;

    DeviceContext->CopySubresourceRegion(PickingReadbackPixel, 0, 0, 0, 0, PickingTexture, 0,
                                         &srcBox);
    DeviceContext->End(PickingFence);

    bPickRequestPending = true;
}

void FD3D11PickingPass::RenderPickingPass(FScene *InScene, const FSceneView *InSceneView,
                                          const FGizmoState &InGizmoState)
{
    if (!InScene || !InSceneView || !RHI)
        return;

    ID3D11DeviceContext *DeviceContext = RHI->GetDeviceContext();

    DeviceContext->OMSetRenderTargets(1, &PickingRTV, PickingDSV);
    UINT clearId[4] = {0, 0, 0, 0};
    DeviceContext->ClearRenderTargetView(PickingRTV, (const float *)clearId);
    DeviceContext->ClearDepthStencilView(PickingDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f,
                                         0);

    DeviceContext->IASetInputLayout(PickingInputLayout);
    DeviceContext->VSSetShader(PickingVertexShader, nullptr, 0);
    DeviceContext->PSSetShader(PickingPixelShader, nullptr, 0);
    DeviceContext->VSSetConstantBuffers(1, 1, &PickingConstantBuffer);
    DeviceContext->PSSetConstantBuffers(1, 1, &PickingConstantBuffer);

    const FMatrix V = InSceneView->GetViewMatrix();
    const FMatrix P = InSceneView->GetProjectionMatrix();

    UINT Stride = sizeof(FStaticMeshBuildVertex);
    UINT Offset = 0;

    for (FPrimitiveSceneProxy *Proxy : InScene->Primitives)
    {
        FStaticMeshSceneProxy *MeshProxy = dynamic_cast<FStaticMeshSceneProxy *>(Proxy);
        if (MeshProxy == nullptr || MeshProxy->RenderData == nullptr ||
            MeshProxy->RenderData->VertexBuffer == nullptr ||
            MeshProxy->RenderData->Vertices.empty())
        {
            continue;
        }

        FPickingConstants Constants = {};
        Constants.MVP =
            MeshProxy->GetModelMatrix().GetTranspose() * V.GetTranspose() * P.GetTranspose();
        Constants.ObjectId = MeshProxy->UUID + 1u;
        DeviceContext->UpdateSubresource(PickingConstantBuffer, 0, nullptr, &Constants, 0, 0);

        DeviceContext->IASetVertexBuffers(0, 1, &MeshProxy->RenderData->VertexBuffer, &Stride,
                                          &Offset);
        DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        DeviceContext->Draw(static_cast<UINT>(MeshProxy->RenderData->Vertices.size()), 0);
    }

    ID3D11DepthStencilState *PrevDepthState = nullptr;
    UINT                     PrevStencilRef = 0;
    DeviceContext->OMGetDepthStencilState(&PrevDepthState, &PrevStencilRef);

    D3D11_DEPTH_STENCIL_DESC DepthDesc = {};
    DepthDesc.DepthEnable = FALSE;
    DepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    DepthDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    DepthDesc.StencilEnable = FALSE;

    ID3D11DepthStencilState *GizmoDepthDisabledState = nullptr;
    if (SUCCEEDED(RHI->GetDevice()->CreateDepthStencilState(&DepthDesc, &GizmoDepthDisabledState)))
    {
        DeviceContext->OMSetDepthStencilState(GizmoDepthDisabledState, 0);
        RenderGizmoPicking(InSceneView, InGizmoState);
        DeviceContext->OMSetDepthStencilState(PrevDepthState, PrevStencilRef);
        GizmoDepthDisabledState->Release();
    }
    else
    {
        RenderGizmoPicking(InSceneView, InGizmoState);
    }

    if (PrevDepthState)
    {
        PrevDepthState->Release();
    }

    ID3D11RenderTargetView *nullRTV = nullptr;
    DeviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);
}

void FD3D11PickingPass::Initialize(FD3D11DynamicRHI *InRHI)
{
    RHI = InRHI;
    CreateShaders();

    const D3D11_VIEWPORT &Viewport = RHI->GetViewport();
    CreateResources((int32)Viewport.Width, (int32)Viewport.Height);
}

void FD3D11PickingPass::Shutdown()
{
    ReleaseResources();
    ReleaseShaders();
    RHI = nullptr;
}

void FD3D11PickingPass::OnWindowResized(int32 InWidth, int32 InHeight)
{
    ReleaseResources();
    CreateResources(InWidth, InHeight);
}

void FD3D11PickingPass::CreateResources(int32 Width, int32 Height)
{
    if (Width <= 0 || Height <= 0)
        return;

    ID3D11Device *Device = RHI->GetDevice();

    // Picking Texture
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = (UINT)Width;
    texDesc.Height = (UINT)Height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R32_UINT;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
    Device->CreateTexture2D(&texDesc, nullptr, &PickingTexture);
    Device->CreateRenderTargetView(PickingTexture, nullptr, &PickingRTV);

    // Picking Depth
    D3D11_TEXTURE2D_DESC depthDesc = texDesc;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    Device->CreateTexture2D(&depthDesc, nullptr, &PickingDepthBuffer);
    Device->CreateDepthStencilView(PickingDepthBuffer, nullptr, &PickingDSV);

    // Readback Pixel
    D3D11_TEXTURE2D_DESC rbDesc = {};
    rbDesc.Width = 1;
    rbDesc.Height = 1;
    rbDesc.MipLevels = 1;
    rbDesc.ArraySize = 1;
    rbDesc.Format = DXGI_FORMAT_R32_UINT;
    rbDesc.SampleDesc.Count = 1;
    rbDesc.Usage = D3D11_USAGE_STAGING;
    rbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    Device->CreateTexture2D(&rbDesc, nullptr, &PickingReadbackPixel);

    // Fence
    D3D11_QUERY_DESC qDesc = {};
    qDesc.Query = D3D11_QUERY_EVENT;
    Device->CreateQuery(&qDesc, &PickingFence);

    // Constant Buffer
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(FPickingConstants);
    cbDesc.Usage = D3D11_USAGE_DEFAULT;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    Device->CreateBuffer(&cbDesc, nullptr, &PickingConstantBuffer);
}

void FD3D11PickingPass::ReleaseResources()
{
    if (PickingTexture)
        PickingTexture->Release();
    PickingTexture = nullptr;
    if (PickingRTV)
        PickingRTV->Release();
    PickingRTV = nullptr;
    if (PickingDepthBuffer)
        PickingDepthBuffer->Release();
    PickingDepthBuffer = nullptr;
    if (PickingDSV)
        PickingDSV->Release();
    PickingDSV = nullptr;
    if (PickingReadbackPixel)
        PickingReadbackPixel->Release();
    PickingReadbackPixel = nullptr;
    if (PickingFence)
        PickingFence->Release();
    PickingFence = nullptr;
    if (PickingConstantBuffer)
        PickingConstantBuffer->Release();
    PickingConstantBuffer = nullptr;
}

void FD3D11PickingPass::CreateShaders()
{
    ID3D11Device *Device = RHI->GetDevice();
    ID3DBlob     *vsBlob = nullptr;
    ID3DBlob     *psBlob = nullptr;
    ID3DBlob     *errorBlob = nullptr;

    D3DCompileFromFile(L"Engine/Shaders/Shader.hlsl", nullptr, nullptr, "pickingVS", "vs_5_0", 0, 0,
                       &vsBlob, &errorBlob);
    Device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr,
                               &PickingVertexShader);

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    Device->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(),
                              vsBlob->GetBufferSize(), &PickingInputLayout);

    D3DCompileFromFile(L"Engine/Shaders/Shader.hlsl", nullptr, nullptr, "pickingPS", "ps_5_0", 0, 0,
                       &psBlob, &errorBlob);
    Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr,
                              &PickingPixelShader);

    if (vsBlob)
        vsBlob->Release();
    if (psBlob)
        psBlob->Release();
    if (errorBlob)
        errorBlob->Release();
}

void FD3D11PickingPass::ReleaseShaders()
{
    if (PickingVertexShader)
        PickingVertexShader->Release();
    PickingVertexShader = nullptr;
    if (PickingPixelShader)
        PickingPixelShader->Release();
    PickingPixelShader = nullptr;
    if (PickingInputLayout)
        PickingInputLayout->Release();
    PickingInputLayout = nullptr;
}

void FD3D11PickingPass::RenderGizmoPicking(const FSceneView  *InSceneView,
                                           const FGizmoState &InGizmoState)
{
    if (InSceneView == nullptr || !InGizmoState.bVisible)
    {
        return;
    }

    const FVector Origin = InGizmoState.Origin;

    FVector XAxis, YAxis, ZAxis;
    GetGizmoAxes(InGizmoState, XAxis, YAxis, ZAxis);

    switch (InGizmoState.Mode)
    {
    case EGizmoMode::Translate:
    case EGizmoMode::Scale:
    {
        const float AxisLength = 120.0f;
        const float AxisThickness = 12.0f;

        DrawPickingAxisBox(InSceneView, Origin, XAxis, AxisLength, AxisThickness, GizmoPickIdX);
        DrawPickingAxisBox(InSceneView, Origin, YAxis, AxisLength, AxisThickness, GizmoPickIdY);
        DrawPickingAxisBox(InSceneView, Origin, ZAxis, AxisLength, AxisThickness, GizmoPickIdZ);
        break;
    }

    case EGizmoMode::Rotate:
    {
        const float Radius = 72.0f;
        const float Thickness = 12.0f;

        DrawPickingRing(InSceneView, Origin, XAxis, Radius, Thickness, GizmoPickIdX);
        DrawPickingRing(InSceneView, Origin, YAxis, Radius, Thickness, GizmoPickIdY);
        DrawPickingRing(InSceneView, Origin, ZAxis, Radius, Thickness, GizmoPickIdZ);
        break;
    }

    default:
        break;
    }
}

void FD3D11PickingPass::DrawPickingRing(const FSceneView *InSceneView, const FVector &Center,
                                        const FVector &Normal, float Radius, float Thickness,
                                        uint32 InObjectId)
{
    if (InSceneView == nullptr || RHI == nullptr)
    {
        return;
    }

    FVector T, B;
    BuildPerpendicularBasis(Normal, T, B);

    constexpr int32                Segments = 32;
    TArray<FStaticMeshBuildVertex> Vertices;
    Vertices.reserve(Segments * 36);

    auto PushTri = [&Vertices](const FVector &A, const FVector &Bv, const FVector &C)
    {
        Vertices.push_back(MakeVertex(A));
        Vertices.push_back(MakeVertex(Bv));
        Vertices.push_back(MakeVertex(C));
    };

    const float Half = Thickness * 0.5f;

    for (int32 i = 0; i < Segments; ++i)
    {
        const float a0 = (2.0f * 3.14159f * (float)i) / (float)Segments;
        const float a1 = (2.0f * 3.14159f * (float)(i + 1)) / (float)Segments;

        const FVector P0 = Center + (T * std::cos(a0) + B * std::sin(a0)) * Radius;
        const FVector P1 = Center + (T * std::cos(a1) + B * std::sin(a1)) * Radius;

        FVector Dir = P1 - P0;
        float   Len = Dir.Length();
        if (Len < 0.001f)
            continue;
        Dir.Normalize();

        FVector t, b;
        BuildPerpendicularBasis(Dir, t, b);

        const FVector S0 = P0 - t * Half - b * Half;
        const FVector S1 = P0 + t * Half - b * Half;
        const FVector S2 = P0 + t * Half + b * Half;
        const FVector S3 = P0 - t * Half + b * Half;

        const FVector E0 = P1 - t * Half - b * Half;
        const FVector E1 = P1 + t * Half - b * Half;
        const FVector E2 = P1 + t * Half + b * Half;
        const FVector E3 = P1 - t * Half + b * Half;

        PushTri(S0, S2, S1);
        PushTri(S0, S3, S2);
        PushTri(E0, E1, E2);
        PushTri(E0, E2, E3);
        PushTri(S0, S1, E1);
        PushTri(S0, E1, E0);
        PushTri(S1, S2, E2);
        PushTri(S1, E2, E1);
        PushTri(S2, S3, E3);
        PushTri(S2, E3, E2);
        PushTri(S3, S0, E0);
        PushTri(S3, E0, E3);
    }

    ID3D11Buffer     *VertexBuffer = nullptr;
    D3D11_BUFFER_DESC Desc = {};
    Desc.Usage = D3D11_USAGE_DEFAULT;
    Desc.ByteWidth = static_cast<UINT>(Vertices.size() * sizeof(FStaticMeshBuildVertex));
    Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA Data = {};
    Data.pSysMem = Vertices.data();

    if (FAILED(RHI->GetDevice()->CreateBuffer(&Desc, &Data, &VertexBuffer)))
    {
        return;
    }

    FPickingConstants Constants = {};
    FMatrix           World;
    World.SetIdentity();

    const FMatrix V = InSceneView->GetViewMatrix();
    const FMatrix P = InSceneView->GetProjectionMatrix();

    Constants.MVP = World.GetTranspose() * V.GetTranspose() * P.GetTranspose();
    Constants.ObjectId = InObjectId;

    ID3D11DeviceContext *DeviceContext = RHI->GetDeviceContext();
    DeviceContext->UpdateSubresource(PickingConstantBuffer, 0, nullptr, &Constants, 0, 0);

    UINT Stride = sizeof(FStaticMeshBuildVertex);
    UINT Offset = 0;
    DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &Stride, &Offset);
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    DeviceContext->Draw(static_cast<UINT>(Vertices.size()), 0);

    VertexBuffer->Release();
}

void FD3D11PickingPass::DrawPickingAxisBox(const FSceneView *InSceneView, const FVector &Origin,
                                           const FVector &AxisDirection, float AxisLength,
                                           float AxisThickness, uint32 InObjectId)
{
    if (InSceneView == nullptr || RHI == nullptr)
    {
        return;
    }

    FVector Dir = AxisDirection;
    Dir.Normalize();

    FVector T, B;
    BuildPerpendicularBasis(Dir, T, B);

    const float   Half = AxisThickness * 0.5f;
    const FVector Start = Origin;
    const FVector End = Origin + Dir * AxisLength;

    const FVector P0 = Start - T * Half - B * Half;
    const FVector P1 = Start + T * Half - B * Half;
    const FVector P2 = Start + T * Half + B * Half;
    const FVector P3 = Start - T * Half + B * Half;

    const FVector P4 = End - T * Half - B * Half;
    const FVector P5 = End + T * Half - B * Half;
    const FVector P6 = End + T * Half + B * Half;
    const FVector P7 = End - T * Half + B * Half;

    TArray<FStaticMeshBuildVertex> Vertices;
    Vertices.reserve(36);

    auto PushTri = [&Vertices](const FVector &A, const FVector &Bv, const FVector &C)
    {
        Vertices.push_back(MakeVertex(A));
        Vertices.push_back(MakeVertex(Bv));
        Vertices.push_back(MakeVertex(C));
    };

    PushTri(P0, P2, P1);
    PushTri(P0, P3, P2);

    PushTri(P4, P5, P6);
    PushTri(P4, P6, P7);

    PushTri(P0, P1, P5);
    PushTri(P0, P5, P4);

    PushTri(P1, P2, P6);
    PushTri(P1, P6, P5);

    PushTri(P2, P3, P7);
    PushTri(P2, P7, P6);

    PushTri(P3, P0, P4);
    PushTri(P3, P4, P7);

    ID3D11Buffer     *VertexBuffer = nullptr;
    D3D11_BUFFER_DESC Desc = {};
    Desc.Usage = D3D11_USAGE_DEFAULT;
    Desc.ByteWidth = static_cast<UINT>(Vertices.size() * sizeof(FStaticMeshBuildVertex));
    Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA Data = {};
    Data.pSysMem = Vertices.data();

    if (FAILED(RHI->GetDevice()->CreateBuffer(&Desc, &Data, &VertexBuffer)))
    {
        return;
    }

    FPickingConstants Constants = {};
    FMatrix           World;
    World.SetIdentity();

    const FMatrix V = InSceneView->GetViewMatrix();
    const FMatrix P = InSceneView->GetProjectionMatrix();

    Constants.MVP = World.GetTranspose() * V.GetTranspose() * P.GetTranspose();
    Constants.ObjectId = InObjectId;

    ID3D11DeviceContext *DeviceContext = RHI->GetDeviceContext();
    DeviceContext->UpdateSubresource(PickingConstantBuffer, 0, nullptr, &Constants, 0, 0);

    UINT Stride = sizeof(FStaticMeshBuildVertex);
    UINT Offset = 0;
    DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &Stride, &Offset);
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    DeviceContext->Draw(static_cast<UINT>(Vertices.size()), 0);

    VertexBuffer->Release();
}

void FD3D11PickingPass::BuildPerpendicularBasis(const FVector &Direction, FVector &OutTangent,
                                                FVector &OutBitangent) const
{
    FVector Dir = Direction;
    Dir.Normalize();

    FVector Up = (std::fabs(Dir.Y) < 0.99f) ? FVector(0.0f, 1.0f, 0.0f) : FVector(0.0f, 0.0f, 1.0f);

    OutTangent = Dir.Cross(Up);
    OutTangent.Normalize();

    OutBitangent = Dir.Cross(OutTangent);
    OutBitangent.Normalize();
}

bool FD3D11PickingPass::TryConsumePickResult(uint32 &OutPickId)
{
    if (!bPickRequestPending || RHI == nullptr || PickingFence == nullptr ||
        PickingReadbackPixel == nullptr)
    {
        return false;
    }

    ID3D11DeviceContext *DeviceContext = RHI->GetDeviceContext();
    if (DeviceContext == nullptr)
    {
        return false;
    }

    const HRESULT QueryResult =
        DeviceContext->GetData(PickingFence, nullptr, 0, D3D11_ASYNC_GETDATA_DONOTFLUSH);
    if (QueryResult == S_FALSE)
    {
        return false;
    }

    if (FAILED(QueryResult))
    {
        bPickRequestPending = false;
        return false;
    }

    D3D11_MAPPED_SUBRESOURCE Mapped = {};
    const HRESULT            MapResult =
        DeviceContext->Map(PickingReadbackPixel, 0, D3D11_MAP_READ, 0, &Mapped);

    if (FAILED(MapResult) || Mapped.pData == nullptr)
    {
        bPickRequestPending = false;
        return false;
    }

    const uint32 RawPickedId = *reinterpret_cast<uint32 *>(Mapped.pData);
    if (RawPickedId == 0u)
    {
        OutPickId = NoHitUUID;
    }
    else if ((RawPickedId & GizmoPickMask) != 0u)
    {
        OutPickId = RawPickedId;
    }
    else
    {
        OutPickId = RawPickedId - 1u;
    }
    DeviceContext->Unmap(PickingReadbackPixel, 0);

    LastPickedId = OutPickId;
    bPickRequestPending = false;
    return true;
}