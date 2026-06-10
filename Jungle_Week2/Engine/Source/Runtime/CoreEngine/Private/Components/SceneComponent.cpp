#include "Components/SceneComponent.h"
#include "Class.h"

UClass *USceneComponent::StaticClass()
{
    static UClass StaticCls("USceneComponent", UActorComponent::StaticClass(),
                            sizeof(USceneComponent));
    return &StaticCls;
}
