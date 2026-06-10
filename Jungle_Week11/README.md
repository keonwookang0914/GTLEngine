# KraftonEngine — Week11 : Animation

> 크래프톤 정글 게임 테크랩 3기 — Week11 (주제: **Animation**)

`C++20 · Win32 · DirectX 11 · ImGui` 기반 엔진 `KraftonEngine`입니다.
Week10의 FBX/스켈레탈 메시 위에 **애니메이션 시스템**을 올린 주차로,
애니메이션 시퀀스 재생부터 상태 머신·애님 그래프·노티파이·2-본 IK까지 다룹니다.

---

## 핵심 주제 — 애니메이션 시스템 (`Source/Engine/Animation`)

- **`UAnimInstance`** — 스켈레탈 메시 애니메이션 플레이어 베이스
- **`UAnimSequenceBase`** → **`UAnimSequence`** — 단일 애니메이션 클립(`UAnimDataModel` 래핑)
- **`UAnimDataModel`** — FBX에서 임포트한 원시 애니메이션 데이터
  (본 트랙의 pos/rot/scale 키, 노티파이 이벤트, 프레임 레이트, 재생 길이)
- **`UAnimSingleNodeInstance`** — 단일 시퀀스 재생(Play/Stop/Looping)
- **`UAnimGraphInstance`** — 노드 그래프/상태 머신 기반 평가
  - `Blend2ByFloat`(float 파라미터로 두 시퀀스 블렌딩)
  - 시퀀스 플레이어, 파라미터(float/bool) 전달
- **`AnimationStateMachine`** — 상태 전이(블렌드 듀레이션/알파)
- **데이터 구조** (`AnimTypes.h`)
  - `FRawAnimSequenceTrack`, `FBoneAnimationTrack`, `FPoseContext`(로컬 본 트랜스폼)
  - `FAnimNotifyEvent`(프레임 시점 이벤트), `FTwoBoneIKChain`(2-본 IK)
- **`AnimInstanceAsset`** — 애님 그래프/상태 머신 애셋 래퍼

지원: 스켈레탈 애니메이션 · 클립 재생 · 상태 머신 전이 · 노드 그래프 · 노티파이 · 2-본 IK

---

## 소스 구조

```
KraftonEngine/Source/Engine/
├── Animation/     # AnimInstance, AnimSequence, AnimDataModel, AnimGraphInstance, StateMachine
├── Mesh/          # FbxImporter, SkeletalMesh
├── Component/ GameFramework/   # SkeletalMeshComponent/Actor
├── Render/        # SkeletalMeshSceneProxy(스키닝), Device, Pipeline
├── Physics/ Lua/ FloatCurve/ Gizmo/ CameraShake/ Asset/ Debug/
├── Materials/ Texture/ Resource/ Collision/ Math/ Core/ Input/
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
- **서드파티**: Autodesk FBX SDK, NuGet(**NVIDIA.PhysX 4.1**, **DirectXTK 2026.x**),
  ImGui, RmlUi, SimpleJSON, FMOD, Lua, sol2
