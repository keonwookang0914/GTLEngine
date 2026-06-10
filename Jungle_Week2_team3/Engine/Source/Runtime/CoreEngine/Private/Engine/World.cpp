#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "Logging/LogMacros.h"
#include "Runtime/Core/CoreGlobals.h"
#include "UObject/UObjectGlobals.h"
#include "Runtime/CoreEngine/Public/EngineGlobals.h"

UWorld::UWorld() = default;

UWorld::~UWorld()
{
    ClearAll();
}

void UWorld::AddActor(AActor *InActor)
{
    if (InActor == nullptr)
    {
        return;
    }

    Actors.push_back(InActor);
}

void UWorld::AddActors(const TArray<AActor *> &InActors)
{
    if (InActors.empty())
    {
        return;
    }

    Actors.reserve(Actors.size() + InActors.size());
    Actors.insert(Actors.end(), InActors.begin(), InActors.end());
}

void UWorld::RemoveActor(AActor *InActor)
{
    if (InActor == nullptr)
    {
        return;
    }

    for (auto It = Actors.begin(); It != Actors.end(); ++It)
    {
        if (*It == InActor)
        {
            Actors.erase(It);
            return;
        }
    }
}

void UWorld::ClearActors()
{
    for (AActor *Actor : Actors)
        delete Actor;
    Actors.clear();
}

void UWorld::ClearPrimitives()
{
    if (Scene == nullptr)
    {
        return;
    }
    if (Scene->Primitives.size() != 0)
    {
        for (FPrimitiveSceneProxy *Primitive : Scene->Primitives)
        {
            if (Primitive)
            {
                delete Primitive;
            }
        }
    }

    if (GRenderer)
    {
        GRenderer->GetMeshRenderer().ClearInstanceMap();
    }

    Scene->Primitives.clear();
}

void UWorld::ClearAll()
{
    ClearPrimitives();

    ClearActors();

    UUIDToComponentMap.clear();
}

void UWorld::AddPrimitive(FPrimitiveSceneProxy *InPrimitive)
{
    if (Scene == nullptr || InPrimitive == nullptr)
    {
        return;
    }

    Scene->AddPrimitive(InPrimitive);
}

TArray<AActor *> &UWorld::GetActors() { return Actors; }

const TArray<AActor *> &UWorld::GetActors() const { return Actors; }

AActor *UWorld::SpawnActor(UClass *Class, const FVector *Location)
{
    AActor *NewActor = NewObject<AActor>(Class);
    NewActor->SetWorld(this);
    NewActor->PostActorCreated();
    NewActor->GetRootComponent()->SetRelativeLocation(*Location);

    AddActor(NewActor);
    return NewActor;
}

FScene *UWorld::GetScene() const { return Scene; }

void UWorld::RegisterComponent(UPrimitiveComponent *InComponent)
{
    if (InComponent == nullptr)
    {
        return;
    }
    UUIDToComponentMap[InComponent->UUID] = InComponent;
}

UPrimitiveComponent *UWorld::FindComponentByUUID(uint32 InUUID)
{
    auto It = UUIDToComponentMap.find(InUUID);
    if (It != UUIDToComponentMap.end())
    {
        return It->second;
    }
    return nullptr;
}
