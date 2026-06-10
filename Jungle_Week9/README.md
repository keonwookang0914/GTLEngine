# LunaticEngine — Week9 : Game Jam #3 + Cinematic

> 크래프톤 정글 게임 테크랩 3기 — Week9 (주제: **Game Jam #3 / Cinematic**)

`C++20 · Win32 · DirectX 11 · ImGui` 기반 엔진 `LunaticEngine`입니다.
세 번째 게임 잼 주차로, 엔진에 **Lua 스크립팅**, **오디오**, **시네마틱 카메라**를 도입해
실제 게임을 만들 수 있는 런타임을 갖췄습니다. (Week7~8의 라이팅/섀도우는 그대로 계승)

---

## 핵심 주제

### 1. Lua 스크립팅
- `FLuaScriptRuntime` — 전역 Lua VM 관리(핫 리로드 지원, 싱글톤)
- `ULuaScriptInstance` — 액터에 붙여 Lua 스크립트를 실행하는 컴포넌트
- `FLuaCoroutineScheduler` — Lua 코루틴/비동기 태스크 스케줄링
- `LuaActorProxy` / `LuaComponentProxy` — `AActor` / `UComponent`의 Lua 바인딩
- 타입 바인딩: Vector, Rotator, Color, Component, Actor

### 2. 오디오
- `FAudioManager`(싱글톤) — `PlayAudio` / `PlaySFX` / `PlayBackground` / Stop / Pause / Resume,
  `SetCategoryVolume`
- `AudioTypes.h` — `ESoundCategory`(SFX / Background), 루프·카테고리·볼륨 제어

### 3. 시네마틱 카메라
- `FPlayerCameraManager` — 카메라 매니저
- `FCameraModifier` → `FCameraModifier_CameraShake` — 카메라 셰이크
- `FCameraShake` / `FCameraShakePattern` — 셰이크 인스턴스/패턴
- `FLuaCameraModifier` — Lua로 제어 가능한 카메라 모디파이어
- `FMinimalViewInfo` — 카메라 뷰 파라미터

---

## 소스 구조

```
LunaticEngine/Source/
├── Engine/
│   ├── Scripting/        # FLuaScriptRuntime, LuaScriptInstance, CoroutineScheduler, 바인딩
│   ├── Audio/            # FAudioManager, AudioTypes
│   ├── Camera/           # PlayerCameraManager, CameraModifier, CameraShake
│   ├── Component/Light/   Component/Movement/   Component/Shape/
│   ├── Render/ Materials/ Mesh/ Texture/ Resource/ Collision/
│   ├── Asset/ Debug/ Core/ Input/ Math/ Serialization/ Profiling/
│   └── Platform/ Runtime/ UI/ Viewport/
├── Editor/    # 에디터 툴링
├── Game/      # 게임 잼 게임 코드
└── ObjViewer/
```

---

## 빌드 & 실행

```bat
GenerateProjectFiles.bat            :: VS 프로젝트 파일 생성
:: 생성된 LunaticEngine.sln 을 Visual Studio 2022로 열어 빌드 (x64)

ReleaseBuild.bat / ReleaseWithObjViewerBuild.bat / DemoBuild.bat
ShippingBuild.bat                   :: 배포용 십핑 빌드 (이번 주차 추가)
```

- **요구사항**: Visual Studio 2022, Windows SDK(DirectX 11)
- **서드파티**(`ThirdParty/`): ImGui, miniaudio, sol2(Lua 바인딩), SimpleJSON + NuGet(DirectXTK)
