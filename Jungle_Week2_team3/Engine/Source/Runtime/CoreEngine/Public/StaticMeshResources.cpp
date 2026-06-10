#include "StaticMeshResources.h"

FStaticMeshRenderData::~FStaticMeshRenderData() 
{
    if (VertexBuffer)
    {
        VertexBuffer->Release();
        VertexBuffer = nullptr;
    }

    Vertices.clear();    
}
