#include "Math/Matrix.h"
#include "Math/Vector4.h"
#include "Math/Rotator.h"
#include <cmath>
#include <algorithm>
#include "Math/MathConstants.h"

const FMatrix FMatrix::Identity(1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f,
                                0.f, 1.f);
const FMatrix FMatrix::Zero(0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f,
                            0.f);

namespace
{
    float DegToRad(float InDegrees) { return InDegrees * Math::Pi / 180.0f; }
    float RadToDeg(float InRadians) { return InRadians * 180.0f / Math::Pi; }
} // namespace

FMatrix::FMatrix(float InMatrix[4][4])
{
    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            M[r][c] = InMatrix[r][c];
        }
    }
}

FMatrix::FMatrix(float M00, float M01, float M02, float M03, float M10, float M11, float M12,
                 float M13, float M20, float M21, float M22, float M23, float M30, float M31,
                 float M32, float M33)
{
    M[0][0] = M00; M[0][1] = M01; M[0][2] = M02; M[0][3] = M03;
    M[1][0] = M10; M[1][1] = M11; M[1][2] = M12; M[1][3] = M13;
    M[2][0] = M20; M[2][1] = M21; M[2][2] = M22; M[2][3] = M23;
    M[3][0] = M30; M[3][1] = M31; M[3][2] = M32; M[3][3] = M33;
}

FMatrix::FMatrix(const FVector4 &col1, const FVector4 &col2, const FVector4 &col3,
                 const FVector4 &col4)
{
    M[0][0] = col1.X; M[0][1] = col2.X; M[0][2] = col3.X; M[0][3] = col4.X;
    M[1][0] = col1.Y; M[1][1] = col2.Y; M[1][2] = col3.Y; M[1][3] = col4.Y;
    M[2][0] = col1.Z; M[2][1] = col2.Z; M[2][2] = col3.Z; M[2][3] = col4.Z;
    M[3][0] = col1.W; M[3][1] = col2.W; M[3][2] = col3.W; M[3][3] = col4.W;
}

void FMatrix::SetIdentity()
{
    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            M[r][c] = (r == c) ? 1.0f : 0.0f;
        }
    }
}

FMatrix FMatrix::operator*(const FMatrix &Other) const
{
    FMatrix Result;
    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            Result.M[r][c] = M[r][0] * Other.M[0][c] + M[r][1] * Other.M[1][c] +
                             M[r][2] * Other.M[2][c] + M[r][3] * Other.M[3][c];
        }
    }
    return Result;
}

FVector4 FMatrix::operator*(const FVector4 &V) const
{
    return FVector4(M[0][0] * V.X + M[0][1] * V.Y + M[0][2] * V.Z + M[0][3] * V.W,
                    M[1][0] * V.X + M[1][1] * V.Y + M[1][2] * V.Z + M[1][3] * V.W,
                    M[2][0] * V.X + M[2][1] * V.Y + M[2][2] * V.Z + M[2][3] * V.W,
                    M[3][0] * V.X + M[3][1] * V.Y + M[3][2] * V.Z + M[3][3] * V.W);
}

FMatrix FMatrix::MakeRotationX(float AngleRad)
{
    float S = std::sin(AngleRad);
    float C = std::cos(AngleRad);
    return FMatrix(1.f, 0.f, 0.f, 0.f, 
                   0.f, C, -S, 0.f, 
                   0.f, S, C, 0.f, 
                   0.f, 0.f, 0.f, 1.f);
}

FMatrix FMatrix::MakeRotationY(float AngleRad)
{
    float S = std::sin(AngleRad);
    float C = std::cos(AngleRad);
    return FMatrix(C, 0.f, S, 0.f, 
                   0.f, 1.f, 0.f, 0.f, 
                   -S, 0.f, C, 0.f, 
                   0.f, 0.f, 0.f, 1.f);
}

FMatrix FMatrix::MakeRotationZ(float AngleRad)
{
    float S = std::sin(AngleRad);
    float C = std::cos(AngleRad);
    return FMatrix(C, -S, 0.f, 0.f, 
                   S, C, 0.f, 0.f, 
                   0.f, 0.f, 1.f, 0.f, 
                   0.f, 0.f, 0.f, 1.f);
}

/**
 * Composition order: Yaw(Y) -> Pitch(X) -> Roll(Z)
 * This matches the Forward vector derivation: Forward = (cosP sinY, sinP, cosP cosY)
 */
FMatrix FMatrix::MakeFromEuler(const FRotator &Rotation)
{
    const float PitchRad = DegToRad(Rotation.Pitch);
    const float YawRad = DegToRad(Rotation.Yaw);
    const float RollRad = DegToRad(Rotation.Roll);

    // M = Ry(Yaw) * Rx(-Pitch) * Rz(-Roll)
    // To match ToRotator:
    // PitchRad = asin(M[1][2])
    // YawRad = atan2(M[0][2], M[2][2])
    // RollRad = atan2(-M[1][0], M[1][1])
    FMatrix Rx = MakeRotationX(-PitchRad);
    FMatrix Ry = MakeRotationY(YawRad);
    FMatrix Rz = MakeRotationZ(-RollRad);

    FMatrix M = Ry * Rx * Rz;
    return M;
}

FRotator FMatrix::ToRotator() const
{
    FRotator Rotation;

    const float sp = std::max(-1.0f, std::min(1.0f, M[1][2]));
    const float PitchRad = std::asin(sp);

    float YawRad = 0.0f;
    float RollRad = 0.0f;

    if (std::abs(std::cos(PitchRad)) > 0.0001f)
    {
        YawRad = std::atan2(M[0][2], M[2][2]);
        RollRad = std::atan2(-M[1][0], M[1][1]);
    }
    else
    {
        YawRad = 0.0f;
        RollRad = std::atan2(M[0][1], M[0][0]);
    }

    Rotation.Pitch = RadToDeg(PitchRad);
    Rotation.Yaw = RadToDeg(YawRad);
    Rotation.Roll = RadToDeg(RollRad);

    return Rotation;
}

FMatrix FMatrix::GetTranspose() const
{
    FMatrix Result;
    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            Result.M[r][c] = M[c][r];
        }
    }
    return Result;
}

FMatrix FMatrix::Inverse()
{
    float det = Determinant();
    if (std::fabs(det) < 0.000001f)
        return FMatrix::Identity;

    FMatrix res;
    float   invDet = 1.0f / det;

    for (int j = 0; j < 4; j++)
    {
        for (int i = 0; i < 4; i++)
        {
            int r0 = (i + 1) % 4, r1 = (i + 2) % 4, r2 = (i + 3) % 4;
            int c0 = (j + 1) % 4, c1 = (j + 2) % 4, c2 = (j + 3) % 4;

            float val = Minor3x3(r0, r1, r2, c0, c1, c2);
            res.M[j][i] = ((i + j) % 2 == 0 ? val : -val) * invDet;
        }
    }
    return res;
}

float FMatrix::Determinant() const
{
    return M[0][0] * Minor3x3(1, 2, 3, 1, 2, 3) - M[0][1] * Minor3x3(1, 2, 3, 0, 2, 3) +
           M[0][2] * Minor3x3(1, 2, 3, 0, 1, 3) - M[0][3] * Minor3x3(1, 2, 3, 0, 1, 2);
}

float FMatrix::Minor3x3(int32 r0, int32 r1, int32 r2, int32 c0, int32 c1, int32 c2) const
{
    float a = M[r0][c0], b = M[r0][c1], c = M[r0][c2];
    float d = M[r1][c0], e = M[r1][c1], f = M[r1][c2];
    float g = M[r2][c0], h = M[r2][c1], i = M[r2][c2];

    return a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
}
