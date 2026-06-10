#pragma once
struct FVector;

struct FVector4
{
    float X;
    float Y;
    float Z;
    float W;

    FVector4() = default;
    FVector4(float x, float y, float z, float w);
    FVector4(const FVector &v, const float w);

    float Dot(const FVector4 &Other);
    float Length();
    float Length3();

    float LengthSquare();
    float LengthSquare3();

    FVector PerspectiveDivide() const;
    FVector XYZ() const;
};