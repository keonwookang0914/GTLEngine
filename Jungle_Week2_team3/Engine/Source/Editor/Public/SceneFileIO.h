#pragma once

#include "SceneRawData.h"

class UWorld;

class FSceneFileIO
{
  public:
    static bool LoadSceneFromFile(const FString &FilePath, UWorld &OutWorld);
    static bool SaveSceneToFile(const FString &FilePath, const UWorld &InWorld);

    static bool ApplySceneRawData(const SceneRawData &InScene, UWorld &OutWorld);
    static bool BuildSceneRawData(const UWorld &InWorld, SceneRawData &OutScene);
};