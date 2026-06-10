#include "Engine/Source/Runtime/Core/CoreGlobals.h"
#include "Runtime/Core/Public/HAL/MemoryBase.h"
#include "Logging/LogOutputDevice.h"

bool GIsRequestingExit = false; /* Indicates that MainLoop() should be exited at the end of the current iteration*/

FLogOutputDevice *GLog = nullptr;
FMalloc *GMalloc = nullptr; /** Global Allocator */
