#pragma once

#include "Math/Matrix.h"
#include "Math/Vector.h"

// TODO: GetViewMatrix -> GetViewMatrix
class FSceneView
{
  public:
    const FMatrix &GetViewMatrix() const { return ViewMatrix; }
    void           SetViewMatrix(const FMatrix &InViewMatrix) { ViewMatrix = InViewMatrix; }

    const FMatrix &GetProjectionMatrix() const { return ProjectionMatrix; }
    void           SetProjectionMatrix(const FMatrix &InProjectionMatrix)
    {
        ProjectionMatrix = InProjectionMatrix;
    }

    const FVector &GetEyePosition() const { return EyePosition; }
    void           SetEyePosition(const FVector &InEyePosition) { EyePosition = InEyePosition; }

    float GetCameraWidth() const { return ViewWidth; }
    void  SetCameraWidth(float InViewWidth) { ViewWidth = InViewWidth; }

    float GetCameraHeight() const { return ViewHeight; }
    void  SetCameraHeight(float InViewHeight) { ViewHeight = InViewHeight; }

    void SetCameraSize(float InViewWidth, float InViewHeight)
    {
        ViewWidth = InViewWidth;
        ViewHeight = InViewHeight;
    }

    FVector4 ProjectWorldToClip(const FVector &WorldPoint) const;
    bool     IsWorldPointVisible(const FVector &WorldPoint) const;

  private:
    FMatrix ViewMatrix;
    FMatrix ProjectionMatrix;
    FVector EyePosition = FVector(0.0f, 0.0f, 0.0f);
    float   ViewWidth = 1.0f;
    float   ViewHeight = 1.0f;
};