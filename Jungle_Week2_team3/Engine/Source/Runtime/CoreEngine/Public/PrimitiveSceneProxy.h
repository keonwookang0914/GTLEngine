#pragma once

#include "Math/Matrix.h"
#include "Math/Rotator.h"
#include "Math/Vector.h"
#include "Math/MathUtility.h"
#include <cmath>

class UPrimitiveComponent;

class FPrimitiveSceneProxy
{
  public:
    FPrimitiveSceneProxy(const UPrimitiveComponent *InComponent);
    virtual ~FPrimitiveSceneProxy() = default;

    const FMatrix &GetModelMatrix()
    {
        FMatrix S = FMatrix::Identity;
        S.M[0][0] = Scale->X;
        S.M[1][1] = Scale->Y;
        S.M[2][2] = Scale->Z;

        const FMatrix R = FMatrix::MakeFromEuler(*Rotation);

        FMatrix T = FMatrix::Identity;
        T.M[0][3] = Location->X;
        T.M[1][3] = Location->Y;
        T.M[2][3] = Location->Z;

        ModelMatrix = T * R * S;
        return ModelMatrix;
    }

  protected:
    FMatrix        ModelMatrix = FMatrix::Identity;
    const FVector *Location = nullptr;
    const FRotator *Rotation = nullptr;
    const FVector  *Scale = nullptr;
};