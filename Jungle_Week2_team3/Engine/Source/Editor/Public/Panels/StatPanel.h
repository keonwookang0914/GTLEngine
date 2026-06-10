#pragma once
#include "UIPanel.h"
#include "HAL/Platform.h"
class SStatPanel : public SUIPanel
{
  public:
    SStatPanel(FString name);
    ~SStatPanel();

    void Render() override;
    void Update(float deltaTime);

    uint32 PrevCount = 0;
    uint32 Countdiff = 0;
    uint32 PrevBytes = 0;
    uint32 BytesDiff = 0;
};
