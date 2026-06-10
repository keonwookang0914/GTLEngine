#pragma once

#include "Containers/Array.h"
#include "Engine/PrimitiveType.h"
#include "HAL/Platform.h"
#include "Math/Rotator.h"
#include "Math/Vector.h"

struct ActorRawData
{
    uint32         UUID;
    FVector        Location;
    FRotator       Rotation;
    FVector        Scale;
    EPrimitiveType Type;

    ActorRawData()
        : UUID(0), Location(0.0f, 0.0f, 0.0f), Rotation(0.0f, 0.0f, 0.0f), Scale(1.0f, 1.0f, 1.0f),
          Type(EPrimitiveType::None)
    {
    }
};

struct SceneRawData
{
    uint32               Version;
    uint32               NextUUID;
    TArray<ActorRawData> Actors;

    SceneRawData() : Version(1), NextUUID(0), Actors() {}
};