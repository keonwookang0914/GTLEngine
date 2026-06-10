#pragma once

#include "Editor.h"
#include "Engine/Source/Runtime/Renderer/Public/RendererModule.h"
#include "GenericPlatform/GenericApplication.h"
#include "HAL/Platform.h"
#include "Scene.h"
#include <vector>
#include <windows.h>

class FEngine;
class FGenericApplication;

class FEngineLoop
{
  public:
    FEngineLoop();
    virtual ~FEngineLoop() {}

  public:
    /**
     * Pre-Initialize the main loop - parse command line, sets up GIsEditor, etc.
     *
     * @param CmdLine The command line.
     * @return The error level; 0 if successful, > 0 if there were errors.
     **/
    int32 PreInit(const TCHAR *CmdLine);

    /**
     * Initialize the main loop (the rest of the initialization).
     *
     * @return The error level; 0 if successful, > 0 if there were errors.
     */
    virtual int32 Init();

    /** Performs shut down. */
    void Exit();

    /** Advances the main loop. */
    virtual void Tick();

    /**
     * Set DeltaTime.
     *
     * \param InDeltaTime : unit is second
     */
    void   SetDeltaTime(double InDeltaTime) { DeltaTime = InDeltaTime; }
    double GetDeltaTime() const { return DeltaTime; }

    int32 GetWindowWidth() const { return Application ? Application->GetWindowWidth() : 0; }
    int32 GetWindowHeight() const { Application ? Application->GetWindowHeight() : 0; }

  private:
    FGenericApplication *Application = nullptr;
    FEngine             *Engine = nullptr;
    FEditor             *Editor = nullptr;
    bool                 bIsRunning = false;

    /** 원래는 FApp 에서 관리하는데, 임시로 여기에 둠 */
    double DeltaTime = 0.0;
};