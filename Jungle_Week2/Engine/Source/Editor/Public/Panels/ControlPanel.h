#pragma once
#include "HAL/Platform.h"
#include "Math/Rotator.h"
#include "Math/Vector.h"
#include "UIPanel.h"

class FEditorViewportClient;

class SControlPanel : public SUIPanel
{
  public:
    SControlPanel(FString name);
    ~SControlPanel();

    void Render() override;

    void Update(float deltaTime) override;

    void SetViewportClient(FEditorViewportClient *InEditorViewportClient);

  private:
    // Spawn Actor
    int32       CurrentItem = 0;
    const char *items[3] = {"Sphere", "Cube", "Plane"};
    int32       NumberOfSpawnActor = 0;
    float       range = 10.f;

    const float DragSpeed = 0.2f;
    const int32 Min = 0;
    const int32 Max = 50;

    FString SceneName = "Default";
    bool    bIsOrthogonal = false;

    // Base By
    const float FOVMin = 5.f;
    const float FOVMax = 170.f;

    FEditorViewportClient *EditorViewportClient = nullptr;

    // Camera
    float    FOV = 5.f;
    FVector  CameraPos;
    FRotator CameraRot;
};
