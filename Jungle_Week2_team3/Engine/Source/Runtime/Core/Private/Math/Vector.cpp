#include "Math/Vector.h"
#include <cmath>

const FVector FVector::Zero{0.f, 0.f, 0.f};

FVector::FVector(float x, float y, float z) : X(x), Y(y), Z(z) {}

float FVector::Dot(const FVector &rhs) const { return (X * rhs.X + Y * rhs.Y + Z * rhs.Z); }

FVector FVector::Cross(const FVector &rhs) const
{
    return FVector{Y * rhs.Z - Z * rhs.Y, Z * rhs.X - X * rhs.Z, X * rhs.Y - Y * rhs.X};
}

float FVector::Length() const { return sqrtf(LengthSquare()); }

float FVector::LengthSquare() const { return (X * X + Y * Y + Z * Z); }

void FVector::Normalize()
{
    float Len = Length();
    if (Len > 0.0f)
    {
        X /= Len;
        Y /= Len;
        Z /= Len;
    }
}
