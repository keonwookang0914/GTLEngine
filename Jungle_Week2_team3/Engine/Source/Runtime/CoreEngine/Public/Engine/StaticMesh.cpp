#include "StaticMesh.h"

UStaticMesh::~UStaticMesh()
{
    if (RenderData)
    {
        delete RenderData;
        RenderData = nullptr;
    }
}

UClass* UStaticMesh::StaticClass()
{
    static UClass ObjectClass("UStaticMesh", UObject::StaticClass(), sizeof(UStaticMesh));
    return &ObjectClass;
}
