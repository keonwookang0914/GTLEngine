#include "Engine/Source/Runtime/SlateCore/Public/UIManager.h"
#include "UIPanel.h"
#include "Engine/Source/ThirdParty/imgui/imgui.h"
#include "Engine/Source/ThirdParty/imgui/imgui_impl_dx11.h"
#include "Engine/Source/ThirdParty/imgui/imgui_impl_win32.h"
#include "Engine/Source/ThirdParty/imgui/imgui_internal.h"

SUIManager &SUIManager::Get()
{
    static SUIManager Manager;
    return Manager;
}

void SUIManager::Initialize(HWND hWnd, ID3D11Device *device, ID3D11DeviceContext *context)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplWin32_Init((void *)hWnd);
    ImGui_ImplDX11_Init(device, context);
    bIsInitialized = true;
}

void SUIManager::Shutdown()
{
    for (auto panel : Panels)
    {
        if (panel)
        {
            delete panel;
        }
    }
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    bIsInitialized = false;
}

void SUIManager::BeginFrame()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void SUIManager::RenderAll()
{
    for (auto Panel : Panels)
    {
        if (Panel)
        {
            Panel->Render();
        }
    }
}

void SUIManager::EndFrame()
{
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void SUIManager::RegisterPanel(SUIPanel *panel) { Panels.push_back(panel); }

void SUIManager::UnregisterPanel(SUIPanel *panel)
{
    auto it = std::find(Panels.begin(), Panels.end(), panel);
    if (it != Panels.end())
    {
        Panels.erase(it);
    }
}
