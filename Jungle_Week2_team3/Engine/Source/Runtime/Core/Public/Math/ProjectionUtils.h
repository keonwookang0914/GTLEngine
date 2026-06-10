#pragma once
#include "Math/Vector4.h"

/*
    D3D NDC space
    - x: [-w, w]
    - y: [-w, w]
    - z: [0, w]
    
*/
bool IsInsideD3DClipSpace(const FVector4 &Clip, float Epsilon)
{
    if (Clip.W <= Epsilon)
    {
        return false;
    }

    return Clip.X >= -Clip.W && Clip.X <= Clip.W && Clip.Y >= -Clip.W && Clip.Y <= Clip.W &&
           Clip.Z >= 0.0f && Clip.Z <= Clip.W;
}