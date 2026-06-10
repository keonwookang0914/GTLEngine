#include "Panels/OutputLog.h"
#include "ThirdParty/imgui/imgui.h"

SOutputLog::SOutputLog(FString name) : SUIPanel(std::move(name)) {}

void SOutputLog::Log(ELogVerbosity Verbosity, const char *Message)
{
    // TODO: Change Deque
    if (LogEntries.size() >= 100)
    {
        LogEntries.pop_front();
    }

    LogEntries.push_back({Verbosity, FString(Message)});
    bScrollToBottom = true;
}

void SOutputLog::Clear() { LogEntries.clear(); }

void SOutputLog::Render()
{
    if (!bIsVisible)
        return;

    if (SizeX > 0.f && SizeY > 0.f)
        ImGui::SetNextWindowSize(ImVec2(SizeX, SizeY), ImGuiCond_FirstUseEver);
    if (PosX > 0.f || PosY > 0.f)
        ImGui::SetNextWindowPos(ImVec2(PosX, PosY), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin(Name.c_str()))
    {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Clear"))
        Clear();

    ImGui::Separator();

    ImGui::BeginChild("ScrollRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    static const ImVec4 Colors[] = {
        {1.f, 1.f, 1.f, 1.f},   // Log     — 흰색
        {1.f, 1.f, 0.f, 1.f},   // Warning — 노란색
        {1.f, 0.3f, 0.3f, 1.f}, // Error   — 빨간색
    };

    for (const FLogEntry &Entry : LogEntries)
    {
        int32 idx = static_cast<int32>(Entry.Verbosity);
        ImGui::TextColored(Colors[idx], "%s", Entry.Message.c_str());
    }

    if (bScrollToBottom)
    {
        ImGui::SetScrollHereY(1.f);
        bScrollToBottom = false;
    }

    ImGui::EndChild();
    ImGui::End();
}
