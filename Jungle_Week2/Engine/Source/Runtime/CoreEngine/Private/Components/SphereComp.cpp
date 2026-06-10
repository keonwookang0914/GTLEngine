#include "Class.h"
#include "Components/SphereComp.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "StaticMeshSceneProxy.h"

 USphereComp::USphereComp() 
 { 
     SetClass(USphereComp::StaticClass()); 
     FVector Scale(Radius, Radius, Radius);
     SetRelativeScale3D(Scale);
 }

UClass *USphereComp::StaticClass()
{
    static UClass StaticCls("USphereComp", UPrimitiveComponent::StaticClass(), sizeof(USphereComp));
    return &StaticCls;
}

void USphereComp::OnRegister()
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
