#include "RendererModule.h"
#include "EditorRenderState.h"
#include "HAL/Platform.h"
#include "PrimitiveSceneProxy.h"
#include "Scene.h"
#include "SceneView.h"
#include "StaticMeshResources.h"
#include "StaticMeshSceneProxy.h"
#include "UIManager.h"
#include "Viewport/EditorViewportState.h"
#include <cassert>

void FRendererModule::StartupModule(HWND hWnd)
{
    RHI.Initialize(hWnd);
    MeshRenderer.Initialize(&RHI);
    PickingPass.Initialize(&RHI);
    D3D11EditorOverlayRenderer.Initialize(&RHI, &MeshRenderer);

    if (Scene == nullptr)
    {
        Scene = new FScene();
    }
    RHI.GetDevice()->QueryInterface(_uuidof(ID3D11Debug), (void **)&debug);
}

void FRendererModule::ShutdownModule()
{
    if (Scene)
    {
        delete Scene;
        Scene = nullptr;
    }

    D3D11EditorOverlayRenderer.Shutdown();
    PickingPass.Shutdown();
    MeshRenderer.Shutdown();
    RHI.Shutdown();

    // 모든 리소스 해제 후 보고 → 남아있는 것만 진짜 누수
    if (debug)
    {
        debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
        debug->Release();
        debug = nullptr;
    }
}

void FRendererModule::BeginFrame()
{
    RHI.BeginFrame();

    SUIManager::Get().BeginFrame();

    // TODO:
    // - 렌더 타깃 / 뎁스 스텐실 바인딩
    // - 뷰포트 설정
    // - 컬러 / 뎁스 버퍼 클리어
    // - 프레임 단위 공통 렌더 상태 설정
}

void FRendererModule::EndFrame()
{
    SUIManager::Get().RenderAll();
    SUIManager::Get().EndFrame();
    RHI.EndFrame();
}

void FRendererModule::OnWindowResized(int32 InWidth, int32 InHeight)
{
    RHI.Resize(InWidth, InHeight);
    PickingPass.OnWindowResized(InWidth, InHeight);
}

void FRendererModule::RenderFrame(const FSceneView *InSceneView, const FGizmoState &InGizmoState,
                                  const FVector4 &Color)
{
    if (InSceneView == nullptr)
    {
        return;
    }

    RenderWorld(InSceneView, Color, true);
    RenderEditorOverlays(InSceneView, InGizmoState);
}

void FRendererModule::RenderWorld(const FSceneView *InSceneView, const FVector4 &Color,
                                  bool bUseInstancing)
{
    if (Scene == nullptr || InSceneView == nullptr)
    {
        return;
    }

    const FMatrix &View = InSceneView->GetViewMatrix();
    const FMatrix &Projection = InSceneView->GetProjectionMatrix();

    MeshRenderer.PrepareShader();

    for (FPrimitiveSceneProxy *PrimitiveSceneProxy : Scene->Primitives)
    {
        if (PrimitiveSceneProxy == nullptr)
        {
            continue;
        }

        FStaticMeshSceneProxy *StaticMeshSceneProxy =
            static_cast<FStaticMeshSceneProxy *>(PrimitiveSceneProxy);

        if (StaticMeshSceneProxy->RenderData == nullptr ||
            StaticMeshSceneProxy->RenderData->VertexBuffer == nullptr ||
            !StaticMeshSceneProxy->RenderData->IsValid())
        {
            continue;
        }

        if (bUseInstancing)
        {
            MeshRenderer.UpdateConstant(FMatrix::Identity, View.GetTranspose(),
                                        Projection.GetTranspose(), true);

            MeshRenderer.RegisterInstance(StaticMeshSceneProxy->RenderData->VertexBuffer,
                                          StaticMeshSceneProxy->RenderData->Vertices.size(),
                                          PrimitiveSceneProxy->GetModelMatrix());
        }
        else
        {
            MeshRenderer.UpdateConstant(PrimitiveSceneProxy->GetModelMatrix().GetTranspose(),
                                        View.GetTranspose(), Projection.GetTranspose(), false);

            MeshRenderer.RenderPrimitive(
                StaticMeshSceneProxy->RenderData->VertexBuffer,
                static_cast<UINT>(StaticMeshSceneProxy->RenderData->Vertices.size()));
        }
    }

    if (bUseInstancing)
        MeshRenderer.Draw();
}

void FRendererModule::RenderEditorOverlays(const FSceneView  *InSceneView,
                                           const FGizmoState &InGizmoState)
{
    if (InSceneView == nullptr)
    {
        return;
    }

    // TODO:
    // - Grid
    // - World Axes
    // - Selection Outline
    // - Transform Gizmo
    // 등 에디터 전용 오버레이 렌더링

    // 예시:
    // D3D11EditorOverlayRenderer.DrawGrid(*InSceneView, 1000.0f, 100.0f);
    // D3D11EditorOverlayRenderer.DrawWorldAxes(*InSceneView);

    D3D11EditorOverlayRenderer.DrawGrid(*InSceneView, 2000.0f, 100.0f);
    D3D11EditorOverlayRenderer.DrawWorldAxes(*InSceneView);
    D3D11EditorOverlayRenderer.DrawTransformGizmo(*InSceneView, InGizmoState);

    /*
    if (bShowGrid)
    {
        D3D11EditorOverlayRenderer.DrawGrid(*InSceneView, 2000.0f, 100.0f);
    }

    if (bShowAxes)
    {
        D3D11EditorOverlayRenderer.DrawWorldAxes(*InSceneView, 120.0f);
    }

    if (bShowGizmo && GizmoState.bVisible)
    {
        D3D11EditorOverlayRenderer.DrawTransformGizmo(*InSceneView, GizmoState);
    }
    */

    /** Line / Triangle 의 경우 이 때 실제로 그려짐 */
    D3D11EditorOverlayRenderer.Draw(*InSceneView, InGizmoState);
}

void FRendererModule::RequestPick(const FSceneView *InSceneView, int32 MouseX, int32 MouseY,
                                  const FGizmoState &InGizmoState)
{
    PickingPass.RequestPick(Scene, InSceneView, MouseX, MouseY, InGizmoState);
}

bool FRendererModule::TryConsumePickResult(uint32 &OutPickId)
{
    return PickingPass.TryConsumePickResult(OutPickId);
}