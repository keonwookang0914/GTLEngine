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

| 주차 | 주제 | 한 줄 요약 |
|:---:|:---|:---|
| **Week1** | Game Jam #1 | 2D 충돌·씬·UI를 직접 구현한 첫 게임 잼 (DirectX, FMOD) |
| **Week2** | 3D | Win32 + D3D11 + ImGui 3D 렌더 골격, `UObject` 계층의 시작 |
| **Week3** | Texture | 텍스처 매핑, 레이캐스트 피킹, 멀티 씬, JSON 직렬화 |
| **Week4** | OBJ | OBJ/MTL 로더, `StaticMesh`, 4분할 뷰포트, 머티리얼 에디터 |
| **Week5** | Game Jam #2 | 엔진 / 에디터 / 클라이언트 / ObjViewer 멀티 타깃 분리 |
| **Week5+** | PIE | Play-In-Editor — 에디터 월드와 플레이 월드 분리 |
| **Week6** | Decal | 데칼 렌더링 (`NipsEngine`) |
| **Week7** | Light | 라이트 컴포넌트(Directional/Ambient), 포워드 라이팅, 포그 |
| **Week8** | Shadow | 섀도우 매핑, 큐브맵 섀도우 풀 |
| **Week9** | Game Jam #3 | Lua 스크립팅, 오디오, 카메라 시스템 (`LunaticEngine`) |
| **Week9+** | Cinematic | 시네마틱 / 카메라 연출 |
| **Week10** | FBX | FBX 임포터, 스켈레탈 메시 파이프라인, PhysX 도입 |
| **Week11** | Animation | 애니메이션 시퀀스 · AnimInstance · 애님 그래프 |
| **Week12** | Particle | 파티클 시스템(이미터/모듈), BVH·KD-Tree 공간 분할 (`JSEngine`) |
| **Week13** | PhysX | NVIDIA PhysX 통합, 천(Cloth) 시뮬레이션, 피직스 애셋 |
| **Week14** | Game Jam #4 | Lua 블루프린트, 디스트리뷰션 커브, 성숙해진 물리 엔진 |

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

## 주차별 하이라이트

**Week1 — Game Jam #1 (2D)**
순수 2D 게임. `UScene`/`USceneManager`로 타이틀·게임 씬을 전환하고, 원·사각형 콜라이더로 충돌을 처리하며
점수 UI와 FMOD 사운드를 붙인 첫 결과물.

**Week2~4 — 3D 엔진의 토대**
Win32 윈도우 + D3D11 디바이스 + ImGui 위에 `UObject` 계층을 세우고(Week2),
텍스처·피킹·멀티 씬·JSON 직렬화를 추가(Week3), OBJ/MTL 로더와 `StaticMesh`,
4분할 뷰포트, 섹션별 머티리얼 슬롯을 구현(Week4).

**Week5~6 — 도구화와 PIE, 데칼**
`Engine / Editor / Client / ObjViewer`로 빌드 타깃을 분리해 "엔진 + 그 위에 만든 도구" 구조를 갖춤.
Play-In-Editor로 편집/플레이 월드를 분리하고, 데칼 렌더링을 추가.

**Week7~8 — 라이팅과 섀도우**
`DirectionalLight`/`AmbientLight` 컴포넌트와 포워드 라이팅, 포그(Week7)에 이어
섀도우 매핑과 큐브맵 섀도우 풀로 그림자를 구현(Week8).

**Week9 — 스크립팅·오디오·시네마틱**
LuaJIT + sol2 기반 스크립팅(코루틴 스케줄러 포함), miniaudio 오디오, 카메라/시네마틱 연출 시스템 도입.

**Week10~11 — 스켈레탈 메시와 애니메이션**
Autodesk FBX SDK로 스켈레탈 메시를 임포트(Week10)하고, `AnimSequence` · `AnimInstance` ·
애님 그래프 · 데이터 모델로 본격적인 애니메이션 시스템을 구축(Week11). 이 시점에 PhysX도 합류.

**Week12 — 파티클과 공간 분할**
이미터/모듈/이벤트로 구성된 파티클 시스템, AABB/OBB/Frustum 등 지오메트리 프리미티브,
BVH·KD-Tree 기반 공간 인덱스, Slate UI 프레임워크를 도입(`JSEngine`).

**Week13~14 — 물리와 비주얼 스크립팅**
NVIDIA PhysX 4.x를 본격 통합하고 NvCloth로 천 시뮬레이션을 구현(Week13).
마지막 주차에는 Lua 블루프린트(비주얼 스크립팅), 프로퍼티 디스트리뷰션 커브,
정돈된 `PhysicsEngine` 서브시스템으로 마무리(Week14).

---

## 기술 스택

| 구분 | 사용 기술 |
|:---|:---|
| 언어 / 표준 | C++20 |
| 플랫폼 | Win32 (Windows) |
| 그래픽스 | DirectX 11, HLSL |
| 그래픽스 헬퍼 | DirectXTK |
| UI / 에디터 | Dear ImGui, (후반) RmlUi · Slate 스타일 위젯 |
| 직렬화 | SimpleJSON (커스텀 JSON) |
| 스크립팅 | LuaJIT + sol2 (Week9~) , Lua 블루프린트 (Week14) |
| 오디오 | FMOD (Week1, Week10~) , miniaudio (Week9) |
| 애셋 임포트 | Autodesk FBX SDK (Week10~) |
| 물리 | NVIDIA PhysX 4.x, NvCloth (Week10/13~) |

---

## 빌드 & 실행

> **요구사항**: Visual Studio 2022 (v143 툴셋), Windows SDK(DirectX 11 포함), x64.
> 대부분의 의존성(ImGui, DirectXTK, SimpleJSON 등)은 프로젝트에 포함되거나 NuGet으로 자동 복원됩니다.

각 주차 폴더로 이동해 빌드합니다. 주차에 따라 빌드 흐름이 조금씩 다릅니다.

**Week3 이후 — 프로젝트 파일 생성 방식**
```bat
:: 1) Visual Studio 솔루션/프로젝트 파일 생성 (포함된 Python 사용)
GenerateProjectFiles.bat

:: 2) 생성된 .sln 을 Visual Studio 2022로 열어 빌드 (x64)
::    또는 패키징 배치 스크립트 사용
ReleaseBuild.bat      :: 릴리스 빌드 + 셰이더/에셋 패키징
GameBuild.bat         :: 게임(클라이언트) 빌드   (후반 주차)
DemoBuild.bat         :: 데모 빌드               (Week7~9)
```

**Week1~2 — 솔루션 직접 빌드**
포함된 `.sln`을 Visual Studio 2022로 열어 바로 빌드합니다.

빌드 산출물(실행 파일·셰이더·에셋)은 주차별 `ReleaseBuild/` 또는 `Bin/<Configuration>/` 등에 생성됩니다.

---

## 저장소 구조

```
GTLEngine/
├── Jungle_Week1/    # Game Jam #1 (2D)
├── Jungle_Week2/    # 3D — Win32 + D3D11 + ImGui 골격
├── Jungle_Week3/    # Texture — 피킹 / 멀티 씬 / JSON
├── Jungle_Week4/    # OBJ — StaticMesh / 4분할 뷰포트
├── Jungle_Week5/    # Game Jam #2 — Engine/Editor/Client/ObjViewer
├── Jungle_Week6/    # Decal      (NipsEngine)
├── Jungle_Week7/    # Light      (KraftonEngine)
├── Jungle_week8/    # Shadow     (KraftonEngine)
├── Jungle_Week9/    # Game Jam #3 + Cinematic (LunaticEngine)
├── Jungle_Week10/   # FBX        (KraftonEngine)
├── Jungle_Week11/   # Animation  (KraftonEngine)
├── Jungle_Week12/   # Particle   (JSEngine)
├── Jungle_Week13/   # PhysX      (KraftonEngine)
├── Jungle_Week14/   # Game Jam #4 (KraftonEngine)
└── README.md
```

각 주차 엔진의 일반적인 소스 레이아웃(후반 주차 기준):

```
<Engine>/Source/
├── Core/            # 엔진 루트, 실행 루프, 플랫폼, 콘솔 변수, 타이머
├── Object/          # UObject, RTTI, FName, ObjectFactory
├── GameFramework/   # AActor, UWorld, 게임 프레임워크
├── Component/       # Scene/Primitive/Camera/Light/StaticMesh/Skeletal …
├── Math/            # FVector, FMatrix, FTransform
├── Render/          # Device(D3D11), Pipeline, Proxy, Culling, Resource
├── Mesh/            # StaticMesh, SkeletalMesh, FbxImporter
├── Animation/       # AnimSequence, AnimInstance, AnimGraph   (Week11~)
├── Particles/       # 파티클 이미터 / 모듈 / 이벤트            (Week12~)
├── Physics/ PhysicsEngine/ Cloth/   # PhysX, 바디/콘스트레인트, 천  (Week13~)
├── Lua/ LuaBlueprint/               # 스크립팅 / 블루프린트       (Week9~)
├── Serialization/   # 씬 저장·복원 (JSON)
├── UI/ Viewport/    # ImGui 에디터 패널, 뷰포트 클라이언트
└── ...
```

---

## 참고

- 정글 과정의 팀 로테이션 때문에 주차별로 팀과 엔진 이름이 다릅니다.
  하나의 코드베이스가 선형으로 이어지는 것이 아니라, **각 주차가 독립적인 결과물**입니다.
- 빌드/사용자 캐시, 에디터 설정(`Editor.ini`, `imgui.ini`), 일부 대용량 바이너리 등은
  `.gitignore`로 제외되어 있을 수 있습니다.
- 주차별 더 자세한 설명은 일부 주차 폴더의 `README.md`(Week2~6)에서 확인할 수 있습니다.
