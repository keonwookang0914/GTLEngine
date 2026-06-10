#pragma once

#include <cstdint>
#include "Runtime/Core/Public/HAL/Platform.h"

class UObject;

class UClass
{
  public:
    UClass(const char *InName, const UClass *InSuperClass, SIZE_T InClassSize);

    const char   *GetName() const;
    const UClass *GetSuperClass() const;
    SIZE_T        GetClassSize() const;


    bool     IsChildOf(const UClass *Other) const;

  private:
    const char   *Name;
    const UClass *SuperClass;
    const SIZE_T  ClassSize;
};