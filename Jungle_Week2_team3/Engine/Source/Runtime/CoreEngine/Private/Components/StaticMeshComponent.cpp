#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "StaticMeshSceneProxy.h"

void UStaticMeshComponent::OnRegister()
{
    AActor *Owner = GetOwner();
    UWorld *World = Owner->GetWorld();

    if (World)
    {
        World->RegisterComponent(this);
        FScene *Scene = World->GetScene();
        Scene->AddPrimitive(new FStaticMeshSceneProxy(this));
    }
}

UClass* UStaticMeshComponent::StaticClass()
{
    static UClass StaticCls("UStaticMeshComponent", UMeshComponent::StaticClass(),
                            sizeof(USceneComponent));
    return &StaticCls;
}
