#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

void UActorComponent::RegisterComponent() 
{ 
	if (!IsRegistered())
	{
        AActor *Owner = GetOwner();
        UWorld *World = Owner->GetWorld();

        if (World == nullptr)
            return;
        
        OnRegister();
        bRegistered = true;
	}
}

bool UActorComponent::IsRegistered() const { return bRegistered; }

UClass *UActorComponent::StaticClass()
{
    static UClass ObjectClass("UActorComponent", UObject::StaticClass(),
                              sizeof(UActorComponent));
    return &ObjectClass;
}