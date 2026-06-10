#pragma once
#include "Runtime/Core/Public/HAL/Platform.h"
struct FMemory
{
    static void *Malloc(SIZE_T Count);
    static void  Free(void *Original);
};
