#include "StaticMeshSceneProxy.h"
#include "Components/CubeComp.h"
#include "Components/PlaneComp.h"
#include "Components/SphereComp.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineGlobals.h"
#include "Math/Matrix.h"
#include "StaticMeshResources.h"
#include "StaticMeshSceneProxy.h"

namespace
{
    FMatrix MakeScaleMatrix(const FVector &Scale)
    {
        return FMatrix(Scale.X, 0.0f, 0.0f, 0.0f, 0.0f, Scale.Y, 0.0f, 0.0f, 0.0f, 0.0f, Scale.Z,
                       0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    }
} // namespace

FStaticMeshSceneProxy::FStaticMeshSceneProxy(const UStaticMeshComponent *InComponent)
    : FPrimitiveSceneProxy(InComponent)
{
    RenderData = nullptr;
    UUID = 0;

    if (InComponent == nullptr)
    {
        return;
    }

    UUID = InComponent->UUID;

    UStaticMesh *StaticMesh = InComponent->GetStaticMesh();
    if (StaticMesh == nullptr)
    {
        return;
    }

    RenderData = StaticMesh->RenderData;
    Location = &InComponent->GetRelativeLocation();
    Scale = &InComponent->GetRelativeScale3D();
}

FStaticMeshSceneProxy::FStaticMeshSceneProxy(const UCubeComp *InComponent)
    : FPrimitiveSceneProxy(InComponent)
{
    RenderData = (GDefaultCube != nullptr) ? GDefaultCube->RenderData : nullptr;
    UUID = 0;

    if (InComponent == nullptr)
    {
        return;
    }

    UUID = InComponent->UUID;
    Location = &InComponent->GetRelativeLocation();
    Scale = &InComponent->GetRelativeScale3D();
}

FStaticMeshSceneProxy::FStaticMeshSceneProxy(const USphereComp *InComponent)
    : FPrimitiveSceneProxy(InComponent)
{
    RenderData = (GDefaultSphere != nullptr) ? GDefaultSphere->RenderData : nullptr;
    UUID = 0;

    if (InComponent == nullptr)
    {
        return;
    }

    UUID = InComponent->UUID;
    Location = &InComponent->GetRelativeLocation();
    Scale = &InComponent->GetRelativeScale3D();
}

FStaticMeshSceneProxy::FStaticMeshSceneProxy(const UPlaneComp *InComponent)
    : FPrimitiveSceneProxy(InComponent)
{
    RenderData = (GDefaultPlane != nullptr) ? GDefaultPlane->RenderData : nullptr;
    UUID = 0;

    if (InComponent == nullptr)
    {
        return;
    }

    UUID = InComponent->UUID;
    Location = &InComponent->GetRelativeLocation();
    Scale = &InComponent->GetRelativeScale3D();
    // PlaneComp에 별도 크기 프로퍼티가 없으면 일단 추가 스케일 없이 사용
    // 필요하면 아래처럼 Size 같은 멤버를 추가해서 반영 가능
    //
    const FVector Size = InComponent->Extent;
     ModelMatrix = ModelMatrix * MakeScaleMatrix(FVector(Size.X, Size.Y, 1.0f));
}
