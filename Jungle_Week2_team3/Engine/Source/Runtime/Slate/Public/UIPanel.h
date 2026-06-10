#pragma once
#include "Containers/Array.h"
#include "Misc/UnrealString.h"
#include "UIMaster.h"

class SUIPanel : public SUIMaster
{
  public:
    explicit SUIPanel(FString name);

    virtual ~SUIPanel() override;

    void Render() override;
    void Update(float deltaTime) override;

    void       RenderChildren();
    void       AddChild(SUIMaster *child);
    void       RemoveChild(SUIMaster *child); //
    SUIMaster *GetChild(const FString &name) const;

    void SetSize(float sizeX, float sizeY); // {0,0} ImGui Auto fit
    void SetPadding(float padding);
    void SetPosition(float PosX, float PosY);

  protected:
    TArray<SUIMaster *> Children;
    float               SizeX = 0.f;
    float               SizeY = 0.f;
    float               PosX = 0.f;
    float               PosY = 0.f;
    float               Padding = 8.f;
};
