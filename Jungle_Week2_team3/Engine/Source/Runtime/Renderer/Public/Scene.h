#pragma once

#include "Components/PrimitiveComponent.h"
#include "Containers/Array.h"
#include "PrimitiveSceneProxy.h"

class FScene
{
  public:
    ~FScene()
    {
        for (auto &Proxy : Primitives)
        {
            delete Proxy;
        }    
        Primitives.clear();
    }
    TArray<FPrimitiveSceneProxy *> Primitives;

    void AddPrimitive(FPrimitiveSceneProxy *InPrimitiveSceneProxy)
    {
        Primitives.push_back(InPrimitiveSceneProxy);
    }
};