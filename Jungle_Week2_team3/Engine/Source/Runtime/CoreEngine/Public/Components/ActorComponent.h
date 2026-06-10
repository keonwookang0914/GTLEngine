#pragma once
#include "Math/Vector.h"
#include "Object.h"

class AActor;

class UActorComponent : public UObject
{
  public:
    static UClass *StaticClass();

    void    SetOwner(AActor *InOwner) { Owner = InOwner; }
    AActor *GetOwner() const { return Owner; }
    void    RegisterComponent();

    // 우선은 구현 안했는데, 해제할 자원들 넣기
    virtual void DestroyComponent() {}

    virtual void OnRegister() {}

    bool IsRegistered() const;

  public:
    AActor *Owner;
    bool    bRegistered = false;
};