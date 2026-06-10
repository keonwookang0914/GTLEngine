#include <windows.h>
#include "Runtime/Launch/Public/LaunchEngineLoop.h"
#include "Runtime/Core/Public/HAL/Platform.h"
#include "Runtime/Core/Public/HAL/PlatformMemory.h"
#include "Runtime/Core/Public/HAL/UnrealMemory.h"
#include "Runtime/Core/Public/HAL/MemoryBase.h"
#include "Runtime/Core/CoreGlobals.h"
#include "Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"

#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#include <d3d11.h>
#include <d3dcompiler.h>
#include <vector>

#include "Runtime/CoreUObject/Public/Object.h"
#include "GameFramework/Actor.h"

#include "Logging/LogMacros.h"
#include "Runtime/Core/CoreGlobals.h"


FEngineLoop GEngineLoop;

int32 Main(const TCHAR* CmdLine)
{
    LARGE_INTEGER StartTime, EndTime, Frequency;
    double        ElapsedTime = 0.0;
    bool          bLimitFrame = false;
    const int32   TargetFPS = 60;

    GEngineLoop.PreInit(CmdLine);
    GEngineLoop.Init();

    QueryPerformanceFrequency(&Frequency);

    while (!IsEngineExitRequested())
    {
        QueryPerformanceCounter(&StartTime);

        GEngineLoop.SetDeltaTime(ElapsedTime / 1000.0);
        GEngineLoop.Tick();

        if (bLimitFrame)
        {
            do
            {
                Sleep(0);

                QueryPerformanceCounter(&EndTime);

                ElapsedTime = (EndTime.QuadPart - StartTime.QuadPart) * 1000.0 / Frequency.QuadPart;
            } while (ElapsedTime < 1000.0 / TargetFPS);
        }
        else
        {
            QueryPerformanceCounter(&EndTime);
            ElapsedTime = (EndTime.QuadPart - StartTime.QuadPart) * 1000.0 / Frequency.QuadPart;
        }
    }

    GEngineLoop.Exit();

    return 0;
}

int32 LaunchWindowsStartup(HINSTANCE hInInstance, HINSTANCE hPrevInstance, char*, int32 nCmdShow, const TCHAR* CmdLine)
{
    // 환경 세팅
    FPlatformMemory::Init();
    GMalloc = new FMalloc;

    int32 ErrorLevel = Main(CmdLine);
    return ErrorLevel;
}


int32 WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int32 nShowCmd)
{
    int32 Result = LaunchWindowsStartup(hInstance, hPrevInstance, lpCmdLine, nShowCmd, nullptr);

    return Result;
}