#pragma once

#include "EditorRenderState.h"
#include "HAL/Platform.h"
#include "Math/Matrix.h"
#include "Math/Vector.h"
#include <d3d11.h>

class FD3D11DynamicRHI;
class FScene;
class FSceneView;

struct FPickingConstants
{
    FMatrix MVP;
    uint32  ObjectId;
    float   Padding[3];
};

class FD3D11PickingPass
{
  public:
    void Initialize(FD3D11DynamicRHI *InRHI);
    void Shutdown();

    void OnWindowResized(int32 InWidth, int32 InHeight);

    /**
     * Issues an asynchronous pick request for the given mouse position.
     *
     * Renders the picking pass, copies the target pixel to the readback buffer,
     * and inserts a GPU fence. The result can be retrieved later via
     * TryConsumePickResult().
     *
     * \param InScene Scene to pick against.
     * \param InSceneView View used for the picking pass.
     * \param MouseX Mouse X in viewport space.
     * \param MouseY Mouse Y in viewport space.
     */
    void RequestPick(FScene *InScene, const FSceneView *InSceneView, int32 MouseX, int32 MouseY,
                     const FGizmoState &InGizmoState);
    /**
     * Tries to consume the pending pick result.
     *
     * \param OutPickId Receives the picked ID on success.
     * \return True if a pick result was available.
     */
    bool TryConsumePickResult(uint32 &OutPickId);

  private:
    void CreateResources(int32 Width, int32 Height);
    void ReleaseResources();

    void CreateShaders();
    void ReleaseShaders();

    void RenderPickingPass(FScene *InScene, const FSceneView *InSceneView,
                           const FGizmoState &InGizmoState);

    void DrawPickingAxisBox(const FSceneView *InSceneView, const FVector &Origin,
                            const FVector &AxisDirection, float AxisLength, float AxisThickness,
                            uint32 InObjectId);

    void DrawPickingRing(const FSceneView *InSceneView, const FVector &Center,
                         const FVector &Normal, float Radius, float Thickness, uint32 InObjectId);

    void BuildPerpendicularBasis(const FVector &Direction, FVector &OutTangent,
                                 FVector &OutBitangent) const;

    void RenderGizmoPicking(const FSceneView *InSceneView, const FGizmoState &InGizmoState);

  private:
    FD3D11DynamicRHI *RHI = nullptr;

    ID3D11Texture2D        *PickingTexture = nullptr;
    ID3D11RenderTargetView *PickingRTV = nullptr;
    ID3D11Texture2D        *PickingDepthBuffer = nullptr;
    ID3D11DepthStencilView *PickingDSV = nullptr;

    ID3D11Texture2D *PickingReadbackPixel = nullptr;
    ID3D11Query     *PickingFence = nullptr;

    ID3D11VertexShader *PickingVertexShader = nullptr;
    ID3D11PixelShader  *PickingPixelShader = nullptr;
    ID3D11InputLayout  *PickingInputLayout = nullptr;
    ID3D11Buffer       *PickingConstantBuffer = nullptr;

    uint32 LastPickedId = 0;
    bool   bPickRequestPending = false;
};