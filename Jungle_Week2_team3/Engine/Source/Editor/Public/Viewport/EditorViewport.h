#pragma once
#include "GenericPlatform/GenericApplicationMessageHandler.h"
#include "HAL/Platform.h"
#include "Viewport/EditorViewportClient.h"

class FEditorViewportClient;

/**
 * Editor viewport message handler responsible for receiving window input,
 * mouse movement, focus, and resize events, then passing them to the
 * editor viewport client.
 */
class FEditorViewport : public FGenericApplicationMessageHandler
{
  public:
    virtual bool OnKeyDown(EKey Key, bool bIsRepeat) override;
    virtual bool OnKeyUp(EKey Key) override;

    virtual bool OnMouseDown(EKey Button, int32 X, int32 Y) override;
    virtual bool OnMouseUp(EKey Button, int32 X, int32 Y) override;
    virtual bool OnMouseDoubleClick(EKey Button, int32 X, int32 Y) override;

    virtual bool OnMouseMove(int32 X, int32 Y) override;
    virtual bool OnRawMouseMove(int32 DeltaX, int32 DeltaY) override;
    virtual bool OnMouseWheel(float Delta, int32 X, int32 Y) override;

    virtual bool OnSizeChanged(int32 Width, int32 Height) override;

    void                   SetEditorViewportClient(FEditorViewportClient *InViewportClient);
    FEditorViewportClient *GetEditorViewportClient();

    virtual void OnFocusLost() override;

  private:
    FEditorViewportClient *ViewportClient = nullptr;
};