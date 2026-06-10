#include "Editor.h"
#include "Panels/ControlPanel.h"
#include "Panels/PropertyPanel.h"
#include "Panels/StatPanel.h"
#include "Viewport/EditorViewport.h"
#include "Viewport/EditorViewportClient.h"
#include "EngineGlobals.h"
#include "Panels/OutputLog.h"
#include <Runtime/Core/CoreGlobals.h>
#include <UIManager.h>

void FEditor::Init()
{
    EditorViewport = new FEditorViewport();
    EditorViewportClient = new FEditorViewportClient();

    EditorViewportClient->SetSceneView(GSceneView);
    EditorViewportClient->SetWorld(&GEngine->GetWorld());
    EditorViewport->SetEditorViewportClient(EditorViewportClient);

    ControlPanel = new SControlPanel("Jungle Control Panel");
    ControlPanel->SetViewportClient(EditorViewportClient);
    ControlPanel->SetSize(530, 330);
    ControlPanel->SetPosition(7, 7);

    PropertyPanel = new SPropertyPanel("Jungle Property Window");
    PropertyPanel->SetCameraportClient(EditorViewportClient);
    PropertyPanel->SetPosition(636, 7);
    PropertyPanel->SetSize(300, 200);

    StatPanel = new SStatPanel("Jungle Stat Panel");
    PropertyPanel->SetPosition(776, 232);
    PropertyPanel->SetSize(249, 188);

    OutputLog = new SOutputLog("Jungle Console Log");
    OutputLog->SetSize(1000, 300);
    OutputLog->SetPosition(12, 720);

    SUIManager::Get().RegisterPanel(ControlPanel);
    SUIManager::Get().RegisterPanel(PropertyPanel);
    SUIManager::Get().RegisterPanel(StatPanel);
    SUIManager::Get().RegisterPanel(OutputLog);

    GLog = OutputLog;
}

void FEditor::Tick(float deltaTime, int32 Width, int32 Height)
{
    (void)Width;
    (void)Height;

    if (EditorViewport && EditorViewport->GetEditorViewportClient())
    {
        FEditorViewportClient *Client = EditorViewport->GetEditorViewportClient();
        Client->Tick(deltaTime);
    }
}

void FEditor::Exit()
{
    SUIManager::Get().UnregisterPanel(ControlPanel);
    SUIManager::Get().UnregisterPanel(PropertyPanel);
    SUIManager::Get().UnregisterPanel(StatPanel);
    SUIManager::Get().UnregisterPanel(OutputLog);

    delete ControlPanel;
    delete PropertyPanel;
    delete OutputLog;
    delete StatPanel;

    ControlPanel = nullptr;
    PropertyPanel = nullptr;
    StatPanel = nullptr;
    OutputLog = nullptr;

    if (EditorViewportClient != nullptr)
    {
        delete EditorViewportClient;
        EditorViewportClient = nullptr;
    }

    if (EditorViewport != nullptr)
    {
        delete EditorViewport;
        EditorViewport = nullptr;
    }
}

FEditorViewport *FEditor::GetEditorViewport() { return EditorViewport; }

FEditorViewportClient *FEditor::GetEditorViewportClient() { return EditorViewportClient; }