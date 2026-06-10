#pragma once

#include "Math/Rotator.h"
#include "Math/Vector.h"
#include "Viewport/EditorViewportState.h"
#include <cmath>

namespace FCameraMath
{
    const float Pi = 3.14159265358979323846f;

    inline float DegToRad(float InDegrees) { return InDegrees * Pi / 180.0f; }

    inline FVector GetWorldUp() { return FVector(0.0f, 1.0f, 0.0f); }

    inline FVector GetForward(const FViewportCameraTransform &InTransform)
    {
        const FRotator &Rotation = InTransform.GetRotation();

        float YawRad = DegToRad(Rotation.Yaw);
        float PitchRad = DegToRad(Rotation.Pitch);

        float CY = std::cos(YawRad);
        float SY = std::sin(YawRad);
        float CP = std::cos(PitchRad);
        float SP = std::sin(PitchRad);

        FVector Forward(CP * SY, SP, CP * CY);
        Forward.Normalize();
        return Forward;
    }

    inline FVector GetRight(const FViewportCameraTransform &InTransform)
    {
        const FVector WorldUp = GetWorldUp();
        const FVector Forward = GetForward(InTransform);

        FVector Right = WorldUp.Cross(Forward);
        if (Right.LengthSquare() <= 0.000001f)
        {
            Right = FVector(1.0f, 0.0f, 0.0f);
        }

        Right.Normalize();
        return Right;
    }

    inline FVector GetUp(const FViewportCameraTransform &InTransform)
    {
        const FVector Forward = GetForward(InTransform);
        const FVector Right = GetRight(InTransform);

        FVector Up = Forward.Cross(Right);
        if (Up.LengthSquare() <= 0.000001f)
        {
            Up = GetWorldUp();
        }

        Up.Normalize();
        return Up;
    }

    inline void GetCameraBasis(const FViewportCameraTransform &InTransform, FVector &OutEye,
                               FVector &OutForward, FVector &OutRight, FVector &OutUp)
    {
        OutEye = InTransform.GetLocation();
        OutForward = GetForward(InTransform);
        OutRight = GetRight(InTransform);
        OutUp = GetUp(InTransform);
    }
} // namespace FCameraMath
