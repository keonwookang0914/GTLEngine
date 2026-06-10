#pragma once

#include "Components.h"
#include "Containers/Array.h"
#include "HAL/Platform.h"
#include <d3d11.h>

class FStaticMeshRenderData
{
  public:
    ~FStaticMeshRenderData();

    bool IsValid() const { return Vertices.size() > 0; }

    void SetVertexBuffer(ID3D11Buffer *InVertexBuffer) { VertexBuffer = InVertexBuffer; }

  public:
    TArray<FStaticMeshBuildVertex> Vertices;
    ID3D11Buffer                  *VertexBuffer = nullptr;
};
