#pragma once
#include "ActorComponent.h"
#include "Math/Rotator.h"
#include "Math/Vector.h"
#include "Object.h"


class USceneComponent : public UActorComponent
{
  public:
    static UClass *StaticClass();

    const FVector  &GetRelativeLocation() const { return RelativeLocation; }
    const FRotator &GetRelativeRotation() const { return RelativeRotation; }
    const FVector  &GetRelativeScale3D() const { return RelativeScale3D; }

    void SetRelativeLocation(const FVector &InLocation) { RelativeLocation = InLocation; }
    void SetRelativeRotation(const FRotator &InRotation) { RelativeRotation = InRotation; }
    void SetRelativeScale3D(const FVector &InScale) { RelativeScale3D = InScale; }

  protected:
    FVector  RelativeLocation;
    FRotator RelativeRotation;
    FVector  RelativeScale3D = FVector(1.0f, 1.0f, 1.0f);
};