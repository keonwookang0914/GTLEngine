#pragma once
#include "HAL/Platform.h"
#include "Vector4.h"

struct FVector4;
struct FRotator;

struct FMatrix
{
    float M[4][4];

    static const FMatrix Identity;
    static const FMatrix Zero;

    FMatrix() = default;
    FMatrix(float InMatrix[4][4]);
    FMatrix(float M00, float M01, float M02, float M03, float M10, float M11, float M12, float M13,
            float M20, float M21, float M22, float M23, float M30, float M31, float M32, float M33);
    FMatrix(const FVector4 &col1, const FVector4 &col2, const FVector4 &col3, const FVector4 &col4);

    void SetIdentity();

    FMatrix operator*(const FMatrix &Other) const;
    FVector4 operator*(const FVector4 &V) const;

    static FMatrix MakeRotationX(float AngleRad);
    static FMatrix MakeRotationY(float AngleRad);
    static FMatrix MakeRotationZ(float AngleRad);
    static FMatrix MakeFromEuler(const struct FRotator &Rotation);
    struct FRotator ToRotator() const;

    float   Determinant() const;
    FMatrix Inverse();
    FMatrix GetTranspose() const;

  private:
    float Minor3x3(int32 r0, int32 r1, int32 r2, int32 c0, int32 c1, int32 c2) const;
};
