#pragma once

struct FVector
{
    float X;
    float Y;
    float Z;

    static const FVector Zero;

    FVector() = default;

    FVector(float x, float y, float z);

    float   Dot(const FVector &rhs) const;
    FVector Cross(const FVector &rhs) const;
    float   Length() const;
    void    Normalize();
    float   LengthSquare() const;

    FVector operator+(const FVector &rhs) const { return FVector{X + rhs.X, Y + rhs.Y, Z + rhs.Z}; }
    FVector &operator+=(const FVector &rhs)
    {
        X += rhs.X;
        Y += rhs.Y;
        Z += rhs.Z;
        return *this;
    }
    FVector operator-(const FVector &rhs) const { return FVector{X - rhs.X, Y - rhs.Y, Z - rhs.Z}; }

    FVector operator-() const { return FVector{-X, -Y, -Z}; }

    FVector &operator-=(const FVector &rhs)
    {
        X -= rhs.X;
        Y -= rhs.Y;
        Z -= rhs.Z;
        return *this;
    }

    FVector operator*(const float rhs) const { return FVector{X * rhs, Y * rhs, Z * rhs}; }

    friend FVector operator*(const float lhs, const FVector &rhs) { return rhs * lhs; }

    FVector &operator*=(const float rhs)
    {
        X *= rhs;
        Y *= rhs;
        Z *= rhs;
        return *this;
    }

    FVector operator/(const float rhs) const { return FVector{X / rhs, Y / rhs, Z / rhs}; }

    FVector &operator/=(const float rhs)
    {
        X /= rhs;
        Y /= rhs;
        Z /= rhs;
        return *this;
    }
};