#pragma once
#include "SceneComponent.h"

class UPrimitiveComponent : public USceneComponent
{
  public:
    UPrimitiveComponent();
    static UClass *StaticClass();
    void           OnRegister() override;
};