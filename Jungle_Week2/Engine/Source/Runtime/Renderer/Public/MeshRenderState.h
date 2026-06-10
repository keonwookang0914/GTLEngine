#pragma once

class FSceneView;
struct FGizmoState;

struct FMeshRenderState
{
    const FSceneView *SceneView = nullptr;

    bool bWireframe = false;
};