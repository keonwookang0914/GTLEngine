#pragma once

#include "Containers/Array.h"
#include "Math/Vector.h"
#include "PrimitiveSceneProxy.h"
#include "Runtime/Renderer/Public/Scene.h"
#include <map>
#include <unordered_map>

class AActor;
class UClass;
class UPrimitiveComponent;

class UWorld
{
  public:
    UWorld();
    ~UWorld();

    void AddActor(AActor *InActor);
    void AddActors(const TArray<AActor *> &InActors);
    void RemoveActor(AActor *InActor);
    void ClearActors();
    void ClearPrimitives();
    void ClearAll();

    void AddPrimitive(FPrimitiveSceneProxy *InPrimitive);

    TArray<AActor *>       &GetActors();
    const TArray<AActor *> &GetActors() const;

    /** Rotation 등 Transform 은 향후 수정 */
    AActor *SpawnActor(UClass *Class, const FVector *Location);

    FScene *GetScene() const;
    void    SetScene(FScene *InScene) { Scene = InScene; }

    void                 RegisterComponent(UPrimitiveComponent *InComponent);
    UPrimitiveComponent *FindComponentByUUID(uint32 InUUID);

  private:
    TArray<AActor *> Actors;
    FScene                        *Scene;

    std::map<uint32, UPrimitiveComponent *> UUIDToComponentMap;
};