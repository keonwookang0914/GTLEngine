# KraftonEngine — Week13 : PhysX

> 크래프톤 정글 게임 테크랩 3기 — Week13 (주제: **PhysX**)

`C++20 · Win32 · DirectX 11 · ImGui` 기반 엔진 `KraftonEngine`입니다.
이번 주차의 핵심 주제는 **NVIDIA PhysX(4.1) 통합**으로, 리지드 바디·조인트(래그돌)·물리 쿼리를 구현하고
**NvCloth 기반 천(Cloth) 시뮬레이션**까지 붙였습니다.

---

## 핵심 주제 — PhysX 통합 (`Source/Engine/Physics`)

- **PhysX 코어 / 씬**
  - `FPhysXCore` — PxFoundation / PxPhysics 싱글톤 관리
  - `FPhysXPhysicsScene` — PxScene 관리(메인 물리 씬)
  - `PhysXCollision`, `PhysXQueryUtils`(레이캐스트/오버랩 쿼리), `PhysXSimulationCallback`(컨택트 콜백)
- **바디 / 컨스트레인트**
  - `FBodyInstance` — PxRigidActor 런타임 래퍼
  - `FBodySetup` — 콜리전 셰이프 템플릿 + PhysX 쿠킹(`PX_PHYSICS_VERSION` 추적)
  - `FConstraintInstance` — PxJoint(D6 조인트, 래그돌 구속)
  - `UPhysicsAsset` — 스켈레탈 피직스 애셋(BodySetup + ConstraintSetup 배열)
- **물리 머티리얼** — `PhysicalMaterial` / `PhysicalMaterialManager`

## 천(Cloth) 시뮬레이션 (`Source/Engine/Cloth`)

- `ClothInstance`, `ClothMesh`, `ClothScene`, `NvClothContext`
- PhysX 브리지: `ClothCollisionBridge`, `ClothCollisionBuilder`, `ClothCollisionTypes`,
  `PhysXClothCollisionReader`(NvCloth 콜리전 ↔ PhysX 바디 연동)

---

## 소스 구조

```
KraftonEngine/Source/Engine/
├── Physics/       # PhysX(Core/Scene/Collision/Query/Callback), BodyInstance/Setup,
│                  #  ConstraintInstance, PhysicsAsset
├── Cloth/         # ClothInstance/Mesh/Scene, NvClothContext, Collision 브리지
├── Animation/ Mesh/ Component/ GameFramework/   # 스켈레탈/애니메이션 (계승)
├── Particles/ Lua/ Gizmo/ FloatCurve/ CameraShake/ Asset/ Debug/
├── Render/ Materials/ Texture/ Resource/ Collision/ Math/ Core/ Input/
└── Serialization/ Profiling/ Platform/ Runtime/ UI/ Viewport/
```

---

## 빌드 & 실행

```bat
GenerateProjectFiles.bat   :: VS 프로젝트 파일 생성
:: 생성된 KraftonEngine.sln 을 Visual Studio 2022로 열어 빌드 (x64)

GameBuild.bat / ReleaseBuild.bat
```

- **요구사항**: Visual Studio 2022, Windows SDK(DirectX 11)
- **물리**: **NVIDIA PhysX 4.1** + NvCloth (PhysX SDK 연동)
- **의존성**: DirectXTK 등 NuGet 자동 복원(`nuget.config`)
