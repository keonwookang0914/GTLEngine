#pragma once

#include "Engine/World.h"

class FEditorViewport;

class FEngine
{
  public:
    FEngine();
    ~FEngine();

    bool Init();
    void Shutdown();

    void Tick(float DeltaTime);

    void ClearWorld();

    UWorld       &GetWorld();
    const UWorld &GetWorld() const;

  private:
    UWorld World;
};