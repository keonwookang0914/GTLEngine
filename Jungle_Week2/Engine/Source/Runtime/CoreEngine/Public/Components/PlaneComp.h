#pragma once
#include "PrimitiveComponent.h"

class UStaticMesh;

class UPlaneComp : public UPrimitiveComponent
{
  public:
    UPlaneComp();
    static UClass *StaticClass();

  public:

    void OnRegister() override;
    // todo: 삭제 고려 (아니면 기본값 설정)
    FVector Extent = {100.f, 100.f, 1.f};
};