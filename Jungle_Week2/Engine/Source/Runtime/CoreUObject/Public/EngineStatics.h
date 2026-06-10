#pragma once

#include "HAL/Platform.h"

class UEngineStatics
{
  public:
    static uint32 GenUUID() { return NextUUID++; }

    static void SetNextUUID(uint32 InNextUUID)
    {    
        NextUUID = InNextUUID;   
    }

    static uint32 NextUUID;
};