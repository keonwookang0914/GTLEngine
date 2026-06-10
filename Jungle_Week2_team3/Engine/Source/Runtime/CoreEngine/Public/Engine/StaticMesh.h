#pragma once

#include "Containers/Array.h"
#include "Math/Vector.h"
#include "Object.h"
#include "StaticMeshResources.h"

class UStaticMesh : public UObject
{
  public:
    ~UStaticMesh();
    FStaticMeshRenderData *RenderData;

    static UClass *StaticClass();

    void Initialize() { RenderData = new FStaticMeshRenderData(); }

  public:
    bool IsValid() const { return RenderData && RenderData->IsValid(); }
};