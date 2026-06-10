#include "D3D11/D3D11DynamicRHI.h"
#include "UIManager.h"
#include <cassert>
#include <d3dcompiler.h>

void FD3D11DynamicRHI::Initialize(HWND hWnd)
{
    CreateDeviceAndSwapChain(hWnd);
    CreateFrameResources();
    SUIManager::Get().Initialize(hWnd, Device, DeviceContext);
}

void FD3D11DynamicRHI::Shutdown()
{
    SUIManager::Get().Shutdown();

    // 파이프라인에 바인딩된 모든 리소스 해제 → Release() 시 refcount가 정상적으로 0에 도달
    if (DeviceContext)
    {
        DeviceContext->ClearState();
        DeviceContext->Flush();
    }

    ReleaseFrameResources();
    ReleaseDeviceAndSwapChain();
}

void FD3D11DynamicRHI::BeginFrame()
{
    BindFrameState();
    ClearFrameBuffers();
}

void FD3D11DynamicRHI::EndFrame()
{
    if (SwapChain != nullptr)
    {
        // SwapChain->Present(1, 0);  // VSynch On
        SwapChain->Present(0, 0);  // VSynch Off => 즉시 표시
    }
}

void FD3D11DynamicRHI::Resize(int32 InWidth, int32 InHeight)
{
    if (SwapChain == nullptr || DeviceContext == nullptr)
    {
        return;
    }

    if (InWidth <= 0 || InHeight <= 0)
    {
        return;
    }

    DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

    ReleaseFrameResources();

    HRESULT hr = SwapChain->ResizeBuffers(0, static_cast<UINT>(InWidth),
                                          static_cast<UINT>(InHeight), DXGI_FORMAT_UNKNOWN, 0);
    assert(SUCCEEDED(hr));

    CreateFrameResources();
}

void FD3D11DynamicRHI::SwapBuffer()
{
    SwapChain->Present(0, 0); // 1: VSync
}

void FD3D11DynamicRHI::CreateDeviceAndSwapChain(HWND hWnd)
{
    RECT ClientRect = {};
    GetClientRect(hWnd, &ClientRect);

    const UINT Width = static_cast<UINT>(ClientRect.right - ClientRect.left);
    const UINT Height = static_cast<UINT>(ClientRect.bottom - ClientRect.top);

    DXGI_SWAP_CHAIN_DESC SwapChainDesc = {};
    SwapChainDesc.BufferDesc.Width = (Width > 0) ? Width : 1;
    SwapChainDesc.BufferDesc.Height = (Height > 0) ? Height : 1;
    SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    SwapChainDesc.SampleDesc.Count = 1;
    SwapChainDesc.SampleDesc.Quality = 0;

    SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc.BufferCount = 2;
    SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    SwapChainDesc.OutputWindow = hWnd;
    SwapChainDesc.Windowed = TRUE;
    SwapChainDesc.Flags = D3D11_CREATE_DEVICE_DEBUG;

    UINT CreateDeviceFlags = 0;
#if defined(_DEBUG)
    CreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL FeatureLevels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1,
                                         D3D_FEATURE_LEVEL_10_0};

    D3D_FEATURE_LEVEL CreatedFeatureLevel = D3D_FEATURE_LEVEL_11_0;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, CreateDeviceFlags, FeatureLevels,
        ARRAYSIZE(FeatureLevels), D3D11_SDK_VERSION, &SwapChainDesc, &SwapChain, &Device,
        &CreatedFeatureLevel, &DeviceContext);
    assert(SUCCEEDED(hr));

    UpdateViewport(SwapChainDesc.BufferDesc.Width, SwapChainDesc.BufferDesc.Height);
}

void FD3D11DynamicRHI::CreateFrameResources()
{
    CreateFrameBuffer();
    CreateDepthStencilBuffer();
    CreateDepthStencilState();
    CreateRasterizerState();

    RECT ClientRect = {};
    if (SwapChain != nullptr)
    {
        DXGI_SWAP_CHAIN_DESC Desc = {};
        HRESULT              hr = SwapChain->GetDesc(&Desc);
        assert(SUCCEEDED(hr));
        UpdateViewport(Desc.BufferDesc.Width, Desc.BufferDesc.Height);
    }
}

void FD3D11DynamicRHI::ReleaseFrameResources()
{
    ReleaseRasterizerState();
    ReleaseDepthStencilState();
    ReleaseDepthStencilBuffer();
    ReleaseFrameBuffer();
}

void FD3D11DynamicRHI::CreateFrameBuffer()
{
    assert(SwapChain != nullptr);
    assert(Device != nullptr);

    HRESULT hr =
        SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&FrameBuffer));
    assert(SUCCEEDED(hr));

    D3D11_RENDER_TARGET_VIEW_DESC RTVDesc = {};
    RTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

    hr = Device->CreateRenderTargetView(FrameBuffer, &RTVDesc, &FrameBufferRTV);
    assert(SUCCEEDED(hr));
}

void FD3D11DynamicRHI::CreateDepthStencilBuffer()
{
    assert(Device != nullptr);
    assert(SwapChain != nullptr);

    DXGI_SWAP_CHAIN_DESC SwapChainDesc = {};
    HRESULT              hr = SwapChain->GetDesc(&SwapChainDesc);
    assert(SUCCEEDED(hr));

    D3D11_TEXTURE2D_DESC DepthDesc = {};
    DepthDesc.Width = SwapChainDesc.BufferDesc.Width;
    DepthDesc.Height = SwapChainDesc.BufferDesc.Height;
    DepthDesc.MipLevels = 1;
    DepthDesc.ArraySize = 1;
    DepthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    DepthDesc.SampleDesc.Count = 1;
    DepthDesc.SampleDesc.Quality = 0;
    DepthDesc.Usage = D3D11_USAGE_DEFAULT;
    DepthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    DepthDesc.CPUAccessFlags = 0;
    DepthDesc.MiscFlags = 0;

    hr = Device->CreateTexture2D(&DepthDesc, nullptr, &DepthStencilBuffer);
    assert(SUCCEEDED(hr));

    hr = Device->CreateDepthStencilView(DepthStencilBuffer, nullptr, &DepthStencilView);
    assert(SUCCEEDED(hr));
}

void FD3D11DynamicRHI::CreateDepthStencilState()
{
    assert(Device != nullptr);

    D3D11_DEPTH_STENCIL_DESC Desc = {};
    Desc.DepthEnable = TRUE;
    Desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    Desc.DepthFunc = D3D11_COMPARISON_LESS;

    Desc.StencilEnable = FALSE;
    Desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    Desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

    HRESULT hr = Device->CreateDepthStencilState(&Desc, &DepthStencilState);
    assert(SUCCEEDED(hr));
}

void FD3D11DynamicRHI::CreateRasterizerState()
{
    assert(Device != nullptr);

    D3D11_RASTERIZER_DESC Desc = {};
    Desc.FillMode = D3D11_FILL_SOLID;
    Desc.CullMode = D3D11_CULL_BACK;
    Desc.FrontCounterClockwise = FALSE;
    Desc.DepthBias = 0;
    Desc.DepthBiasClamp = 0.0f;
    Desc.SlopeScaledDepthBias = 0.0f;
    Desc.DepthClipEnable = TRUE;
    Desc.ScissorEnable = FALSE;
    Desc.MultisampleEnable = FALSE;
    Desc.AntialiasedLineEnable = FALSE;

    HRESULT hr = Device->CreateRasterizerState(&Desc, &RasterizerState);
    assert(SUCCEEDED(hr));
}

void FD3D11DynamicRHI::ReleaseDeviceAndSwapChain()
{
    SafeRelease(SwapChain);
    SafeRelease(DeviceContext);
    SafeRelease(Device);
}

void FD3D11DynamicRHI::ReleaseFrameBuffer()
{
    SafeRelease(FrameBufferRTV);
    SafeRelease(FrameBuffer);
}

void FD3D11DynamicRHI::ReleaseDepthStencilBuffer()
{
    SafeRelease(DepthStencilView);
    SafeRelease(DepthStencilBuffer);
}

void FD3D11DynamicRHI::ReleaseDepthStencilState() { SafeRelease(DepthStencilState); }

void FD3D11DynamicRHI::ReleaseRasterizerState() { SafeRelease(RasterizerState); }

void FD3D11DynamicRHI::BindFrameState()
{
    assert(DeviceContext != nullptr);

    DeviceContext->OMSetRenderTargets(1, &FrameBufferRTV, DepthStencilView);
    DeviceContext->OMSetDepthStencilState(DepthStencilState, 0);
    DeviceContext->RSSetState(RasterizerState);
    DeviceContext->RSSetViewports(1, &ViewportInfo);
}

void FD3D11DynamicRHI::ClearFrameBuffers()
{
    assert(DeviceContext != nullptr);

    if (FrameBufferRTV != nullptr)
    {
        DeviceContext->ClearRenderTargetView(FrameBufferRTV, ClearColor);
    }

    if (DepthStencilView != nullptr)
    {
        DeviceContext->ClearDepthStencilView(DepthStencilView,
                                             D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    }
}

void FD3D11DynamicRHI::UpdateViewport(UINT Width, UINT Height)
{
    ViewportInfo.TopLeftX = 0.0f;
    ViewportInfo.TopLeftY = 0.0f;
    ViewportInfo.Width = static_cast<FLOAT>((Width > 0) ? Width : 1);
    ViewportInfo.Height = static_cast<FLOAT>((Height > 0) ? Height : 1);
    ViewportInfo.MinDepth = 0.0f;
    ViewportInfo.MaxDepth = 1.0f;
}