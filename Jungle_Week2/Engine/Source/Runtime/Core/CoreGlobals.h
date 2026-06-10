#pragma once

class FMalloc;

class FLogOutputDevice;

extern bool GIsRequestingExit;

inline bool IsEngineExitRequested()
{ return GIsRequestingExit; }

extern FMalloc *GMalloc;

extern FLogOutputDevice *GLog;
