#include "Runtime/CoreUObject/Public/ObjectFactory.h"
#include "UObject/UObjectGlobals.h"
#include "Runtime/CoreUObject/Public/Object.h"

UObject* FObjectFactory::ConstructObject(UClass* Class) 
{ 
	return NewObject<UObject>(Class);
}
