#pragma once

#include "Class.h"
#include "Object.h"

class FObjectFactory
{
  public:
    static UObject *ConstructObject(UClass *Class);
};
