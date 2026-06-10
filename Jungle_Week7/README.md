# KraftonEngine — Week7 : Light

> 크래프톤 정글 게임 테크랩 3기 — Week7 (주제: **Light**)

`C++20 · Win32 · DirectX 11 · ImGui` 기반 엔진 `KraftonEngine`입니다.
이번 주차의 핵심 주제는 **라이팅(Light)** 으로, 컴포넌트 기반 라이트 시스템과 포워드 라이팅 파이프라인,
그리고 지수 높이 안개(Exponential Height Fog)를 구현했습니다.

저장소에는 `Editor` · `Engine` · `ObjViewer` 세 빌드 타깃이 함께 들어 있습니다.

---

## 핵심 주제 — 라이팅

- **라이트 컴포넌트 계층** (`Source/Engine/Components` → 이후 `Component/Light`)
  - `ULightComponentBase` → `ULightComponent`
  - `UDirectionalLightComponent` (방향광)
  - `UPointLightComponent` (점광원, `LightFalloffExponent`)
  - `USpotLightComponent` (스포트라이트)
  - `UAmbientLightComponent` (앰비언트)
  - `ULocalLightComponent`
- **렌더 프록시** — `FLightSceneProxy` (`Render/Proxy/LightSceneProxy`)
- **포워드 라이팅** — 씬의 라이트를 GPU 유니폼으로 전달해 포워드 패스에서 셰이딩
- **지수 높이 안개** — `UExponentialHeightFogComponent`
  (FogDensity, FogHeightFalloff, FogHeight, StartDistance, FogCutoffDistance, FogMaxOpacity)
  - 셰이더: `Shaders/Common/FogCommon.hlsl` (투과도 계산 및 안개 블렌딩)

지원 라이트 타입: **Directional · Point · Spot · Ambient**

---

## 소스 구조

```
KraftonEngine/Source/
├── Engine/
│   ├── Object/ GameFramework/ Components/   # UObject, Actor, 라이트 등 컴포넌트
│   ├── Render/                              # Device(D3D11), Pipeline, Proxy(Light), Fog …
│   ├── Materials/ Mesh/ Texture/ Resource/  # 머티리얼·메시·텍스처·리소스
│   ├── Collision/ Math/ Core/ Input/        # 충돌·수학·코어·입력
│   ├── Serialization/ Profiling/ FileSystem/
│   └── Platform/ Runtime/ UI/ Viewport/
├── Editor/    # PIE, Selection, Subsystem, UI, Viewport
└── ObjViewer/ # OBJ 모델 검수 전용 뷰어
```

---

## 빌드 & 실행

```bat
GenerateProjectFiles.bat            :: VS 프로젝트 파일 생성
:: 생성된 KraftonEngine.sln 을 Visual Studio 2022로 열어 빌드 (x64)

ReleaseBuild.bat                    :: 릴리스 빌드 + 패키징
ReleaseWithObjViewerBuild.bat       :: ObjViewer 포함 릴리스 빌드
DemoBuild.bat                       :: 데모 빌드
```

- **요구사항**: Visual Studio 2022, Windows SDK(DirectX 11)
- **의존성**: DirectXTK 등은 NuGet으로 자동 복원(`nuget.config`)
