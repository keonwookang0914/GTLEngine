#pragma once
#include "Ray.h"
#include "Vector.h"

inline bool IntersectRayAABB(const FRay &Ray, const FVector &BoxMin, const FVector &BoxMax, float &OutT)
{
    constexpr float Epsilon = 0.000001f;

    float TMin = 0.0f;
    float TMax = (std::numeric_limits<float>::max)();

    auto UpdateSlab = [&](float Origin, float Direction, float MinV, float MaxV) -> bool
    {
        if (std::fabs(Direction) <= Epsilon)
        {
            return (Origin >= MinV && Origin <= MaxV);
        }

        const float InvD = 1.0f / Direction;
        float       T0 = (MinV - Origin) * InvD;
        float       T1 = (MaxV - Origin) * InvD;

        if (T0 > T1)
        {
            const float Temp = T0;
            T0 = T1;
            T1 = Temp;
        }

        TMin = TMin < T0 ? T0 : TMin;
        TMax = TMax > T1 ? T1 : TMax;

        return TMax >= TMin;
    };

    if (!UpdateSlab(Ray.Origin.X, Ray.Direction.X, BoxMin.X, BoxMax.X))
    {
        return false;
    }

    if (!UpdateSlab(Ray.Origin.Y, Ray.Direction.Y, BoxMin.Y, BoxMax.Y))
    {
        return false;
    }

    if (!UpdateSlab(Ray.Origin.Z, Ray.Direction.Z, BoxMin.Z, BoxMax.Z))
    {
        return false;
    }

    OutT = TMin;
    return true;
}

inline bool IntersectRayTriangle(const FRay &Ray, const FVector &V0, const FVector &V1, const FVector &V2, float &OutT)
{
    constexpr float Epsilon = 0.000001f;

    const FVector E1 = V1 - V0;
    const FVector E2 = V2 - V0;

    const FVector P = Ray.Direction.Cross(E2);
    const float   Det = E1.Dot(P);

    if (std::fabs(Det) <= Epsilon)
    {
        return false;
    }

    const float   InvDet = 1.0f / Det;
    const FVector T = Ray.Origin - V0;

    const float U = T.Dot(P) * InvDet;
    if (U < 0.0f || U > 1.0f)
    {
        return false;
    }

    const FVector Q = T.Cross(E1);
    const float   V = Ray.Direction.Dot(Q) * InvDet;
    if (V < 0.0f || (U + V) > 1.0f)
    {
        return false;
    }

    const float HitT = E2.Dot(Q) * InvDet;
    if (HitT <= Epsilon)
    {
        return false;
    }

    OutT = HitT;
    return true;
}