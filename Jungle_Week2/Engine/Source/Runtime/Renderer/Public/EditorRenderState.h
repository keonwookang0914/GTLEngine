#pragma once

#include "Viewport/EditorViewportState.h"

class FSceneView;

enum class EGizmoMode : uint8
{
    None,
    Translate,
    Rotate,
    Scale
};

enum class EGizmoSpace : uint8
{
    World,
    Local
};

enum class EGizmoAxis : uint8
{
    None,
    X,
    Y,
    Z
};

struct FGizmoState
{
    bool        bVisible = false;
    EGizmoMode  Mode = EGizmoMode::Translate;
    EGizmoSpace Space = EGizmoSpace::World;
    EGizmoAxis  HighlightedAxis = EGizmoAxis::None;
    EGizmoAxis  ActiveAxis = EGizmoAxis::None;

    FVector  Origin = FVector(0.0f, 0.0f, 0.0f);
    FRotator Rotation = FRotator(0.0f, 0.0f, 0.0f);
};

struct FEditorRenderState
{
    const FSceneView *SceneView = nullptr;

    bool bShowGrid = true;
    bool bShowWorldAxes = true;
    bool bShowGizmo = true;
    bool bShowSelectionOutline = true;

    FGizmoState Gizmo;
};