#include "Engine/Source/Runtime/Slate/Public/UIMaster.h"

SUIMaster::SUIMaster(FString name) : Name(name), bIsVisible(true) {}

SUIMaster::~SUIMaster() {}

void SUIMaster::Update(float deltaTime) {}

void SUIMaster::Show() { bIsVisible = true; }

void SUIMaster::Hide() { bIsVisible = false; }

bool SUIMaster::IsVisible() const { return bIsVisible; }

const FString SUIMaster::GetName() const { return Name; }
