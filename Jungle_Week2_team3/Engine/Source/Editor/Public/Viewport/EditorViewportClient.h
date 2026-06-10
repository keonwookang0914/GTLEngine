#pragma once
#include "SceneView.h"
#include "EditorRenderState.h"
#include "EditorViewportState.h"
#include "HAL/Platform.h"
#include "Input/InputEvent.h"

class UWorld;
class AActor;
class FSceneView;

/**
 * Editor viewport client responsible for input handling, camera updates, selection state, gizmo
 * behavior, and viewport drawing.
 */
class FEditorViewportClient
{
  public:
    FEditorViewportClient() = default;
    ~FEditorViewportClient() { int a = 1; }

    explicit FEditorViewportClient(const FEditorCameraControllerConfig &InConfig) : Config(InConfig)
    {
    }

    void Tick(float DeltaTime);
    void Draw(float Width, float Height);
    void ResetInputState();
    void UpdateGizmoHoverByRaycast();

    void    SetWorld(UWorld *InWorld) { World = InWorld; }
    UWorld *GetWorld() const { return World; }

    void SetSceneView(FSceneView *InSceneView) { SceneView = InSceneView; }

    void SetCameraLocation(const FVector &InLocation);
    void SetCameraRotation(const FRotator &InRotation);
    void SetCameraRotation(float InYawDegrees, float InPitchDegrees, float InRollDegrees = 0.0f);
    void SetCameraFov(float InVerticalFovDegrees);
    void SetProjectionMode(EEditorProjectionMode InProjectionMode);

    const FVector  &GetCameraLocation() const { return CameraTransform.GetLocation(); }
    const FRotator &GetCameraRotation() const { return CameraTransform.GetRotation(); }
    void            GetCameraRotation(float &OutYawDegrees, float &OutPitchDegrees,
                                      float &OutRollDegrees) const;
    float           GetCameraFov() const;

    FViewportCameraTransform       &GetCameraTransform() { return CameraTransform; }
    const FViewportCameraTransform &GetCameraTransform() const { return CameraTransform; }

    const FEditorViewportViewState &GetCameraState() const { return ViewState; }

    void    SetSelectedActor(AActor *InActor);
    AActor *GetSelectedActor() const { return SelectedActor; }

    FGizmoState       &GetGizmoState() { return GizmoState; }
    const FGizmoState &GetGizmoState() const { return GizmoState; }

    bool                InputKey(EKey Key, EInputEvent Event);
    bool                InputAxis(EKey Key, float Delta);
    bool                MouseMove(int32 X, int32 Y);
    bool                CapturedMouseMove(int32 DeltaX, int32 DeltaY);
    void                ProcessClick(int32 X, int32 Y);
    EPointerReleaseType GetReleaseType(int32 X, int32 Y) const;

    void SetShowGrid(bool bInShowGrid) { bShowGrid = bInShowGrid; }
    void SetShowAxes(bool bInShowAxes) { bShowAxes = bInShowAxes; }
    void SetShowGizmo(bool bInShowGizmo)
    {
        bShowGizmo = bInShowGizmo;
        UpdateGizmoFromSelection();
    }

    bool IsGridVisible() const { return bShowGrid; }
    bool IsAxesVisible() const { return bShowAxes; }
    bool IsGizmoVisible() const { return bShowGizmo; }

    static FSceneView BuildSceneView(const FViewportCameraTransform &InCameraTransform,
                                     const FEditorViewportViewState &InViewState,
                                     float InViewportWidth, float InViewportHeight);

  private:
    void ResetVelocity();
    void UpdateCamera(float InDeltaTime);
    void UpdateRotation(float InDeltaTime);
    void UpdateFreeMove(float InDeltaTime);
    void UpdatePan(float InDeltaTime);
    void UpdateZoom(float InDeltaTime);
    void UpdateGizmoFromSelection();
    void UpdateGizmoDrag(int32 InMouseX, int32 InMouseY);
    void EndGizmoDrag();

    static float Clamp(float InValue, float InMinValue, float InMaxValue);

  private:
    UWorld     *World = nullptr;
    FSceneView *SceneView = nullptr;

    FEditorCameraControllerConfig Config;
    FEditorCameraInput            Input;
    FViewportCameraTransform      CameraTransform;
    FEditorViewportViewState      ViewState;

    AActor                    *SelectedActor = nullptr;
    class UPrimitiveComponent *SelectedComponent = nullptr;
    FGizmoState                GizmoState;

    FVector LinearVelocity = FVector(0.0f, 0.0f, 0.0f);
    float   FovVelocity = 0.0f;

    bool bLeftMouseDown = false;
    bool bRightMouseDown = false;
    bool bMiddleMouseDown = false;

    int32 MouseDownX = 0;
    int32 MouseDownY = 0;
    int32 LastMouseX = 0;
    int32 LastMouseY = 0;

    bool bShowGrid = true;
    bool bShowAxes = true;
    bool bShowGizmo = true;

    bool     bPendingLeftPressPick = false;
    bool     bIsDraggingGizmo = false;
    FVector  DragStartActorLocation = FVector(0.0f, 0.0f, 0.0f);
    FRotator DragStartActorRotation = FRotator(0.0f, 0.0f, 0.0f);
    FVector  DragStartActorScale = FVector(1.0f, 1.0f, 1.0f);
    int32    DragStartMouseX = 0;
    int32    DragStartMouseY = 0;

    bool bScaleUniformModifier = false;
};