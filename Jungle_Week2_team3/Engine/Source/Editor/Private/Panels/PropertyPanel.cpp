#include "Panels/PropertyPanel.h"
#include "GameFramework/Actor.h"
#include "HAL/Platform.h"
#include "Logging/LogMacros.h"
#include "Math/MathUtility.h"
#include "Runtime/Core/CoreGlobals.h"
#include "Viewport/EditorViewportClient.h"
#include <EngineGlobals.h>
#include <imgui.h>
#include <imgui_stdlib.h>

SPropertyPanel::SPropertyPanel(FString name) : SUIPanel(std::move(name)) {}

SPropertyPanel::~SPropertyPanel() {}

void SPropertyPanel::Render()
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

    /**
     * TODO
     * Get SRT from Picked Actor and Set Actor's SRT.
     */
    // Test Code

    // Location
    PickedActor = EditorViewportClient->GetSelectedActor();
    if (PickedActor == nullptr)
    {
        ImGui::End();
        ImGui::PopStyleVar();
        return;
    }
    ImGui::Text("Picked Actor: %s", PickedActor->GetRootComponent()->GetClass()->GetName());
    ImGui::Text("UUID: %d", PickedActor->GetRootComponent()->UUID);

    PrimitivePos = PickedActor->GetActorLocation();
    ImGui::InputFloat3("Translation", &PrimitivePos.X, "%.3f");
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        PickedActor->SetActorLocation(PrimitivePos);
        UE_LOG(LogProperty, Log, "Position: (%f, %f, %f)", PrimitivePos.X, PrimitivePos.Y,
               PrimitivePos.Z);
    }
    PrimitiveRot = PickedActor->GetActorRotation();
    // Rotation
    ImGui::InputFloat3("Rotation", &PrimitiveRot.Pitch, "%.3f");
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        Clamp(&PrimitiveRot.Pitch, -89.f, 89.f);
        PickedActor->SetActorRotation(PrimitiveRot);
        UE_LOG(LogProperty, Log, "Rotation: (%f, %f, %f)", PrimitiveRot.Pitch, PrimitiveRot.Yaw,
               PrimitiveRot.Roll);
    }

    // Scale 3D
    PrimitiveScale = PickedActor->GetActorScale3D();
    ImGui::InputFloat3("Scale", &PrimitiveScale.X, "%.3f");
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        PickedActor->SetActorScale3D(PrimitiveScale);
        UE_LOG(LogProperty, Log, "Scale: (%f, %f, %f)", PrimitiveScale.X, PrimitiveScale.Y,
               PrimitiveScale.Z);
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void SPropertyPanel::Update(float deltaTime) {}

void SPropertyPanel::SetCameraportClient(FEditorViewportClient *InEditorViewportClient)
{
    EditorViewportClient = InEditorViewportClient;
}
