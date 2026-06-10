#include "Object.h"

UObject::UObject() : ClassPrivate(UObject::StaticClass()) {}

UObject::~UObject() {}

UClass *UObject::StaticClass()
{
    static UClass ObjectClass("UObject", nullptr, sizeof(UObject));
    return &ObjectClass;
}

bool UObject::IsA(const UClass *OtherClass) 
{ return GetClass()->IsChildOf(OtherClass); }
