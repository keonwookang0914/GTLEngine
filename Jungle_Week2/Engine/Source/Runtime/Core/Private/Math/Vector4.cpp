#include "Math/Vector4.h"
#include "Math/Vector.h"
#include <cmath>

FVector4::FVector4(float x, float y, float z, float w) : X(x), Y(y), Z(z), W(w) {}

FVector4::FVector4(const FVector &v, const float w) : X(v.X), Y(v.Y), Z(v.Z), W(w) {}

float FVector4::Dot(const FVector4 &Other) { return X * Other.X + Y * Other.Y + Z * Other.Z; }

float FVector4::Length() { return sqrtf(LengthSquare()); }

float FVector4::Length3() { return sqrtf(LengthSquare3()); }

float FVector4::LengthSquare() { return X * X + Y * Y + Z * Z + W * W; }

float FVector4::LengthSquare3() { return (X * X + Y * Y + Z * Z); }

FVector FVector4::PerspectiveDivide() const { return {X / W, Y / W, Z / W}; }

FVector FVector4::XYZ() const { return {X, Y, Z}; }
