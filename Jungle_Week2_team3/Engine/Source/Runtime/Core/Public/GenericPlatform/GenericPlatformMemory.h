#pragma once
#include "Runtime/Core/Public/HAL/Platform.h"

/** 원래는 OS 에 따라 달라지는데 당장은 여기서 확정지어둠 */
struct FPlatformMemoryStats
{
    uint32 TotalAllocationBytes;
    uint32 TotalAllocationCount;
};

class FBasicVirtualMemoryBlock
{
  public:
    /** 단순화 함 */
    void *Ptr;

    FBasicVirtualMemoryBlock() : Ptr(nullptr) {}
    FBasicVirtualMemoryBlock(void *InPtr) : Ptr(InPtr) {}
};

/** Generic implementation for most platforms, these tend to be unused and unimplemented. */
struct FGenericPlatformMemory
{
    /** Initializes platform memory specific constants. */
    static void Init();


    static FPlatformMemoryStats GetStats();
};




