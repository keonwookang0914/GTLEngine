#pragma once
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#include "HAL/Platform.h"
#include <Windows.h>
#include <d3d11.h>

class FD3D11DynamicRHI
{
  public:
    void Initialize(HWND hWnd);
    void Shutdown();

    void BeginFrame();
    void EndFrame();

    void Resize(int32 InWidth, int32 InHeight);
    void SwapBuffer();

    ID3D11Device        *GetDevice() const { return Device; }
    ID3D11DeviceContext *GetDeviceContext() const { return DeviceContext; }
    IDXGISwapChain      *GetSwapChain() const { return SwapChain; }

    ID3D11RenderTargetView  *GetRenderTarGetCamera() const { return FrameBufferRTV; }
    ID3D11DepthStencilView  *GetDepthStencilView() const { return DepthStencilView; }
    ID3D11DepthStencilState *GetDepthStencilState() const { return DepthStencilState; }
    ID3D11RasterizerState   *GetRasterizerState() const { return RasterizerState; }

    const D3D11_VIEWPORT &GetViewport() const { return ViewportInfo; }

  private:
    void CreateDeviceAndSwapChain(HWND hWnd);
    void CreateFrameResources();
    void ReleaseFrameResources();

    void CreateFrameBuffer();
    void CreateDepthStencilBuffer();
    void CreateDepthStencilState();
    void CreateRasterizerState();

    void ReleaseDeviceAndSwapChain();
    void ReleaseFrameBuffer();
    void ReleaseDepthStencilBuffer();
    void ReleaseDepthStencilState();
    void ReleaseRasterizerState();

    void BindFrameState();
    void ClearFrameBuffers();
    void UpdateViewport(UINT Width, UINT Height);

    template <typename T> static void SafeRelease(T *&Resource)
    {
        if (Resource != nullptr)
        {
            Resource->Release();
            Resource = nullptr;
        }
    }

  private:
    ID3D11Device        *Device = nullptr;
    ID3D11DeviceContext *DeviceContext = nullptr;
    IDXGISwapChain      *SwapChain = nullptr;

    ID3D11Texture2D        *FrameBuffer = nullptr;
    ID3D11RenderTargetView *FrameBufferRTV = nullptr;

    ID3D11Texture2D         *DepthStencilBuffer = nullptr;
    ID3D11DepthStencilView  *DepthStencilView = nullptr;
    ID3D11DepthStencilState *DepthStencilState = nullptr;

    ID3D11RasterizerState *RasterizerState = nullptr;
    D3D11_VIEWPORT         ViewportInfo = {};

    FLOAT ClearColor[4] = {0.025f, 0.025f, 0.025f, 1.0f};
};