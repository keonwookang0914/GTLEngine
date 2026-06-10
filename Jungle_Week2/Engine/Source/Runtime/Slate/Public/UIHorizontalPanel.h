#pragma once
#include "Runtime/Slate/Public/UIMaster.h"
#include <vector>

class SUIPanel; // Forward declaration

class SUIHorizontalPanel : public SUIMaster
{
  public:
    SUIHorizontalPanel(const FString name);
    virtual ~SUIHorizontalPanel() override;

    void Render() override;

    void AddChild(SUIMaster *child);
    void RemoveChild(SUIMaster *child);

  private:
    std::vector<SUIMaster *> Children;
    float                    Padding = 5.f; // 위젯 간 간격
};