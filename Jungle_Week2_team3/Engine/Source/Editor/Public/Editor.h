#pragma once
#include "HAL/Platform.h"

class SControlPanel;
class SPropertyPanel;
class SStatPanel;
class SOutputLog;
class FEditorViewport;
class FEditorViewportClient;

class FEditor
{
  public:
    void Init();
    void Tick(float DeltaTime, int32 Width, int32 Height);
    void Exit();

    FEditorViewport       *GetEditorViewport();
    FEditorViewportClient *GetEditorViewportClient() ;

  private:
    SControlPanel  *ControlPanel = nullptr;
    SPropertyPanel *PropertyPanel = nullptr;
    SStatPanel     *StatPanel = nullptr;
    SOutputLog     *OutputLog = nullptr;

    FEditorViewport       *EditorViewport = nullptr;
    FEditorViewportClient *EditorViewportClient = nullptr;
};