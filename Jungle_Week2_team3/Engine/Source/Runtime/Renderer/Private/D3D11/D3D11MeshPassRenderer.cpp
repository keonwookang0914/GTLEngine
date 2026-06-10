#include "D3D11/D3D11MeshPassRenderer.h"
#include "D3D11/D3D11DynamicRHI.h"
#include "Engine/Extras/Cube.h"
#include "Engine/Extras/Sphere.h"
#include "EngineGlobals.h"
#include "StaticMeshResources.h"
#include <d3dcompiler.h>
#include "UObject/UObjectGlobals.h"

namespace
{
    constexpr const wchar_t *ShaderFilePath = L"Engine/Shaders/Shader.hlsl";
    constexpr const char    *VertexShaderEntry = "mainVS";
    constexpr const char    *PixelShaderEntry = "mainPS";
    constexpr const char    *VertexShaderTarget = "vs_5_0";
    constexpr const char    *PixelShaderTarget = "ps_5_0";
} // namespace

void FD3D11MeshPassRenderer::Initialize(FD3D11DynamicRHI *InRHI)
{
    RHI = InRHI;
    Stride = sizeof(FStaticMeshBuildVertex);
    
    GDefaultSphere = NewObject<UStaticMesh>(UStaticMesh::StaticClass());
    GDefaultSphere->RenderData = new FStaticMeshRenderData;
    const size_t SphereVertexCount = sizeof(sphere_vertices) / sizeof(FStaticMeshBuildVertex);
    GDefaultSphere->RenderData->Vertices.assign(sphere_vertices,
                                                sphere_vertices + SphereVertexCount);
    GDefaultSphere->RenderData->VertexBuffer =
        CreateVertexBuffer(GDefaultSphere->RenderData->Vertices.data(),
                           static_cast<UINT>(GDefaultSphere->RenderData->Vertices.size() *
                                             sizeof(FStaticMeshBuildVertex)));

    GDefaultCube = NewObject<UStaticMesh>(UStaticMesh::StaticClass());
    GDefaultCube->RenderData = new FStaticMeshRenderData;

    const size_t CubeVertexCount = sizeof(cube_vertices) / sizeof(FStaticMeshBuildVertex);
    GDefaultCube->RenderData->Vertices.assign(cube_vertices, cube_vertices + CubeVertexCount);

    GDefaultCube->RenderData->VertexBuffer =
        CreateVertexBuffer(GDefaultCube->RenderData->Vertices.data(),
                           static_cast<UINT>(GDefaultCube->RenderData->Vertices.size() *
                                             sizeof(FStaticMeshBuildVertex)));

    GDefaultPlane = NewObject<UStaticMesh>(UStaticMesh::StaticClass());
    GDefaultPlane->RenderData = new FStaticMeshRenderData;

    FStaticMeshBuildVertex PlaneVertices[3] = {{1.0f, -1.0f, -0.5f, 1, 0, 0, 1},
                                               {0.0f, 1.0f, -0.5f, 0, 1, 0, 1},
                                               {-1.0f, -1.0f, -0.5f, 0, 0, 1, 1}};

    GDefaultPlane->RenderData->Vertices.assign(std::begin(PlaneVertices), std::end(PlaneVertices));

    GDefaultPlane->RenderData->VertexBuffer =
        CreateVertexBuffer(GDefaultPlane->RenderData->Vertices.data(),
                           static_cast<UINT>(GDefaultPlane->RenderData->Vertices.size() *
                                             sizeof(FStaticMeshBuildVertex)));

    CreateShader();
    CreateConstantBuffer();
    CreateInstanceBuffer(CurrentMaxInstanceNum);
}

void FD3D11MeshPassRenderer::Shutdown()
{
    InstanceMap.clear();

    ReleaseInstanceBuffer();
    ReleaseConstantBuffer();
    ReleaseShader();

    RHI = nullptr;
    Stride = 0;
}

void FD3D11MeshPassRenderer::Draw()
{
    if (InstanceMap.empty())
        return;

    for (auto &Pair : InstanceMap)
    {
        auto &VertexBuffer = Pair.first;
        auto &InstanceDataArray = Pair.second;

        RenderInstance(VertexBuffer, InstanceDataArray.VertexNum);
        InstanceDataArray.NextInstanceNum = 0;
    }

    // InstanceMap.clear();
}

void FD3D11MeshPassRenderer::PrepareShader()
{
    ID3D11DeviceContext *DeviceContext = GetDeviceContext();
    if (DeviceContext == nullptr)
    {
        return;
    }

    DeviceContext->IASetInputLayout(SimpleInputLayout);
    DeviceContext->VSSetShader(SimpleVertexShader, nullptr, 0);
    DeviceContext->PSSetShader(SimplePixelShader, nullptr, 0);

    if (ConstantBuffer != nullptr)
    {
        DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
        DeviceContext->PSSetConstantBuffers(0, 1, &ConstantBuffer);
    }
}

void FD3D11MeshPassRenderer::RenderPrimitive(ID3D11Buffer *VertexBuffer, UINT NumVertices)
{
    if (VertexBuffer == nullptr || NumVertices == 0)
    {
        return;
    }

    ID3D11DeviceContext *DeviceContext = GetDeviceContext();
    if (DeviceContext == nullptr)
    {
        return;
    }

    UINT Offset = 0;

    DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &Stride, &Offset);
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    DeviceContext->Draw(NumVertices, 0);
}

void FD3D11MeshPassRenderer::RegisterInstance(ID3D11Buffer *VertexBuffer, UINT NumVertices,
                                              const FMatrix &InModelMatrix)
{
    // TODO: InstanceMap 개선
    /** 이미 등록된 Primitive List 의 경우 중도에 삭제되는 등의 작업이 없음 가정 */
    /** 즉, Data.Matrices 의 Index 가 유지됨 */
    /** Scene 이 Clear 되는 경우에는 InstanceMap 도 Clear 하므로 문제 없음 */

    if (InstanceMap.count(VertexBuffer) > 0)
    {
        FInstanceDataArray &Data = InstanceMap[VertexBuffer];

        if (Data.NextInstanceNum >= Data.Matrices.size())
        {
            Data.Matrices.push_back(InModelMatrix);
        }
        else
        {
            Data.Matrices[Data.NextInstanceNum] = InModelMatrix;
        }

        Data.NextInstanceNum++;
    }
    else
    {
        // 최초 등록
        FInstanceDataArray &Data = InstanceMap[VertexBuffer];

        Data.VertexNum = NumVertices;
        Data.NextInstanceNum = 1;
        Data.Matrices.push_back(InModelMatrix);
    }
}

void FD3D11MeshPassRenderer::RenderInstance(ID3D11Buffer *VertexBuffer, UINT NumVertices)
{
    HRESULT Hr;

    if (VertexBuffer == nullptr || InstanceBuffer == nullptr || NumVertices == 0)
    {
        return;
    }

    ID3D11DeviceContext *DeviceContext = GetDeviceContext();
    if (DeviceContext == nullptr)
    {
        return;
    }

    const FInstanceDataArray &InstanceDataArray = InstanceMap[VertexBuffer];
    D3D11_MAPPED_SUBRESOURCE  Mapped;

    if (InstanceDataArray.Matrices.size() > CurrentMaxInstanceNum)
        CreateInstanceBuffer(RoundUpToPowerOfTwo(InstanceDataArray.Matrices.size()));

    Hr = DeviceContext->Map(InstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Mapped);

    if (FAILED(Hr))
    {
        return;
    }

    FInstanceBufferType *Data = (FInstanceBufferType *)Mapped.pData;
    for (SIZE_T i = 0; i < InstanceDataArray.Matrices.size(); i++)
    {
        Data->ModelMatrix = InstanceDataArray.Matrices[i].GetTranspose();
        Data++;
    }
    DeviceContext->Unmap(InstanceBuffer, 0);

    ID3D11Buffer *Buffers[2] = {VertexBuffer, InstanceBuffer};
    UINT          Strides[2] = {sizeof(FStaticMeshBuildVertex), sizeof(FInstanceBufferType)};
    UINT          Offsets[2] = {0, 0};

    DeviceContext->IASetVertexBuffers(0, 2, Buffers, Strides, Offsets);
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    DeviceContext->DrawInstanced(NumVertices, InstanceDataArray.Matrices.size(), 0, 0);
}

void FD3D11MeshPassRenderer::UpdateConstant(const FMatrix &World, const FMatrix &View,
                                            const FMatrix &Projection, bool bUseInstance)
{
    if (ConstantBuffer == nullptr)
    {
        return;
    }

    ID3D11DeviceContext *DeviceContext = GetDeviceContext();
    if (DeviceContext == nullptr)
    {
        return;
    }

    FMatrix    MVP = World * View * Projection;
    FConstants Constants = {};
    Constants.MVP = MVP;
    Constants.UseInstance = bUseInstance;

    DeviceContext->UpdateSubresource(ConstantBuffer, 0, nullptr, &Constants, 0, 0);
}

ID3D11Buffer *FD3D11MeshPassRenderer::CreateVertexBuffer(FStaticMeshBuildVertex *Vertices,
                                                         UINT                    ByteWidth)
{
    if (Vertices == nullptr || ByteWidth == 0)
    {
        return nullptr;
    }

    ID3D11Device *Device = GetDevice();
    if (Device == nullptr)
    {
        return nullptr;
    }

    D3D11_BUFFER_DESC BufferDesc = {};
    BufferDesc.ByteWidth = ByteWidth;
    BufferDesc.Usage = D3D11_USAGE_DEFAULT;
    BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    BufferDesc.CPUAccessFlags = 0;
    BufferDesc.MiscFlags = 0;
    BufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA InitData = {};
    InitData.pSysMem = Vertices;

    ID3D11Buffer *VertexBuffer = nullptr;
    HRESULT       Hr = Device->CreateBuffer(&BufferDesc, &InitData, &VertexBuffer);
    if (FAILED(Hr))
    {
        return nullptr;
    }

    return VertexBuffer;
}

void FD3D11MeshPassRenderer::ReleaseVertexBuffer(ID3D11Buffer *VertexBuffer)
{
    if (VertexBuffer != nullptr)
    {
        VertexBuffer->Release();
    }
}

void FD3D11MeshPassRenderer::ClearInstanceMap() 
{ 
    InstanceMap.clear();
}

void FD3D11MeshPassRenderer::CreateShader()
{
    ID3DBlob *VertexShaderBlob = nullptr;
    ID3DBlob *PixelShaderBlob = nullptr;

    const bool bCompiledVS = CompileShaderFromFile(ShaderFilePath, VertexShaderEntry,
                                                   VertexShaderTarget, VertexShaderBlob);
    if (!bCompiledVS)
    {
        return;
    }

    const bool bCreatedVS = CreateVertexShaderInternal(VertexShaderBlob);
    if (!bCreatedVS)
    {
        ReleaseBlob(VertexShaderBlob);
        return;
    }

    const bool bCompiledPS =
        CompileShaderFromFile(ShaderFilePath, PixelShaderEntry, PixelShaderTarget, PixelShaderBlob);
    if (!bCompiledPS)
    {
        ReleaseShader();
        ReleaseBlob(VertexShaderBlob);
        return;
    }

    const bool bCreatedPS = CreatePixelShaderInternal(PixelShaderBlob);
    if (!bCreatedPS)
    {
        ReleaseShader();
        ReleaseBlob(VertexShaderBlob);
        ReleaseBlob(PixelShaderBlob);
        return;
    }

    const bool bCreatedLayout = CreateInputLayoutInternal(VertexShaderBlob);
    if (!bCreatedLayout)
    {
        ReleaseShader();
        ReleaseBlob(VertexShaderBlob);
        ReleaseBlob(PixelShaderBlob);
        return;
    }

    ReleaseBlob(VertexShaderBlob);
    ReleaseBlob(PixelShaderBlob);
}

void FD3D11MeshPassRenderer::CreateConstantBuffer()
{
    ID3D11Device *Device = GetDevice();
    if (Device == nullptr)
    {
        return;
    }

    if (ConstantBuffer)
    {
        ConstantBuffer->Release();
        ConstantBuffer = nullptr;
    }

    D3D11_BUFFER_DESC BufferDesc = {};
    BufferDesc.ByteWidth = sizeof(FConstants);
    BufferDesc.Usage = D3D11_USAGE_DEFAULT;
    BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    BufferDesc.CPUAccessFlags = 0;
    BufferDesc.MiscFlags = 0;
    BufferDesc.StructureByteStride = 0;

    HRESULT Hr = Device->CreateBuffer(&BufferDesc, nullptr, &ConstantBuffer);
    if (FAILED(Hr))
    {
        ConstantBuffer = nullptr;
    }
}

void FD3D11MeshPassRenderer::CreateInstanceBuffer(uint32 NewMaxInstanceNum)
{
    CurrentMaxInstanceNum = NewMaxInstanceNum;
    ID3D11Device *Device = GetDevice();
    if (Device == nullptr)
    {
        return;
    }

    if (InstanceBuffer)
    {
        InstanceBuffer->Release();
        InstanceBuffer = nullptr;
    }

    D3D11_BUFFER_DESC BufferDesc = {};
    BufferDesc.ByteWidth = sizeof(FInstanceBufferType) * CurrentMaxInstanceNum;
    BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    BufferDesc.MiscFlags = 0;
    BufferDesc.StructureByteStride = 0;

    HRESULT Hr = Device->CreateBuffer(&BufferDesc, nullptr, &InstanceBuffer);
    if (FAILED(Hr))
    {
        InstanceBuffer = nullptr;
    }
}

void FD3D11MeshPassRenderer::ReleaseShader()
{
    if (SimpleInputLayout != nullptr)
    {
        SimpleInputLayout->Release();
        SimpleInputLayout = nullptr;
    }

    if (SimpleVertexShader != nullptr)
    {
        SimpleVertexShader->Release();
        SimpleVertexShader = nullptr;
    }

    if (SimplePixelShader != nullptr)
    {
        SimplePixelShader->Release();
        SimplePixelShader = nullptr;
    }
}

void FD3D11MeshPassRenderer::ReleaseConstantBuffer()
{
    if (ConstantBuffer != nullptr)
    {
        ConstantBuffer->Release();
        ConstantBuffer = nullptr;
    }
}

void FD3D11MeshPassRenderer::ReleaseInstanceBuffer()
{
    if (InstanceBuffer)
    {
        InstanceBuffer->Release();
        InstanceBuffer = nullptr;
    }
}

ID3D11Device *FD3D11MeshPassRenderer::GetDevice() const
{
    if (RHI == nullptr)
    {
        return nullptr;
    }

    return RHI->GetDevice();
}

ID3D11DeviceContext *FD3D11MeshPassRenderer::GetDeviceContext() const
{
    if (RHI == nullptr)
    {
        return nullptr;
    }

    return RHI->GetDeviceContext();
}

void FD3D11MeshPassRenderer::ReleaseBlob(ID3DBlob *&Blob)
{
    if (Blob != nullptr)
    {
        Blob->Release();
        Blob = nullptr;
    }
}

bool FD3D11MeshPassRenderer::CompileShaderFromFile(const wchar_t *FileName, const char *EntryPoint,
                                                   const char *Target, ID3DBlob *&OutShaderBlob)
{
    OutShaderBlob = nullptr;

    ID3DBlob *ErrorBlob = nullptr;

    HRESULT Hr = D3DCompileFromFile(FileName, nullptr, nullptr, EntryPoint, Target, 0, 0,
                                    &OutShaderBlob, &ErrorBlob);

    ReleaseBlob(ErrorBlob);

    return SUCCEEDED(Hr) && OutShaderBlob != nullptr;
}

bool FD3D11MeshPassRenderer::CreateVertexShaderInternal(ID3DBlob *VertexShaderBlob)
{
    if (VertexShaderBlob == nullptr)
    {
        return false;
    }

    ID3D11Device *Device = GetDevice();
    if (Device == nullptr)
    {
        return false;
    }

    HRESULT Hr =
        Device->CreateVertexShader(VertexShaderBlob->GetBufferPointer(),
                                   VertexShaderBlob->GetBufferSize(), nullptr, &SimpleVertexShader);
    return SUCCEEDED(Hr);
}

bool FD3D11MeshPassRenderer::CreatePixelShaderInternal(ID3DBlob *PixelShaderBlob)
{
    if (PixelShaderBlob == nullptr)
    {
        return false;
    }

    ID3D11Device *Device = GetDevice();
    if (Device == nullptr)
    {
        return false;
    }

    HRESULT Hr =
        Device->CreatePixelShader(PixelShaderBlob->GetBufferPointer(),
                                  PixelShaderBlob->GetBufferSize(), nullptr, &SimplePixelShader);
    return SUCCEEDED(Hr);
}

bool FD3D11MeshPassRenderer::CreateInputLayoutInternal(ID3DBlob *VertexShaderBlob)
{
    if (VertexShaderBlob == nullptr)
    {
        return false;
    }

    ID3D11Device *Device = GetDevice();
    if (Device == nullptr)
    {
        return false;
    }

    D3D11_INPUT_ELEMENT_DESC Layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},

        {"INSTANCE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"INSTANCE", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"INSTANCE", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"INSTANCE", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1},
    };

    HRESULT Hr =
        Device->CreateInputLayout(Layout, ARRAYSIZE(Layout), VertexShaderBlob->GetBufferPointer(),
                                  VertexShaderBlob->GetBufferSize(), &SimpleInputLayout);
    return SUCCEEDED(Hr);
}