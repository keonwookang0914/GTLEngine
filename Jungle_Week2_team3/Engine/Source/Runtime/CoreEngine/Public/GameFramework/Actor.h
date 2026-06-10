#pragma once

#include "Containers/Array.h"
#include "Math/Rotator.h"
#include "Math/Vector.h"
#include "Object.h"


class USceneComponent;
class UWorld;

class AActor : public UObject
{
  public:
    virtual ~AActor() override;
    static UClass *StaticClass();

    int Test() override { return 2; }

    void    SetWorld(UWorld *InWorld) { World = InWorld; }
    UWorld *GetWorld() const;

    FVector  GetActorLocation();
    FRotator GetActorRotation();
    FVector  GetActorScale3D();

    void SetActorLocation(FVector InRelativeLocation);
    void SetActorRotation(FRotator InRelativeRotation);
    void SetActorScale3D(FVector InScale3D);

    void PostActorCreated();

    /**
     * 언리얼 기준 World - Level - Actor 로 Actor 의 OwingLevel 로부터 GetWorld 를 얻음.
     * 현재는 Level 구조가 따로 없으므로 우선 Actor 에 바로 위치시킴
     */
    UWorld *World = nullptr;

    USceneComponent *GetRootComponent() const;
    void             SetRootComponent(USceneComponent *InComponent);

  private:
    USceneComponent *RootComponent = nullptr;
};