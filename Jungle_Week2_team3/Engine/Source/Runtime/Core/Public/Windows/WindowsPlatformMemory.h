#pragma once
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformMemory.h"
#include "Runtime/Core/Public/HAL/Platform.h"

class FPlatformVirtualMemoryBlock : public FBasicVirtualMemoryBlock
{
  public:
    FPlatformVirtualMemoryBlock() {}
    FPlatformVirtualMemoryBlock(void *InPtr) : FBasicVirtualMemoryBlock(InPtr) {}
};

struct FWindowsPlatformMemory : public FGenericPlatformMemory
{
    /** Memory Pool Setup ... */
    static void Init();

    static FPlatformMemoryStats GetStats();

    static FPlatformVirtualMemoryBlock AllocateVirtual(SIZE_T Size);
};




