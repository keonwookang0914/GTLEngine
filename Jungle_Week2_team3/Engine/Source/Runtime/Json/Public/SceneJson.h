#pragma once

#include "SceneRawData.h"
#include <string>

namespace SceneJson
{
    bool SaveScene(const FString &FilePath, const SceneRawData &Scene);
    bool LoadScene(const FString &FilePath, SceneRawData &OutScene);
} // namespace SceneJson