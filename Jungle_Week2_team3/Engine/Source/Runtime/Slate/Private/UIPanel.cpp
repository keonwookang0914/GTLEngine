#include "Engine/Source/Runtime/Slate/Public/UIPanel.h"
#include "ThirdParty/imgui/imgui.h"
#include <Logging/LogMacros.h>

SUIPanel::SUIPanel(FString name) : SUIMaster(std::move(name)) {}

SUIPanel::~SUIPanel() {}

void SUIPanel::Render()
{
    if (!bIsVisible)
        return;

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;

    if (PosX > 0.f || PosY > 0.f)
        ImGui::SetNextWindowPos(ImVec2(PosX, PosY), ImGuiCond_FirstUseEver);
    if (SizeX > 0.f && SizeY > 0.f)
        ImGui::SetNextWindowSize(ImVec2(SizeX, SizeY), ImGuiCond_FirstUseEver);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(Padding, Padding));

    if (ImGui::Begin(Name.c_str(), nullptr, flags))
        RenderChildren();

    ImGui::End();
    ImGui::PopStyleVar();
}

void SUIPanel::Update(float deltaTime)
{
    for (auto *child : Children)
    {
        if (child->IsVisible())
            child->Update(deltaTime);
    }
}

void SUIPanel::RenderChildren()
{
    for (auto *child : Children)
    {
        if (child->IsVisible())
            child->Render();
    }
}

void SUIPanel::AddChild(SUIMaster *child) { Children.push_back(child); }

void SUIPanel::RemoveChild(SUIMaster *child)
{
    auto it = std::find(Children.begin(), Children.end(), child);
    if (it != Children.end())
    {
        Children.erase(it);
    }
}

SUIMaster *SUIPanel::GetChild(const FString &name) const
{
    for (auto child : Children)
    {
        if (child->GetName() == name)
            return child;
    }
    return nullptr;
}

void SUIPanel::SetSize(float sizeX, float sizeY)
{
    SizeX = sizeX;
    SizeY = sizeY;
}

void SUIPanel::SetPadding(float padding) { Padding = padding; }

void SUIPanel::SetPosition(float InPosX, float InPosY)
{
    PosX = InPosX;
    PosY = InPosY;
}
