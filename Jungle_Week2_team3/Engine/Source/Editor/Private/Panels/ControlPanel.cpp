#include "Panels/ControlPanel.h"
#include "Components/CubeComp.h"
#include "Components/PlaneComp.h"
#include "SceneFileIO.h"
#include "Viewport/EditorViewportClient.h"
#include "EngineGlobals.h"
#include "EngineStatics.h"
#include "GameFramework/Actor.h"
#include "HAL/Platform.h"
#include "Logging/LogMacros.h"
#include "Math/Rotator.h"
#include "Runtime/Core/CoreGlobals.h"
#include "SceneJson.h"
#include <Components/SphereComp.h>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <random>
#include "UObject/UObjectGlobals.h"

SControlPanel::SControlPanel(FString name) : SUIPanel(std::move(name)) {}

SControlPanel::~SControlPanel() {}

void SControlPanel::Render()
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
        return;
    }

    ImGui::Text("Hello Jungle World!");

    ImGuiIO io = ImGui::GetIO();

    ImGui::Text("FPS %.0f (%.0f ms) ", io.Framerate, 1000.f / io.Framerate);

    ImGui::Separator();
    ImGui::Combo("Primitive", &CurrentItem, items, IM_COUNTOF(items));

    bool bSpawnActor = ImGui::Button("Spawn");

    ImGui::SameLine();
    ImGui::DragScalar("Number of spawn", ImGuiDataType_S32, &NumberOfSpawnActor, DragSpeed, &Min,
                      &Max);

    // TODO: after fix Cube, Plane, SphereComp
    if (bSpawnActor)
    {

        std::random_device                    random;
        std::mt19937                          Gen(random());
        std::uniform_real_distribution<float> Dist(-range, range);

        FVector          Pos;
        for (int32 i = 0; i < NumberOfSpawnActor; ++i)
        {
            Pos = FVector(Dist(Gen), Dist(Gen), Dist(Gen)) * 20;
            AActor *tmp = GEngine->GetWorld().SpawnActor(AActor::StaticClass(), &Pos);
            
            switch (CurrentItem)
            {
            case 0: // Sphere
                tmp->SetRootComponent(NewObject<USphereComp>(USphereComp::StaticClass()));
                break;
            case 1: // Cube
                tmp->SetRootComponent(NewObject<UCubeComp>(UCubeComp::StaticClass()));
                break;
            case 2: // Plane
                tmp->SetRootComponent(NewObject<UPlaneComp>(UPlaneComp::StaticClass()));
                break;
            }

            if (tmp)
            {
                tmp->SetActorLocation(Pos);
                UE_LOG(LogSpawn, Log, "%s Spawn Success! [UUID: %d], [addr: %p], ",
                       tmp->GetRootComponent()->GetClass()->GetName(),
                       tmp->GetRootComponent()->UUID, tmp);
            }
            else
            {
                UE_LOG(LogSpawn, Log, "Spawn Failed");
            }
        }
    }

    /**
     * Set Scene name.
     *
     */
    if (ImGui::InputText("Scene Name", &SceneName,
                         ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsNoBlank))
    {
        UE_LOG(LogControlPanel, Log, "SceneName : %s", SceneName.c_str());
    }

    /**
     * TODO: Scene IO
     *
     */
    if (ImGui::Button("New Scene"))
    {
        if (EditorViewportClient)
        {
            EditorViewportClient->SetSelectedActor(nullptr);
        }

        GEngine->GetWorld().ClearAll();
        UEngineStatics::SetNextUUID(0);
    }
    if (ImGui::Button("Save Scene"))
    {
        FString FilePath = SceneName + ".Scene";
        if (FSceneFileIO::SaveSceneToFile(FilePath, GEngine->GetWorld()))
        {
            UE_LOG(LogFile, Log, "Save File Success Path: %s", FilePath.c_str());
        }
        else
        {
            UE_LOG(LogFile, Error, "Save File Failed.");
        }
    }
    if (ImGui::Button("Load Scene"))
    {
        if (EditorViewportClient)
        {
            EditorViewportClient->SetSelectedActor(nullptr);
        }

        FString FilePath = SceneName + ".Scene";
        if (FSceneFileIO::LoadSceneFromFile(FilePath, GEngine->GetWorld()))
        {
            UE_LOG(LogFile, Log, "Load File Success Path: %s", FilePath.c_str());
        }
        else
        {
            UE_LOG(LogFile, Error, "Load File Failed.");
        }
    }
    ImGui::Separator();

    if (EditorViewportClient)
    {
        /**
         * Projection Mode.
         */
        if (ImGui::Checkbox("Orthographic", &bIsOrthogonal))
        {
            if (bIsOrthogonal)
            {
                EditorViewportClient->SetProjectionMode(EEditorProjectionMode::Orthographic);
            }
            else
            {
                EditorViewportClient->SetProjectionMode(EEditorProjectionMode::Perspective);
            }
        }

        /**
         * Camera FOV.
         */
        FOV = EditorViewportClient->GetCameraFov();
        if (ImGui::DragScalar("FOV", ImGuiDataType_Float, &FOV, DragSpeed, &FOVMin, &FOVMax))
        {
            EditorViewportClient->SetCameraFov(FOV);
        }

        /**
         * Camera Location.
         */
        CameraPos = EditorViewportClient->GetCameraLocation();
        ImGui::InputFloat3("Camera Location", &CameraPos.X, "%.3f");
        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            EditorViewportClient->SetCameraLocation(CameraPos);

            UE_LOG(LogProperty, Log, "Camera Position: (%.3f, %.3f, %.3f)", CameraPos.X,
                   CameraPos.Y, CameraPos.Z);
        }
        /**
         * Camera Rotation.
         */
        CameraRot = EditorViewportClient->GetCameraRotation();
        ImGui::InputFloat3("Camera Rotation", &CameraRot.Pitch, "%.3f");
        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            EditorViewportClient->SetCameraRotation(CameraRot);

            UE_LOG(LogProperty, Log, "Camera Position: (%.3f, %.3f, %f)", CameraRot.Pitch,
                   CameraRot.Yaw, CameraRot.Roll);
        }
    }
    FGizmoState State = EditorViewportClient->GetGizmoState();
    switch (State.Mode)
    {
    case EGizmoMode::Translate:
        ImGui::Text("Gizmo MOde: Translate");
        break;
    case EGizmoMode::Rotate:
        ImGui::Text("Gizmo MOde: Rotate");
        break;
    case EGizmoMode::Scale:
        ImGui::Text("Gizmo MOde: Scale");
        break;
    }
    

    ImGui::End();
    ImGui::PopStyleVar();
}

void SControlPanel::Update(float deltaTime) { (void)deltaTime; }

void SControlPanel::SetViewportClient(FEditorViewportClient *InEditorViewportClient)
{
    EditorViewportClient = InEditorViewportClient;
}
