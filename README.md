# 크래프톤 정글 게임 테크랩 3기 — 자체 엔진 제작 프로젝트 (Week1 ~ Week14)

> **C++20 · Win32 · DirectX 11 · ImGui** 기반으로 14주 동안 밑바닥부터 직접 만든 게임 엔진과 에디터.

크래프톤 정글 게임 테크랩 3기에서 **14주에 걸쳐 진행한 자체 엔진 제작 프로젝트**입니다.
2D 게임 잼에서 시작해, 매주 하나의 기술 주제를 더해가며 언리얼 엔진을 닮은 `UObject / Actor / Component` 구조 위에
렌더러 · 에디터 · 애니메이션 · 파티클 · 물리(PhysX)까지 갖춘 3D 엔진으로 키워나간 과정의 기록입니다.

각 주차 폴더(`Jungle_Week1` ~ `Jungle_Week14`)는 **그 주차의 독립적인 엔진 스냅샷**입니다.
정글 과정 특성상 매주 팀 구성이 바뀌었고, 그에 따라 엔진 이름도 함께 바뀝니다
(예: `NipsEngine`, `LunaticEngine`, `JSEngine`, `KraftonEngine`). 그래서 이 저장소는
하나의 엔진을 이어 커밋한 것이 아니라, **14번의 도전을 모아 둔 포트폴리오**에 가깝습니다.

---

## 14주 로드맵

| 주차 | 주제 |
|:---:|:---:|
| **Week1** | Game Jam #1 |
| **Week2** | 3D |
| **Week3** | Texture|
| **Week4** | OBJ|
| **Week5** | Game Jam #2 |
| **Week5+** | PIE |
| **Week6** | Decal |
| **Week7** | Light |
| **Week8** | Shadow |
| **Week9** | Game Jam #3 |
| **Week9+** | Cinematic |
| **Week10** | FBX |
| **Week11** | Animation |
| **Week12** | Particle |
| **Week13** | PhysX |
| **Week14** | Game Jam #4 |

---

## 공통 아키텍처

주차마다 이름과 세부 구조는 달라지지만, 다음 설계 원칙은 14주 내내 일관되게 유지·발전했습니다.

- **오브젝트 모델** — 언리얼 스타일의 `UObject → AActor → UComponent` 계층.
  자체 구현한 **RTTI**, **`FName`** 기반 이름 관리, `ObjectFactory`를 통한 인스턴스 생성.
- **렌더링** — **DirectX 11** 포워드 렌더링 파이프라인.
  `Render/Device`(스왑체인·래스터라이저·뎁스), 씬 프록시(`SceneProxy`), 렌더 커맨드/컬렉터, 컬링.
- **에디터** — **ImGui** 기반 툴링. 아웃라이너 · 프로퍼티 · 콘솔 · 스탯 프로파일러 · 머티리얼 에디터 ·
  멀티 뷰포트 · 기즈모(Translate/Rotate/Scale) · 레이캐스트 피킹 · 박스 선택.
- **월드 / 씬** — `UWorld`와 씬 컨텍스트로 액터를 관리하고, JSON으로 씬을 저장·복원.
  중반 이후 **PIE(Play-In-Editor)** 로 편집 월드와 플레이 월드를 분리.
- **에셋 파이프라인** — OBJ → 쿡된 `StaticMesh`, FBX → `SkeletalMesh`, 바이너리 베이킹/캐시 로딩.

### 그래픽스 컨벤션
- 좌표계: **왼손 좌표계** · 행렬: **Row-major**, 왼쪽→오른쪽 곱
- 회전: 오일러 각(Pitch/Yaw/Roll), **Degree**, 적용 순서 **Yaw → Pitch → Roll**
- 면 노멀 기준: **CCW** · NDC 범위: `[-1,-1,0] ~ [1,1,1]`

### 코드 컨벤션 (언리얼 접두사 차용)
`F`(값 타입/struct) · `U`(UObject 계열) · `A`(Actor 계열) · `T`(템플릿) · `S`(Slate) · `I`(인터페이스) · `E`(enum)

---

## 기술 스택

| 구분 | 사용 기술 |
|:---|:---|
| 언어 / 표준 | C++20 |
| 플랫폼 | Win32 (Windows) |
| 그래픽스 | DirectX 11, HLSL |
| 그래픽스 헬퍼 | DirectXTK |
| UI / 에디터 | Dear ImGui |
| 직렬화 | SimpleJSON (커스텀 JSON) |
| 스크립팅 | LuaJIT + sol2 , Lua 블루프린트 |
| 오디오 | FMOD, miniaudio |
| 애셋 임포트 | Autodesk FBX SDK |
| 물리 | NVIDIA PhysX 4.x, NvCloth |

---

## 빌드 & 실행

```
요구사항: Visual Studio 2022 (v143 툴셋)
Windows SDK(DirectX 11 포함), x64.
대부분의 의존성(ImGui, DirectXTK, SimpleJSON 등)은 프로젝트에 포함되거나 NuGet으로 자동 복원됩니다.
```