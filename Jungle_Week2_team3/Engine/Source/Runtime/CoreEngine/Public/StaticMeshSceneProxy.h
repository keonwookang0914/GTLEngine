#pragma once
#include "Components/CubeComp.h"
#include "Components/PlaneComp.h"
#include "Components/SphereComp.h"
#include "Math/Vector.h"
#include "PrimitiveSceneProxy.h"

class UStaticMeshComponent;
class FStaticMeshRenderData;

class FStaticMeshSceneProxy : public FPrimitiveSceneProxy
{
  public:
    FStaticMeshSceneProxy(const UStaticMeshComponent *InComponent);
    FStaticMeshSceneProxy(const UCubeComp *InComponent);
    FStaticMeshSceneProxy(const USphereComp *InComponent);
    FStaticMeshSceneProxy(const UPlaneComp *InComponent);

    const FStaticMeshRenderData *RenderData = nullptr;
    uint32                       UUID = 0;

  private:
};