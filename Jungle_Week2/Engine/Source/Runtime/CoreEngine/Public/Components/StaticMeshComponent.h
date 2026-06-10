#pragma once
#include "MeshComponent.h"

class UStaticMesh;

class UStaticMeshComponent : public UMeshComponent
{
  public:
    UStaticMesh *StaticMesh = nullptr;

  public:
    static UClass *StaticClass();

    void SetStaticMesh(UStaticMesh *InMesh) { StaticMesh = InMesh; }

    UStaticMesh *GetStaticMesh() const { return StaticMesh; }

    void OnRegister() override;
};