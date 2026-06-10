#include "PrimitiveSceneProxy.h"
#include "Components/PrimitiveComponent.h"

FPrimitiveSceneProxy::FPrimitiveSceneProxy(const UPrimitiveComponent *InComponent)
{
    Location = &InComponent->GetRelativeLocation();
    Rotation = &InComponent->GetRelativeRotation();
    Scale = &InComponent->GetRelativeScale3D();
}
