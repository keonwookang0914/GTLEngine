# NipsEngine — Week6 : Decal

> 크래프톤 정글 게임 테크랩 3기 — Week6 (주제: **Decal**)

`C++ · Win32 · DirectX 11 · ImGui` 기반의 3D 에디터 엔진 `NipsEngine`입니다.
이번 주차의 핵심 주제는 **데칼(Decal) 렌더링**으로, OBB 투영 기반의 데칼 컴포넌트와 전용 셰이더를 추가했습니다.
기존의 `UObject / Actor / Component` 구조, StaticMesh 파이프라인, ImGui 에디터 위에 데칼과 포스트프로세스가 얹혀 있습니다.

---

## 핵심 주제 — Decal

- **`UDecalComponent`** (`UPrimitiveComponent` 상속)
  - 텍스처 로딩/캐시(`ResourceManager` 연동)
  - 페이드 제어: `FadeAlpha`, `FadeInStartDelay`, `FadeInDuration`, `FadeStartDelay`, `FadeDuration`
    (페이드 값이 0이면 영구 데칼)
  - `GetDecalOBB()`, `GetDecalViewProjection()` — 투영 기반 데칼 렌더링
  - 에디터 피킹을 위한 레이캐스트 지원
- **`ShaderDecal.hlsl`**
  - `DecalViewProjection` 행렬로 데칼을 표면에 투영
  - OBB 범위를 벗어난 픽셀은 클리핑(discard)
  - `DecalTexture` 샘플 후 `FadeAlpha`로 블렌딩 (픽셀별 페이드)

---

## 엔진 기능

- **오브젝트 시스템** — `UObject` + 커스텀 RTTI(`DECLARE_CLASS`/`DEFINE_CLASS`), `FName` 풀, `ObjectIterator`,
  World/Level/Actor/Component 계층
- **렌더링 (DirectX 11)** — PrimitiveComponent 렌더링, 머티리얼 슬롯,
  포스트프로세스(Outline, FXAA, HeightFog, FireBall, DepthScene),
  View Mode(Lit/Unlit/Wireframe), 스텐실 기반 선택 마스크
- **에셋** — OBJ/MTL 로더, StaticMesh, 바이너리 메시 캐시(`.bin`), 텍스처 리소스 관리
- **에디터 (ImGui)** — 4분할 뷰포트, 아웃라이너, 프로퍼티 에디터, 머티리얼 에디터,
  기즈모(Translate/Rotate/Scale), 콘솔, 스탯 프로파일러
- **기타** — SubUV 애니메이션, 빌보드 텍스트, 무브먼트 컴포넌트(Projectile/Rotating)

---

## 소스 구조

```
NipsEngine/Source/Engine/
├── Object/         # UObject, RTTI, FName, ObjectIterator
├── GameFramework/  # AActor, Level, World, WorldContext
├── Component/      # DecalComponent, StaticMeshComponent, CameraComponent …
├── Core/           # ResourceManager, ResourceTypes
├── Geometry/       # AABB, OBB, Frustum, Ray
├── Math/           # Matrix, Vector …
├── Render/         # 렌더러, Material, PostProcess, Shader, RenderBus/Collector
├── Asset/          # OBJ/MTL 파싱, StaticMesh, 바이너리 직렬화
├── Serialization/  # JSON 씬 저장/복원
├── Runtime/        # 프레임 타이밍, 월드 틱
└── Slate/          # ImGui 기반 에디터 위젯
```

---

## 빌드 & 실행

```bat
GenerateProjectFiles.bat   :: VS 프로젝트 파일 생성 (포함된 Python 사용)
:: 생성된 NipsEngine.sln 을 Visual Studio 2022로 열어 빌드 (x64)

ReleaseBuild.bat           :: Release|x64 빌드 후 exe/Shaders/Asset/Settings/Saves 를
                           ::  ReleaseBuild/ 로 패키징
```

- **요구사항**: Visual Studio 2022, Windows SDK(DirectX 11)
- **서드파티**: ImGui, SimpleJSON (`ThirdParty/`), 그 외 NuGet 패키지
