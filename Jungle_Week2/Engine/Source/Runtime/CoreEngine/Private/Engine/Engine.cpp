#include "Engine/Engine.h"
#include "Viewport/EditorViewport.h"
#include "EngineGlobals.h"

FEngine::FEngine() : World() { }

FEngine::~FEngine()
{
}

bool FEngine::Init()
{
    return true;
}

void FEngine::Shutdown() { ClearWorld(); }

void FEngine::Tick(float DeltaTime)
{
}

void FEngine::ClearWorld()
{
    World.ClearAll();
    World.SetScene(nullptr); //dangling 방지
}

UWorld &FEngine::GetWorld() { return World; }

const UWorld &FEngine::GetWorld() const { return World; }