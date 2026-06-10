#pragma once

#include "D3D11/D3D11DynamicRHI.h"
#include "D3D11/D3D11EditorOverlayRenderer.h"
#include "D3D11/D3D11MeshPassRenderer.h"
#include "D3D11/D3D11PickingPass.h"
#include "Math/Vector4.h"
#include <Windows.h>


class FScene;
class FSceneView;

class FRendererModule
{
  public:
    void StartupModule(HWND hWnd);
    void ShutdownModule();

    void BeginFrame();
    void EndFrame();

    void OnWindowResized(int32 InWidth, int32 InHeight);

    void    SetScene(FScene *InScene) { Scene = InScene; }
    FScene *GetScene() const { return Scene; }

    void RenderFrame(const FSceneView *InSceneView, const FGizmoState &InGizmoState,
                     const FVector4 &Color = FVector4(1.0f, 1.0f, 1.0f, 1.0f));

    void RenderWorld(const FSceneView *InSceneView,
                     const FVector4   &Color = FVector4(1.0f, 1.0f, 1.0f, 1.0f),
                     bool              bUseInstancing = false);

    void RenderEditorOverlays(const FSceneView *InSceneView, const FGizmoState &InGizmoState);

    /** Picking API */
    bool TryConsumePickResult(uint32 &OutPickId);
    void RequestPick(const FSceneView *InSceneView, int32 MouseX, int32 MouseY,
                     const FGizmoState &InGizmoState);

    FD3D11DynamicRHI            &GetRHI() { return RHI; }
    FD3D11MeshPassRenderer      &GetMeshRenderer() { return MeshRenderer; }
    FD3D11EditorOverlayRenderer &GetD3D11EditorOverlayRenderer()
    {
        return D3D11EditorOverlayRenderer;
    }

  private:
    FD3D11DynamicRHI            RHI;
    FD3D11MeshPassRenderer      MeshRenderer;
    FD3D11PickingPass           PickingPass;
    FD3D11EditorOverlayRenderer D3D11EditorOverlayRenderer;
    FScene                     *Scene = nullptr;
    ID3D11Debug                *debug;
};