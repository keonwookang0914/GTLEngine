#include "Components/CubeComp.h"
#include "Class.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "StaticMeshSceneProxy.h"

UCubeComp::UCubeComp() 
{ 
    SetClass(UCubeComp::StaticClass()); 
    
    // BoxExtent가 "반쪽 크기"라면 실제 전체 크기를 맞추기 위해 2배
    FVector Scale(BoxExtent * 2.0f);
    SetRelativeScale3D(Scale);
}

UClass *UCubeComp::StaticClass()
{
    static UClass StaticCls("UCubeComp", UPrimitiveComponent::StaticClass(), sizeof(UCubeComp));
    return &StaticCls;
}

void UCubeComp::OnRegister()
{
    AActor *Owner = GetOwner();
    UWorld *World = Owner->GetWorld();

    if (World)
    {
        World->RegisterComponent(this);
        FScene *Scene = World->GetScene();
        Scene->AddPrimitive(static_cast<FPrimitiveSceneProxy *>(new FStaticMeshSceneProxy(this)));
    }
}
