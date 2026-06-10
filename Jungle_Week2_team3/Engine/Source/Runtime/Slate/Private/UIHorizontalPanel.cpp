#include "Runtime/Slate/Public/UIHorizontalPanel.h"
#include "ThirdParty/imgui/imgui.h"

SUIHorizontalPanel::SUIHorizontalPanel(const FString name) : SUIMaster(std::move(name)) {}

SUIHorizontalPanel::~SUIHorizontalPanel() {}

void SUIHorizontalPanel::AddChild(SUIMaster *child) { Children.push_back(child); }

void SUIHorizontalPanel::RemoveChild(SUIMaster *child)
{
    auto it = std::find(Children.begin(), Children.end(), child);
    if (it != Children.end())
        Children.erase(it);
}

void SUIHorizontalPanel::Render()
{
    if (!bIsVisible)
        return;

    for (size_t i = 0; i < Children.size(); ++i)
    {
        SUIMaster *child = Children[i];
        if (!child || !child->IsVisible())
            continue;

        child->Render();

        if (i < Children.size() - 1)
            ImGui::SameLine(); // 수평 배치
    }
    ImGui::NewLine(); // 마지막 줄 끝
}