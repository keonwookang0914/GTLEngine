#pragma once

#include "HAL/Platform.h"
#include "Misc/UnrealString.h"


enum class EPrimitiveType : uint8
{
    None = 0,
    Sphere,
    Cube,
    Plane,
    Count
};

struct FPrimitiveTypeEntry
{
    EPrimitiveType Type;
    const char    *DisplayName;
    const char    *JsonName;
};

/**
 * @brief 메모: Plane을 json에서는 'triangle'로 명명하므로 Display와 Json의 처리 함수를 구분
 */
// TODO: 추후 이 클래스 이용하여 에디터에 Primitive 이름 띄울 것임. (매핑된 문자열 사용)
namespace PrimitiveType
{
    bool IsValid(EPrimitiveType Type);

    const char *ToDisplayString(EPrimitiveType Type);
    const char *ToJsonString(EPrimitiveType Type);

    bool FromDisplayString(const FString &InString, EPrimitiveType &OutType);
    bool FromJsonString(const FString &InString, EPrimitiveType &OutType);

    const FPrimitiveTypeEntry *FindByType(EPrimitiveType Type);
    const FPrimitiveTypeEntry *FindByDisplayString(const FString &InString);
    const FPrimitiveTypeEntry *FindByJsonString(const FString &InString);
} // namespace PrimitiveType