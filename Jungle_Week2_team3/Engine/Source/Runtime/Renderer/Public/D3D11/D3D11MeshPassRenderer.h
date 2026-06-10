#pragma once

#include "Containers/Array.h"
#include "HAL/Platform.h"
#include "Math/Matrix.h"
#include "Math/Vector4.h"
#include <d3d11.h>
#include <unordered_map>


class FD3D11DynamicRHI;
struct FStaticMeshBuildVertex;

struct alignas(16) FConstants
{
    FMatrix MVP;
    int     UseInstance;
};

struct alignas(16) FInstanceBufferType
{
    FMatrix ModelMatrix;
};

struct FInstanceDataArray
{
    TArray<FMatrix> Matrices;
    uint32          VertexNum;
    uint32          NextInstanceNum;

    FInstanceDataArray() : VertexNum(0), NextInstanceNum(0) {}

    FInstanceDataArray(uint32 InVertexNum) : VertexNum(InVertexNum), NextInstanceNum(0) {}
};

class FD3D11MeshPassRenderer
{
  public:
    void Initialize(FD3D11DynamicRHI *InRHI);
    void Shutdown();

    void Draw();

    void PrepareShader();
    void RenderPrimitive(ID3D11Buffer *VertexBuffer, UINT NumVertices);
    void RegisterInstance(ID3D11Buffer *VertexBuffer, UINT NumVertices,
                          const FMatrix &InModelMatrix);
    void RenderInstance(ID3D11Buffer *VertexBuffer, UINT NumVertices);

    void UpdateConstant(const FMatrix &World, const FMatrix &View, const FMatrix &Projection,
                        bool bUseInstance = false);

    ID3D11Buffer *CreateVertexBuffer(FStaticMeshBuildVertex *Vertices, UINT ByteWidth);
    void          ReleaseVertexBuffer(ID3D11Buffer *VertexBuffer);

    void ClearInstanceMap();

  private:
    void CreateShader();
    void CreateConstantBuffer();
    void CreateInstanceBuffer(uint32 NewMaxInstanceNum);
    void ReleaseShader();
    void ReleaseConstantBuffer();
    void ReleaseInstanceBuffer();

  private:
    ID3D11Device        *GetDevice() const;
    ID3D11DeviceContext *GetDeviceContext() const;

    static void ReleaseBlob(ID3DBlob *&Blob);

    bool CompileShaderFromFile(const wchar_t *FileName, const char *EntryPoint, const char *Target,
                               ID3DBlob *&OutShaderBlob);

    bool CreateVertexShaderInternal(ID3DBlob *VertexShaderBlob);
    bool CreatePixelShaderInternal(ID3DBlob *PixelShaderBlob);
    bool CreateInputLayoutInternal(ID3DBlob *VertexShaderBlob);

  private:
    FD3D11DynamicRHI *RHI = nullptr;

    ID3D11InputLayout  *SimpleInputLayout = nullptr;
    ID3D11VertexShader *SimpleVertexShader = nullptr;
    ID3D11PixelShader  *SimplePixelShader = nullptr;
    ID3D11Buffer       *ConstantBuffer = nullptr;

    UINT Stride = 0;

    uint32                                                 CurrentMaxInstanceNum = 1024;
    ID3D11Buffer                                          *InstanceBuffer = nullptr;
    std::unordered_map<ID3D11Buffer *, FInstanceDataArray> InstanceMap;

};