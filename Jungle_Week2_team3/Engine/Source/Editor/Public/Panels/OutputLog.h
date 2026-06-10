#pragma once

#include "Containers/Deque.h"
#include "HAL/Platform.h"
#include "Logging/LogOutputDevice.h"
#include "Misc/UnrealString.h"
#include "UIPanel.h"


class SUIButton;

struct FLogEntry
{
    ELogVerbosity Verbosity;
    FString       Message;
};

class SOutputLog : public SUIPanel, public FLogOutputDevice
{
  public:
    explicit SOutputLog(FString name);
    virtual ~SOutputLog() override = default;

    // FLogOutputDevice
    void Log(ELogVerbosity Verbosity, const char *Message) override;

    // SUIPanel
    void Render() override;

    void Clear();

  private:
    TDeque<FLogEntry> LogEntries;
    bool              bScrollToBottom = false;
    int32             size;
};
