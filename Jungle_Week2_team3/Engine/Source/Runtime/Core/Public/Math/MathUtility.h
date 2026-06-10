#pragma once
#include "HAL/Platform.h"

uint32 RoundUpToPowerOfTwo(uint32 v);
float DegreesToRadians(float Deg);

template<typename T> 
T Clamp(T* value, T min, T max)
{
    if (*value < min)
        *value = min;
    else if (*value > max)
        *value = max;
    return *value;
}
