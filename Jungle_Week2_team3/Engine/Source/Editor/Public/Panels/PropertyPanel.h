#pragma once

#include "UIPanel.h"
#include <Math/Rotator.h>
#include <Math/Vector.h>

class FEditorViewportClient;
class AActor;

class SPropertyPanel : public SUIPanel
{
  public:
    SPropertyPanel(FString name);
    ~SPropertyPanel();

    void Render() override;
    void Update(float deltaTime) override;
    void SetCameraportClient(FEditorViewportClient *InEditorViewportClient);

  private:
    FVector  PrimitivePos;
    FRotator PrimitiveRot;
    FVector  PrimitiveScale;
    AActor  *PickedActor;

    FEditorViewportClient *EditorViewportClient = nullptr;
};
