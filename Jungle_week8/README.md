# KraftonEngine — Week8 : Shadow

> 크래프톤 정글 게임 테크랩 3기 — Week8 (주제: **Shadow**)

`C++20 · Win32 · DirectX 11 · ImGui` 기반 엔진 `KraftonEngine`입니다.
Week7의 라이팅 위에 **그림자(Shadow Mapping)** 를 올린 주차로,
큐브맵 섀도우 풀, 디렉셔널 라이트용 아틀라스/캐스케이드 섀도우, VSM(Variance Shadow Map)까지 다룹니다.

---

## 핵심 주제 — 섀도우 매핑

- **섀도우 풀** — `FTextureCubeShadowPool` (`Render/Resource/TexturePool`)
  - 큐브맵 그림자 관리(면 6장), **4단계 해상도 티어**(`TierCount=4`)
  - VSM(Variance Shadow Mapping) 지원
  - 핸들 구조: `FCubeShadowHandle`(CubeIndex / TierIndex)
- **그림자 데이터 구조** — `FShadowMapKey`(아틀라스/큐브맵 변형), `FShadowInfo`(PSM 플래그, CubeTierIndex)
- **포워드 라이트 GPU 데이터** — `ForwardLightData.h`
  (`FAmbientLightGPU`, `FDirectionalLightGPU`, `FShadowInfo` + 섀도우 행렬)
- **그림자를 지원하는 라이트** — Directional / Point / Spot / Ambient
  (`UDirectionalLightComponent::GetShadowMapKey()`, `GetShadowHandleSet()` 등)
- **셰이더**
  - `Shaders/Geometry/ShadowDepth.hlsl` (PSM 투영, VSM 깊이 분산 저장)
  - `Shaders/Geometry/ShadowClear.hlsl`
  - `Shaders/Editor/ShadowDepthDebug.hlsl`
- **에디터 UI** — `EditorShadowPropertyWidget`
  (섀도우 해상도 스케일, Bias / Slope Bias, Sharpen, 아틀라스 레이어 프리뷰)

그림자 종류: **점/스포트 라이트(큐브맵) · 디렉셔널 라이트(아틀라스/캐스케이드) · VSM**

---

## 소스 구조

```
KraftonEngine/Source/
├── Engine/
│   ├── Component/Light/   # 그림자 지원 라이트 컴포넌트
│   ├── Render/
│   │   ├── Resource/TexturePool/   # TextureCubeShadowPool
│   │   └── Proxy/ Pipeline/ Device/
│   ├── Mesh/ Materials/ Texture/ Resource/ Collision/ Math/
│   ├── Core/ Input/ Serialization/ Profiling/
│   └── Platform/ Runtime/ UI/ Viewport/
├── Editor/    # EditorShadowPropertyWidget 등
└── ObjViewer/
```

---

## 빌드 & 실행

```bat
GenerateProjectFiles.bat            :: VS 프로젝트 파일 생성
:: 생성된 KraftonEngine.sln 을 Visual Studio 2022로 열어 빌드 (x64)

ReleaseBuild.bat / ReleaseWithObjViewerBuild.bat / DemoBuild.bat
```

- **요구사항**: Visual Studio 2022, Windows SDK(DirectX 11)
- **의존성**: DirectXTK 등 NuGet 자동 복원(`nuget.config`)

> 폴더명이 소문자(`Jungle_week8`)인 점에 유의하세요.
