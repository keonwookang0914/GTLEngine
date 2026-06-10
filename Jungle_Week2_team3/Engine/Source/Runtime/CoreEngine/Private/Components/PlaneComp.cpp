#include "Components/PlaneComp.h"
#include "Class.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "StaticMeshSceneProxy.h"

 UPlaneComp::UPlaneComp() 
 { 
     SetClass(UPlaneComp::StaticClass());
     SetRelativeScale3D(Extent);
 }

UClass *UPlaneComp::StaticClass()
{
    static UClass StaticCls("UPlaneComp", UPrimitiveComponent::StaticClass(), sizeof(UPlaneComp));
    return &StaticCls;
}

void UPlaneComp::OnRegister()
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
