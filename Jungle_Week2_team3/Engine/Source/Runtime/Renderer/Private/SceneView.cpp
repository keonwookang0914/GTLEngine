#include "SceneView.h"
#include "Math/MathConstants.h"
#include "Math/ProjectionUtils.h"

FVector4 FSceneView::ProjectWorldToClip(const FVector &WorldPoint) const
{
    const FVector4 World4(WorldPoint.X, WorldPoint.Y, WorldPoint.Z, 1.0f);
    const FVector4 View4 = GetViewMatrix() * World4;
    const FVector4 Clip4 = GetProjectionMatrix() * View4;
    return Clip4;
}

// Returns true if the world-space point is inside the D3D clip volume
bool FSceneView::IsWorldPointVisible(const FVector &WorldPoint) const
{
    return IsInsideD3DClipSpace(ProjectWorldToClip(WorldPoint), Math::SmallNumber);
}