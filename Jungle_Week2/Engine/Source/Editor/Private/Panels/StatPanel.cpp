#include "Panels/StatPanel.h"
#include "HAL/MemoryBase.h"
#include "Runtime/Core/CoreGlobals.h"
#include <imgui.h>

SStatPanel::SStatPanel(FString name) : SUIPanel(std::move(name)) {}

SStatPanel::~SStatPanel() {}

void SStatPanel::Render()
{
    if (!bIsVisible)
        return;

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;

    if (PosX > 0.f || PosY > 0.f)
        ImGui::SetNextWindowPos(ImVec2(PosX, PosY), ImGuiCond_FirstUseEver);
    if (SizeX > 0.f && SizeY > 0.f)
        ImGui::SetNextWindowSize(ImVec2(SizeX, SizeY), ImGuiCond_FirstUseEver);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(Padding, Padding));

    if (!ImGui::Begin(Name.c_str(), nullptr, flags))
    {
        ImGui::End();
        ImGui::PopStyleVar();
        return;
    }

    ImGui::Text("CurrentAllocationCount: %d", GMalloc->MallocStats.CurrentAllocationCount);
    ImGui::Text("CurrentAllocationBytes: %d", GMalloc->MallocStats.CurrentAllocationBytes);

    ImGui::Text("TotalAllocationCount: %d", GMalloc->MallocStats.TotalAllocationCount);
    ImGui::Text("TotalAllocationBytes: %d", GMalloc->MallocStats.TotalAllocationBytes);

    ImGui::End();
    ImGui::PopStyleVar();
}

void SStatPanel::Update(float deltaTime) {}
