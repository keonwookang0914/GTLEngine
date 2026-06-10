#include "GameFramework/Actor.h"
#include "Class.h"
#include "Components/CubeComp.h"
#include "Components/PlaneComp.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComp.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineGlobals.h"
#include "Math/MathUtility.h"
#include "Object.h"
#include "StaticMeshResources.h"
#include "UObject/UObjectGlobals.h"

AActor::~AActor()
{
    if (RootComponent)
    {
        delete RootComponent;
    }
    RootComponent = nullptr;
}

UClass *AActor::StaticClass()
{
    static UClass ActorClass("AActor", UObject::StaticClass(), sizeof(AActor));
    return &ActorClass;
}

UWorld *AActor::GetWorld() const { return World; }

FVector AActor::GetActorLocation()
{
    return (RootComponent != nullptr) ? RootComponent->GetRelativeLocation() : FVector::Zero;
}

FRotator AActor::GetActorRotation()
{
    return (RootComponent != nullptr) ? RootComponent->GetRelativeRotation() : FRotator::Zero;
}

FVector AActor::GetActorScale3D()
{
    return (RootComponent != nullptr) ? RootComponent->GetRelativeScale3D() : FVector::Zero;
}

void AActor::SetActorLocation(FVector InRelativeLocation)
{
    if (RootComponent)
    {
        RootComponent->SetRelativeLocation(InRelativeLocation);
    }
}

void AActor::SetActorRotation(FRotator InRelativeRotation)
{
    if (RootComponent)
    {
        RootComponent->SetRelativeRotation(InRelativeRotation);
    }
}

void AActor::SetActorScale3D(FVector InScale3D)
{
    if (RootComponent)
    {
        RootComponent->SetRelativeScale3D(InScale3D);
    }
}

void AActor::PostActorCreated()
{
    // Default Root Component
    RootComponent = NewObject<USceneComponent>(USceneComponent::StaticClass());
    RootComponent->SetOwner(this);
}

USceneComponent *AActor::GetRootComponent() const { return RootComponent; }

void AActor::SetRootComponent(USceneComponent *InComponent)
{
    if (InComponent)
    {
        if (RootComponent)
        {

            InComponent->SetOwner(this);
            // 원래는 RootComponent 의 정보로 overwrite 하는거였는데, 지금은 InComponent 값을 그대로
            // 가져가게 함
            /*
            InComponent->SetOwner(RootComponent->GetOwner());
            InComponent->SetRelativeLocation(RootComponent->GetRelativeLocation());
            InComponent->SetRelativeRotation(RootComponent->GetRelativeRotation());
            InComponent->SetRelativeScale3D(RootComponent->GetRelativeScale3D());
            */
            RootComponent->DestroyComponent();
            delete RootComponent;
            RootComponent = nullptr;

            RootComponent = InComponent;
        }
        else
        {
            InComponent->SetOwner(this);
            RootComponent = InComponent;
        }

        RootComponent->RegisterComponent();
    }
    else
    {
        if (RootComponent)
        {
            // InComponent == nullptr 인 경우 RootComponent 해제

            RootComponent->DestroyComponent();
            delete RootComponent;
            RootComponent = nullptr;
        }
        else
        {
            // do nothing
        }
    }
}