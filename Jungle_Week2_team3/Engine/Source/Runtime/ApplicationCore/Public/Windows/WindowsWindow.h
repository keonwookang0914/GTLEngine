#pragma once

#include "HAL/Platform.h"
#include <Windows.h>
#include <cstdint>

class FWindowsWindow
{
  public:
    FWindowsWindow() = default;
    ~FWindowsWindow() = default;

    bool Create(HINSTANCE InInstance, const wchar_t *InTitle, int32 InWidth, int32 InHeight);

    void Destroy();

    void Show();
    void Hide();

    HWND GetHWnd() const { return HWnd; }

    int32 GetWidth() const { return Width; }
    int32 GetHeight() const { return Height; }

    void SetSize(int32 InWidth, int32 InHeight)
    {
        Width = InWidth;
        Height = InHeight;
    }

  private:
    HWND  HWnd = nullptr;
    int32 Width = 0;
    int32 Height = 0;
};