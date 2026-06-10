#pragma once

#include "GenericPlatform/GenericApplication.h"
#include "GenericPlatform/GenericApplicationMessageHandler.h"
#include "HAL/Platform.h"
#include "Windows/WindowsWindow.h"
#include <Windows.h>
#include <cstdint>

class FWindowsApplication : public FGenericApplication
{
  public:
    FWindowsApplication();
    virtual ~FWindowsApplication() override;

    static FWindowsApplication *Create();

  public:
    virtual void SetMessageHandler(FGenericApplicationMessageHandler *InMessageHandler) override;
    virtual FGenericApplicationMessageHandler *GetMessageHandler() const override;

    virtual bool CreateApplicationWindow(const wchar_t *InTitle, int32 InWidth,
                                         int32 InHeight) override;
    virtual void DestroyApplicationWindow() override;

    virtual int32 GetWindowWidth() const override { return Window.GetWidth(); }
    virtual int32 GetWindowHeight() const override { return Window.GetHeight(); }

    virtual void PumpMessages() override;

    virtual void ShowWindow() override;
    virtual void HideWindow() override;

    virtual void *GetNativeWindowHandle() const override;

    virtual bool ProcessKeyDownEvent(EKey Key, bool bIsRepeat) override;
    virtual bool ProcessKeyUpEvent(EKey Key) override;

    virtual bool ProcessMouseDownEvent(EKey Button, int32 X, int32 Y) override;
    virtual bool ProcessMouseUpEvent(EKey Button, int32 X, int32 Y) override;
    virtual bool ProcessMouseDoubleClickEvent(EKey Button, int32 X, int32 Y) override;

    virtual bool ProcessMouseMoveEvent(int32 X, int32 Y) override;
    virtual bool ProcessRawMouseMoveEvent(int32 DeltaX, int32 DeltaY) override;
    virtual bool ProcessMouseWheelEvent(float Delta, int32 X, int32 Y) override;

    LRESULT ProcessMessage(HWND InHWnd, UINT InMessage, WPARAM InWParam, LPARAM InLParam);

    FWindowsWindow       &GetWindow() { return Window; }
    const FWindowsWindow &GetWindow() const { return Window; }

  private:
    EKey TranslateKey(WPARAM InWParam) const;
    EKey TranslateMouseButton(UINT InMessage) const;
    void RegisterRawMouseInput();

  private:
    FGenericApplicationMessageHandler *MessageHandler = nullptr;
    FWindowsWindow                     Window;
    bool                               bRawMouseInputRegistered = false;
    bool                               bHasLastMousePosition = false;
    int32                              LastMouseX = 0;
    int32                              LastMouseY = 0;

    BYTE *RawData = nullptr;
    uint32 MaxRawDataNum = 128;
};