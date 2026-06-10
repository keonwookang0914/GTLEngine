# KraftonEngine — Week10 : FBX

> 크래프톤 정글 게임 테크랩 3기 — Week10 (주제: **FBX**)

`C++20 · Win32 · DirectX 11 · ImGui` 기반 엔진 `KraftonEngine`입니다.
이번 주차의 핵심 주제는 **FBX 임포트와 스켈레탈 메시(Skeletal Mesh)** 로,
Autodesk FBX SDK로 본 계층과 스킨 데이터를 읽어와 GPU 스키닝으로 렌더링하는 파이프라인을 구축했습니다.
또한 이 시점부터 **PhysX**(NuGet)와 **Lua**가 엔진에 합류합니다.

---

## 핵심 주제 — FBX & 스켈레탈 메시

- **FBX 임포터** — `Mesh/FbxImporter` (`#include <fbxsdk.h>`)
  - 본 계층 파싱(`ParseBone`), 스킨 데이터 파싱(`ParseSkin`)
  - 머티리얼 수집, 탄젠트 생성, 삼각형화(triangulation)
- **스켈레탈 메시 클래스**
  - `USkeletalMesh` / `SkeletalMeshAsset` — 스켈레탈 메시 애셋 데이터
  - `USkeletalMeshComponent` — 메시 + 애니메이션 인스턴스 소유 컴포넌트
  - `ASkeletalMeshActor` — 액터 래퍼
  - `FSkeletalMeshSceneProxy` — 동적 정점 버퍼 + 스키닝 렌더 프록시
- **스키닝 정점** — `FVertexPNCTBW` (본 인덱스/가중치 포함), `FBone` 배열로 스켈레톤 구성

---

## 소스 구조

```
KraftonEngine/Source/Engine/
├── Mesh/          # FbxImporter, SkeletalMesh(Asset), StaticMesh
├── Component/     # SkeletalMeshComponent 등
├── GameFramework/ # SkeletalMeshActor, AActor, UWorld
├── Render/        # Proxy(SkeletalMeshSceneProxy), Device(D3D11), Pipeline
├── Physics/       # PhysX 연동 (이번 주차 도입)
├── Lua/           # Lua 스크립팅 (도입)
├── Animation 기반(FloatCurve)/ Gizmo/ CameraShake/ Asset/ Debug/
├── Materials/ Texture/ Resource/ Collision/ Math/ Core/ Input/
└── Serialization/ Profiling/ Platform/ Runtime/ UI/ Viewport/
```

---

## 빌드 & 실행

```bat
GenerateProjectFiles.bat   :: VS 프로젝트 파일 생성
:: 생성된 KraftonEngine.sln 을 Visual Studio 2022로 열어 빌드 (x64)

GameBuild.bat              :: 게임(클라이언트) 빌드 + 패키징
ReleaseBuild.bat           :: 릴리스 빌드
```

- **요구사항**: Visual Studio 2022, Windows SDK(DirectX 11)
- **서드파티**
  - Autodesk **FBX SDK** (`ThirdParty/fbx` — include/lib)
  - NuGet: **NVIDIA.PhysX 4.1.2**, **DirectXTK**
  - 그 외: ImGui, RmlUi, SimpleJSON, FMOD, Lua, sol2
