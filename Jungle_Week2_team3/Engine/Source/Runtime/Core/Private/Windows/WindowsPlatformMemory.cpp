#include "Runtime/Core/Public/Windows/WindowsPlatformMemory.h"

void FWindowsPlatformMemory::Init()
{ /** 현재는 아무것도 안함 */ }

FPlatformMemoryStats FWindowsPlatformMemory::GetStats() 
{ return FPlatformMemoryStats(); }

FPlatformVirtualMemoryBlock AllocateVirtual(SIZE_T Size) { return FPlatformVirtualMemoryBlock(); }
