#pragma once

#include "HAL/Platform.h"
#include "Math/Rotator.h"
#include "Math/Vector.h"

/**
 * Defines shared editor viewport types used for camera movement,
 * projection settings, pointer interaction, and gizmo behavior.
 * Includes transient input state, controller configuration, and
 * persistent view state for the editor viewport.
 */

enum class EPointerReleaseType : uint8
{
    Click,
    Drag
};

enum class EEditorProjectionMode : uint8
{
    Perspective,
    Orthographic,
};

enum class EEditorCameraControlMode : uint8
{
    None,
    Rotate,
    Pan,
    Zoom,
};

struct FEditorViewportViewState
{
    EEditorProjectionMode ProjectionMode = EEditorProjectionMode::Perspective;

    float VerticalFovDegrees = 60.0f;
    float NearPlane = 1.0f;
    float FarPlane = 100000.0f;
    float OrthoWidth = 2048.0f;
};

struct FEditorCameraInput
{
    float MoveForward = 0.0f;
    float MoveRight = 0.0f;
    float MoveUpLocal = 0.0f;
    float MoveUpWorld = 0.0f;

    float YawDelta = 0.0f;
    float PitchDelta = 0.0f;
    float RollDelta = 0.0f;

    float Zoom = 0.0f;
    float PanX = 0.0f;
    float PanY = 0.0f;

    EEditorCameraControlMode ControlMode = EEditorCameraControlMode::None;

    void ResetFrameInput()
    {
        YawDelta = 0.0f;
        PitchDelta = 0.0f;
        PanX = 0.0f;
        PanY = 0.0f;
        Zoom = 0.0f;
        ControlMode = EEditorCameraControlMode::None;
    }
};

struct FEditorCameraControllerConfig
{
    float TranslationAcceleration = 2500.0f;
    float TranslationDamping = 10.0f;

    float RotationSpeed = 0.1f;
    float ZoomSpeed = 2000.0f;

    float MinPitchDegrees = -89.0f;
    float MaxPitchDegrees = 89.0f;

    float PanSpeed = 0.3f;
    float ZoomTranslationScale = 1.0f;
};

struct FViewportCameraTransform
{
  public:
    const FVector  &GetLocation() const { return CaemeraLocation; }
    const FRotator &GetRotation() const { return ViewRotation; }

    void SetLocation(const FVector &InLocation) { CaemeraLocation = InLocation; }
    void SetRotation(const FRotator &InRotation) { ViewRotation = InRotation; }

  private:
    FVector  CaemeraLocation = FVector(360.0f, 360.0f, 360.0f);
    FRotator ViewRotation = FRotator(-45.0f, -135.0f, 0.0f);
};