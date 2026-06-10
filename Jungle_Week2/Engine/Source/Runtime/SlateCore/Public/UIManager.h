#pragma once
#include <wtypes.h>
#include "Engine/Source/Runtime/Core/Public/Containers/Array.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
class SUIPanel;

class SUIManager
{
  public:
    static SUIManager &Get();

    void Initialize(HWND hWnd, ID3D11Device *device, ID3D11DeviceContext *context);

    void Shutdown();

    void BeginFrame();
    void RenderAll();
    void EndFrame();

    void RegisterPanel(SUIPanel *panel);
    void UnregisterPanel(SUIPanel *panel);

  private:
    SUIManager() = default;

    TArray<SUIPanel *> Panels; // Panel List
    bool              bIsInitialized = false;
};
