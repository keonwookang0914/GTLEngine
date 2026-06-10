#pragma once

#include "Class.h"
#include "HAL/Platform.h"

class UObject
{
  public:
    UObject();
    virtual ~UObject();

    virtual int Test() { return 1; }

    static UClass *StaticClass();

    const UClass *GetClass() const { return ClassPrivate; }
    void    SetClass(const UClass *InClass) { ClassPrivate = InClass; }

    bool IsA(const UClass *OtherClass);

  public:
    uint32 UUID = 0;
    uint32 InternalIndex = 0;

  private:
    const UClass *ClassPrivate = nullptr;

    static void *operator new(SIZE_T) = delete;
};