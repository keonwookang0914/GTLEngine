# KraftonEngine — Week14 : Game Jam #4 (Final)

> 크래프톤 정글 게임 테크랩 3기 — Week14 (주제: **Game Jam #4**)

14주 프로젝트의 **마지막 주차**입니다. 그동안 쌓아 올린 `KraftonEngine`
(라이팅·섀도우·FBX/스켈레탈·애니메이션·파티클·PhysX·Lua·오디오)을 토대로 진행한 네 번째 게임 잼이며,
게임 제작을 가속하기 위한 **Lua 블루프린트(비주얼 스크립팅)**, **정돈된 PhysicsEngine**, **디스트리뷰션 시스템**을 추가했습니다.

---

## 핵심 주제

### 1. Lua 블루프린트 (비주얼 스크립팅) — `Source/Engine/LuaBlueprint`
- `ULuaBlueprintAsset` — 그래프/노드 직렬화를 포함한 블루프린트 애셋
- `LuaBlueprintCompiler` — 비주얼 그래프를 **Lua 소스로 컴파일**
- `LuaBlueprintManager` — 애셋 로딩/캐시
- `LuaBlueprintTypes` — **70여 종의 노드 타입**
  (EventBeginPlay, EventTick, EventHit, CallFunction, SpawnActor, AddForce …)
- 지원: Exec 핀, 데이터 타입(Bool/Int/Float/String/Vector/Object/Array),
  그래프 순회, 리플렉션 기반 함수 호출, 배열 반복, 커스텀 이벤트

### 2. 정돈된 PhysicsEngine — `Source/Engine/PhysicsEngine`
- `FKAggregateGeom` — 여러 셰이프를 담는 컨테이너(비지터 패턴, 평면/이름 인덱싱)
- 셰이프 엘리먼트: `FKBoxElem`, `FKSphereElem`, `FKSphylElem`(캡슐), `FKConvexElem`,
  베이스 `FKShapeElem`(`EAggCollisionShape`)
- `FBodySetup`, `UPhysicsAsset`, `PhysicsAssetBuilder`, `PhysicsAssetManager`
  → PhysX 구현 세부와 분리된 고수준 셰이프/바디 추상화

### 3. 디스트리뷰션 — `Source/Engine/Distributions`
- `UDistribution`(추상, `GetRange()`), `UDistributionFloat`, `UDistributionVector`
- 파티클·이펙트의 파라미터 랜덤화에 사용

---

## 소스 구조 (Week14 시점, 약 31개 모듈)

```
KraftonEngine/Source/Engine/
├── LuaBlueprint/   # 비주얼 스크립팅 → Lua 컴파일
├── PhysicsEngine/  # AggregateGeom, ShapeElem, BodySetup, PhysicsAsset(Builder/Manager)
├── Distributions/  # DistributionFloat/Vector
├── Physics/ Cloth/         # PhysX 4.1, NvCloth (계승)
├── Animation/ Mesh/ Particles/   # 스켈레탈·애니메이션·파티클 (계승)
├── Lua/ Audio/ CameraShake/ Gizmo/ FloatCurve/ Asset/ Debug/
├── Component/ GameFramework/ Object/ Collision/ Math/
├── Render/ Materials/ Texture/ Resource/ Core/ Input/
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
- **물리**: NVIDIA PhysX 4.1 + NvCloth
- **의존성**: DirectXTK 등 NuGet 자동 복원(`nuget.config`)

> 14주의 마무리 — 렌더링·물리·애니메이션·스크립팅을 갖춘 엔진 위에서 실제 게임을 만든 주차입니다.
