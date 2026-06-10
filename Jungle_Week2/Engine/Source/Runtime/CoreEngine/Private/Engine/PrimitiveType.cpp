#include "Engine/PrimitiveType.h"

namespace
{
    static constexpr FPrimitiveTypeEntry GPrimitiveTypeTable[] = {
        {EPrimitiveType::None, "None", "None"},
        {EPrimitiveType::Sphere, "Sphere", "Sphere"},
        {EPrimitiveType::Cube, "Cube", "Cube"},
        {EPrimitiveType::Plane, "Plane", "Triangle"},
    };

    static_assert(sizeof(GPrimitiveTypeTable) / sizeof(GPrimitiveTypeTable[0]) ==
                      static_cast<uint8>(EPrimitiveType::Count),
                  "Primitive type table size must match EPrimitiveType.");

    constexpr uint8 GetPrimitiveTypeCount() { return static_cast<uint8>(EPrimitiveType::Count); }
} // namespace

namespace PrimitiveType
{
    bool IsValid(EPrimitiveType Type)
    {
        return Type > EPrimitiveType::None && Type < EPrimitiveType::Count;
    }

    const FPrimitiveTypeEntry *FindByType(EPrimitiveType Type)
    {
        const uint8 Index = static_cast<uint8>(Type);
        if (Index >= GetPrimitiveTypeCount())
        {
            return nullptr;
        }

        return &GPrimitiveTypeTable[Index];
    }

    const FPrimitiveTypeEntry *FindByDisplayString(const FString &InString)
    {
        for (uint8 Index = 1; Index < GetPrimitiveTypeCount(); ++Index)
        {
            const FPrimitiveTypeEntry &Entry = GPrimitiveTypeTable[Index];
            if (InString == Entry.DisplayName)
            {
                return &Entry;
            }
        }

        return nullptr;
    }

    const FPrimitiveTypeEntry *FindByJsonString(const FString &InString)
    {
        for (uint8 Index = 1; Index < GetPrimitiveTypeCount(); ++Index)
        {
            const FPrimitiveTypeEntry &Entry = GPrimitiveTypeTable[Index];
            if (InString == Entry.JsonName)
            {
                return &Entry;
            }
        }

        return nullptr;
    }

    const char *ToDisplayString(EPrimitiveType Type)
    {
        const FPrimitiveTypeEntry *Entry = FindByType(Type);
        return Entry ? Entry->DisplayName : nullptr;
    }

    const char *ToJsonString(EPrimitiveType Type)
    {
        const FPrimitiveTypeEntry *Entry = FindByType(Type);
        return Entry ? Entry->JsonName : nullptr;
    }

    bool FromDisplayString(const FString &InString, EPrimitiveType &OutType)
    {
        const FPrimitiveTypeEntry *Entry = FindByDisplayString(InString);
        if (Entry == nullptr)
        {
            OutType = EPrimitiveType::None;
            return false;
        }

        OutType = Entry->Type;
        return true;
    }

    bool FromJsonString(const FString &InString, EPrimitiveType &OutType)
    {
        const FPrimitiveTypeEntry *Entry = FindByJsonString(InString);
        if (Entry == nullptr)
        {
            OutType = EPrimitiveType::None;
            return false;
        }

        OutType = Entry->Type;
        return true;
    }
} // namespace PrimitiveType