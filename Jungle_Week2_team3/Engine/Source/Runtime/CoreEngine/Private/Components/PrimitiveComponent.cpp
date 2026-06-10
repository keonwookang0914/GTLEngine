#include "Components/PrimitiveComponent.h"
#include "Class.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Runtime/Renderer/Public/Scene.h"
#include "EngineStatics.h"

 UPrimitiveComponent::UPrimitiveComponent() 
 { 
     UUID = UEngineStatics::GenUUID();
 }

UClass *UPrimitiveComponent::StaticClass()
{
    static UClass StaticCls("UPrimitiveComponent", USceneComponent::StaticClass(),
                            sizeof(UPrimitiveComponent));
    return &StaticCls;
}

void UPrimitiveComponent::OnRegister()
{
    AActor *Owner = GetOwner();
    UWorld *World = Owner->GetWorld();

    if (World)
    {
        FScene *Scene = World->GetScene();
        Scene->AddPrimitive(new FPrimitiveSceneProxy(this));
    }
}