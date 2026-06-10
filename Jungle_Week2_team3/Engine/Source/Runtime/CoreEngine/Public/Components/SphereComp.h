#pragma once
#include "PrimitiveComponent.h"

class UStaticMesh;

class USphereComp : public UPrimitiveComponent
{
  public:
    USphereComp();
    static UClass *StaticClass();

  public:
    void OnRegister() override;
    float Radius = 50.f;
};