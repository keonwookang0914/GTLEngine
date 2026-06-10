#include "Runtime/Core/Public/HAL/UnrealMemory.h"
#include "Runtime/Core/Public/HAL/MemoryBase.h"
#include "Runtime/Core/CoreGlobals.h"
#include <memory>

void *FMemory::Malloc(SIZE_T Count) { return GMalloc->Malloc(Count); }

void FMemory::Free(void *Original) { GMalloc->Free(Original); }

void *operator new(size_t Size) 
{ 
	if (GMalloc)
	{
        return FMemory::Malloc(Size); 
	}

	return std::malloc(Size);
}

void operator delete(void *Ptr) noexcept 
{ 
	FMemory::Free(Ptr); 
}

void *operator new[](size_t Size) 
{
    if (GMalloc)
        return FMemory::Malloc(Size);

	return std::malloc(Size);
}

void operator delete[](void *Ptr) noexcept 
{ 
	FMemory::Free(Ptr); 
}
