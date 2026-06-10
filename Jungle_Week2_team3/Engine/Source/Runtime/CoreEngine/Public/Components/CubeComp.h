#pragma once
#include "PrimitiveComponent.h"

class UStaticMesh;

class UCubeComp : public UPrimitiveComponent
{
  public:
    UCubeComp();

    static UClass *StaticClass();

  public:
    void OnRegister() override;
    // todo: 기본값 설정
    FVector BoxExtent = {50.f, 50.f, 50.f};
};