#include "Class.h"

UClass::UClass(const char *InName, const UClass *InSuperClass, SIZE_T InClassSize)
    : Name(InName), SuperClass(InSuperClass), ClassSize(InClassSize)
{
}

const char *UClass::GetName() const { return Name; }

const UClass *UClass::GetSuperClass() const { return SuperClass; }

SIZE_T UClass::GetClassSize() const { return ClassSize; }

bool UClass::IsChildOf(const UClass *Other) const
{
    if (Other == nullptr)
    {
        return false;
    }

    const UClass *Current = this;
    while (Current != nullptr)
    {
        if (Current == Other)
        {
            return true;
        }

        Current = Current->GetSuperClass();
    }

    return false;
}