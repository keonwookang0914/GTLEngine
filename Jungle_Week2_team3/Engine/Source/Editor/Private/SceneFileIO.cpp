#include "Class.h"
#include "Components/CubeComp.h"
#include "Components/PlaneComp.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComp.h"
#include "Editor/public/SceneFileIO.h"
#include "Engine/PrimitiveType.h"
#include "Engine/World.h"
#include "EngineGlobals.h"
#include "GameFramework/Actor.h"
#include "Runtime/Core/CoreGlobals.h"
#include "SceneJson.h"
#include "SceneRawData.h"
#include <EngineStatics.h>
#include <Logging/LogMacros.h>
#include "Runtime/Core/CoreGlobals.h"
#include "UObject/UObjectGlobals.h"

namespace
{
    static void ApplyTransformToSceneComponent(USceneComponent    *InComponent,
                                               const ActorRawData &InRawData)
    {
        if (InComponent == nullptr)
        {
            return;
        }

        InComponent->SetRelativeLocation(InRawData.Location);
        InComponent->SetRelativeRotation(InRawData.Rotation);
        InComponent->SetRelativeScale3D(InRawData.Scale);
    }

    static USceneComponent *CreatePrimitiveComponent(EPrimitiveType InType)
    {
        switch (InType)
        {
        case EPrimitiveType::Cube:
        {
            
            UCubeComp *CubeComp = NewObject<UCubeComp>(UCubeComp::StaticClass());
            if (CubeComp != nullptr)
            {
                CubeComp->BoxExtent = FVector(50.0f, 50.0f, 50.0f);
            }
            return CubeComp;
        }

        case EPrimitiveType::Sphere:
        {
            USphereComp *SphereComp = NewObject<USphereComp>(USphereComp::StaticClass());
            if (SphereComp != nullptr)
            {
                SphereComp->SetClass(USphereComp::StaticClass());
            }
            return SphereComp;
        }

        case EPrimitiveType::Plane:
        {
            UPlaneComp *PlaneComp = NewObject<UPlaneComp>(UPlaneComp::StaticClass());
            if (PlaneComp != nullptr)
            {
                PlaneComp->SetClass(UPlaneComp::StaticClass());
            }
            return PlaneComp;
        }

        case EPrimitiveType::None:
        default:
            return nullptr;
        }
    }

    static AActor *CreateActorFromRawData(const ActorRawData &InRawData)
    {
        AActor *NewActor = NewObject<AActor>(AActor::StaticClass());

        if (NewActor == nullptr)
        {
            delete NewActor;
            return nullptr;
        }

        NewActor->SetClass(AActor::StaticClass());
        NewActor->UUID = InRawData.UUID;

        if (!PrimitiveType::IsValid(InRawData.Type))
        {
            delete NewActor;
            return nullptr;
        }

        USceneComponent *NewRootComponent = CreatePrimitiveComponent(InRawData.Type);
        if (NewRootComponent == nullptr)
        {
            delete NewActor;
            delete NewRootComponent;
            return nullptr;
        }
        // NewActor->SetWorld(&GEngine->GetWorld());
        ApplyTransformToSceneComponent(NewRootComponent, InRawData);
        NewRootComponent->UUID = InRawData.UUID;
        NewActor->SetRootComponent(NewRootComponent);

        return NewActor;
    }

    static EPrimitiveType GetPrimitiveTypeFromComponentClass(const UClass *InClass)
    {
        if (InClass == nullptr)
        {
            return EPrimitiveType::None;
        }

        if (InClass->IsChildOf(UCubeComp::StaticClass()))
        {
            return EPrimitiveType::Cube;
        }

        if (InClass->IsChildOf(USphereComp::StaticClass()))
        {
            return EPrimitiveType::Sphere;
        }

        if (InClass->IsChildOf(UPlaneComp::StaticClass()))
        {
            return EPrimitiveType::Plane;
        }

        return EPrimitiveType::None;
    }

    static bool BuildRawDataFromActor(const AActor *InActor, ActorRawData &OutRawData)
    {
        if (InActor == nullptr)
        {
            return false;
        }

        const USceneComponent *RootComponent = InActor->GetRootComponent();
        if (RootComponent == nullptr)
        {
            return false;
        }

        const UClass *RootClass = RootComponent->GetClass();
        if (RootClass == nullptr)
        {
            return false;
        }

        const EPrimitiveType PrimitiveTypeValue = GetPrimitiveTypeFromComponentClass(RootClass);
        if (!PrimitiveType::IsValid(PrimitiveTypeValue))
        {
            return false;
        }

        OutRawData.UUID = InActor->GetRootComponent()->UUID;
        OutRawData.Location = RootComponent->GetRelativeLocation();
        OutRawData.Rotation = RootComponent->GetRelativeRotation();
        OutRawData.Scale = RootComponent->GetRelativeScale3D();
        OutRawData.Type = PrimitiveTypeValue;

        return true;
    }
} // namespace

bool FSceneFileIO::LoadSceneFromFile(const FString &FilePath, UWorld &OutWorld)
{
    SceneRawData LoadedScene;
    if (!SceneJson::LoadScene(FilePath, LoadedScene))
    {
        return false;
    }


    return ApplySceneRawData(LoadedScene, OutWorld);
}

bool FSceneFileIO::SaveSceneToFile(const FString &FilePath, const UWorld &InWorld)
{
    SceneRawData Scene;
    if (!BuildSceneRawData(InWorld, Scene))
    {
        return false;
    }

    return SceneJson::SaveScene(FilePath, Scene);
}

bool FSceneFileIO::ApplySceneRawData(const SceneRawData &InScene, UWorld &OutWorld)
{
    TArray<AActor *> SpawnedActors;
    SpawnedActors.reserve(InScene.Actors.size());

    for (const ActorRawData &RawActor : InScene.Actors)
    {
        AActor *NewActor = CreateActorFromRawData(RawActor);
        if (NewActor == nullptr)
        {
            for (AActor *SpawnedActor : SpawnedActors)
            {
                delete SpawnedActor;
            }
            return false;
        }
        SpawnedActors.push_back(NewActor);
    }

    OutWorld.ClearAll();
    OutWorld.AddActors(SpawnedActors);

    for (AActor *Actor : SpawnedActors)
    {
        Actor->SetWorld(&OutWorld);
        if (USceneComponent *Root = Actor->GetRootComponent())
        {
            Root->RegisterComponent();
        }
        UE_LOG(LogLoadScene, Log, "Spawn Actor [UUID: %d]", Actor->UUID);
    }

    UEngineStatics::SetNextUUID(InScene.NextUUID); // Set UUID
    return true;
}

bool FSceneFileIO::BuildSceneRawData(const UWorld &InWorld, SceneRawData &OutScene)
{
    OutScene = SceneRawData{};
    OutScene.Version = 1;
    OutScene.NextUUID = 0;

    const TArray<AActor *> &Actors = InWorld.GetActors();
    OutScene.Actors.reserve(Actors.size());

    uint32 MaxUUID = 0;

    for (const AActor *Actor : Actors)
    {
        ActorRawData RawData;
        if (!BuildRawDataFromActor(Actor, RawData))
        {
            return false;
        }

        if (RawData.UUID > MaxUUID)
        {
            MaxUUID = RawData.UUID;
        }

        OutScene.Actors.push_back(RawData);
    }

    OutScene.NextUUID = MaxUUID + 1;
    return true;
}