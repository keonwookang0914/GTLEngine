#pragma once
#include "Runtime/Core/Public/HAL/UnrealMemory.h"
#include "Runtime/CoreUObject/Public/Class.h"
#include "EngineStatics.h"
#include <cassert>
#include <new>
#include <type_traits>

template <typename T> T *NewObject(const UClass *Class)
{
    // 최종 결과물 크기가 생성할 메모리보다 작거나 같아야함
    assert(sizeof(T) <= Class->GetClassSize());
    //static_assert(std::is_base_of_v<UObject, T> == true)
    
    void *Memory = FMemory::Malloc(Class->GetClassSize());
    T    *Object = ::new (Memory) T();
    Object->SetClass(Class);

    // 생성자를 호출해 할당받은 메모리 초기화
    return Object;
    
}