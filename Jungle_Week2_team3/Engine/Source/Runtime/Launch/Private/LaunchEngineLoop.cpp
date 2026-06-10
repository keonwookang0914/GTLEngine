#include "LaunchEngineLoop.h"
#include "Editor.h"
#include "Editor/Public/SceneFileIO.h"
#include "Viewport/EditorViewport.h"
#include "EngineGlobals.h"
#include "HAL/Platform.h"
#include "Logging/LogMacros.h"
#include "Misc/UnrealString.h"
#include "Runtime/Core/CoreGlobals.h"
#include "Windows/WindowsApplication.h"
#include "UObject/UObjectGlobals.h"
#include "Runtime/CoreEngine/Public/GameFramework/Actor.h"


FEngineLoop::FEngineLoop() {}

int32 FEngineLoop::PreInit(const TCHAR *CmdLine)
{
    GEngine = new FEngine();
    GSceneView = new FSceneView();
    GRenderer = new FRendererModule();

    GDefaultCube = nullptr;
    GDefaultSphere = nullptr;
    GDefaultPlane = nullptr;

    if (GEngine == nullptr || GSceneView == nullptr || GRenderer == nullptr)
    {
        delete GRenderer;
        GRenderer = nullptr;

        delete GSceneView;
        GSceneView = nullptr;

        delete GEngine;
        GEngine = nullptr;

        return 1;
    }

    return 0;
}

int32 FEngineLoop::Init()
{
    bool bResult = false;

    Application = FWindowsApplication::Create();
    if (Application == nullptr)
    {
        return 1;
    }

    bResult = Application->CreateApplicationWindow(L"Jungle Techlab Week2", 1024, 1024);
    if (!bResult)
    {
        delete Application;
        Application = nullptr;
        return 1;
    }

    Application->ShowWindow();

    if (GEngine != nullptr)
    {
        GEngine->Init();
    }

    if (GRenderer == nullptr)
    {
        return 1;
    }

    GRenderer->StartupModule((HWND)Application->GetNativeWindowHandle());

    if (GEngine != nullptr)
    {
        GEngine->GetWorld().SetScene(GRenderer->GetScene());
    }

    if (GEngine != nullptr)
    {
        const FString DefaultScenePath = "Default.Scene";
        if (!FSceneFileIO::LoadSceneFromFile(DefaultScenePath, GEngine->GetWorld()))
        {
            UE_LOG(LogFile, Warning, "Failed to load startup scene: %s", DefaultScenePath.c_str());
        }
        else
        {
            UE_LOG(LogFile, Log, "Loaded startup scene: %s", DefaultScenePath.c_str());
        }
    }

    Editor = new FEditor();
    if (Editor == nullptr)
    {
        return 1;
    }

    Editor->Init();

    FEditorViewport       *EditorViewport = Editor->GetEditorViewport();
    FEditorViewportClient *EditorViewportClient = Editor->GetEditorViewportClient();

    if (EditorViewportClient != nullptr && GEngine != nullptr)
    {
        EditorViewportClient->SetWorld(&GEngine->GetWorld());
    }

    if (EditorViewport != nullptr)
    {
        Application->SetMessageHandler(EditorViewport);
    }

    UE_LOG(LogTemp, Log, "Hello Jungle %d!", 2026);

    return 0;
}

void InitStaticMesh() {}

void FEngineLoop::Exit()
{
    delete GDefaultSphere;
    delete GDefaultCube;
    delete GDefaultPlane;
    GDefaultSphere = nullptr;
    GDefaultCube = nullptr;
    GDefaultPlane = nullptr;

    if (Application != nullptr)
    {
        Application->SetMessageHandler(nullptr);
        Application->DestroyApplicationWindow();

        delete Application;
        Application = nullptr;
    }

    if (Editor != nullptr)
    {
        Editor->Exit();
        delete Editor;
        Editor = nullptr;
    }

    if (GRenderer)
    {
        GRenderer->ShutdownModule();
        delete GRenderer;
        GRenderer = nullptr;
    }

    delete GSceneView;
    GSceneView = nullptr;

    if (GEngine != nullptr)
    {
        GEngine->Shutdown();
        delete GEngine;
        GEngine = nullptr;
    }
}

void FEngineLoop::Tick()
{
    if (Application == nullptr)
    {
        return;
    }

    Application->PumpMessages();

    const float DeltaTime = GetDeltaTime();

    if (GEngine != nullptr)
    {
        GEngine->Tick(DeltaTime);
    }

    if (Editor != nullptr)
    {
        Editor->Tick(DeltaTime, Application->GetWindowWidth(), Application->GetWindowHeight());
    }

    if (GRenderer != nullptr && Editor != nullptr)
    {
        FEditorViewportClient *Client = Editor->GetEditorViewportClient();
        if (Client != nullptr)
        {
            const int32 Width = Application->GetWindowWidth();
            const int32 Height = Application->GetWindowHeight();

            GRenderer->BeginFrame();
            Client->Draw(static_cast<float>(Width), static_cast<float>(Height));
            GRenderer->EndFrame();
        }
    }
}