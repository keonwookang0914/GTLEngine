# JSEngine — Week12 : Particle

> 크래프톤 정글 게임 테크랩 3기 — Week12 (주제: **Particle**)

`C++20 · Win32 · DirectX 11 · ImGui` 기반 엔진 `JSEngine`입니다.
이번 주차의 핵심 주제는 **파티클 시스템**으로, 모듈 기반 이미터 구조를 구현했습니다.
더불어 **지오메트리 프리미티브**, **공간 분할(BVH/KD-Tree)**, 자체 **Slate UI 프레임워크**가 함께 도입되었습니다.

---

## 핵심 주제 — 파티클 시스템 (`Source/Engine/Particle`)

- **`UParticleSystemComponent`** — 파티클 시스템 소유 컴포넌트
- **`FParticleEmitterInstance`** — 이미터 런타임 인스턴스
- **`ParticleModules`** — 모듈 기반 파티클 동작
  (Required, Spawn, Lifetime, Velocity, Acceleration, Orbit, Scale, Color, Rotation, Collision, Beam …)
- **`ParticleAsset`** — 파티클 시스템 애셋 컨테이너
- **`ParticleEventManager`** — 파티클 이벤트 디스패치
- **`ParticleDistributions`** — 파라미터용 랜덤 분포 커브
- **`ParticleBeamPath`** — 빔/트레일 경로 생성
- **타입**: Sprite · Mesh · Beam · Ribbon (`EDynamicEmitterType`)

## 함께 도입된 시스템

- **Geometry** (`Source/Engine/Geometry`) — AABB, OBB, Frustum, Plane, Triangle, Ray, Edge, Transform
- **Spatial** (`Source/Engine/Spatial`) — 공간 가속 구조
  - `BVH`(Bounding Volume Hierarchy), `KDTree`, `WorldSpatialIndex`(월드 단위 공간 인덱스)
- **Slate UI** (`Source/Engine/Slate`) — 자체 위젯 프레임워크
  - `SlateApplication`(입력/페인트/포커스 관리), `SWidget`, `SWindow`, `SViewport`,
    `SSplitter`(H/V/Cross 도킹 스플리터)

---

## 소스 구조

```
JSEngine/Source/Engine/
├── Particle/      # ParticleSystemComponent, EmitterInstance, Modules, Asset, EventManager
├── Geometry/      # AABB, OBB, Frustum, Plane, Triangle, Ray, Transform
├── Spatial/       # BVH, KDTree, WorldSpatialIndex
├── Slate/         # SlateApplication, SWidget, SWindow, SViewport, SSplitter
├── Animation/ Asset/ Audio/ Camera/ Component/ Collision/
├── Render/ Object/ GameFramework/ Math/ Input/ Core/
└── Serialization/ Runtime/ UI/
```

---

## 빌드 & 실행

```bat
GenerateProjectFiles.bat   :: VS 프로젝트 파일 생성
:: 생성된 JSEngine.sln 을 Visual Studio 2022로 열어 빌드 (x64)

GameBuild.bat / ReleaseBuild.bat
CheckMissingLFS.bat        :: Git LFS 누락 파일 검사 (대용량 에셋용)
```

- **요구사항**: Visual Studio 2022, Windows SDK(DirectX 11), **Git LFS**
- **서드파티**: NuGet(**DirectXTK 2026.x**), ImGui, RmlUi 등
