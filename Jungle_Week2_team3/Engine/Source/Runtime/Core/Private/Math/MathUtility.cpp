#include "Math/MathUtility.h"

uint32 RoundUpToPowerOfTwo(uint32 v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}


float DegreesToRadians(float Deg)
{ return Deg * 0.01745329251994329577f; }
