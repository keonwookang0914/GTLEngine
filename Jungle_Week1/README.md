# Week1 — Game Jam #1 : 피카츄 발리볼

> 크래프톤 정글 게임 테크랩 3기 / Team5 — `KraftonJungle3_Week1_Team5`

14주 자체 엔진 프로젝트의 **첫 게임 잼** 결과물입니다.
`Win32 + DirectX 11`로 직접 만든 2D 게임 프레임워크 위에서 동작하는 **피카츄 발리볼**(2인 대전 발리볼) 게임입니다.

엔진/에디터가 아직 없는 시점이라, 씬·게임 오브젝트·충돌·렌더러를 게임에 필요한 만큼 직접 구현한 것이 특징입니다.

---

## 게임 개요

- **장르**: 2D 실시간 대전 스포츠 (발리볼)
- **플레이어**: 좌/우 두 명(`UPikachu`)이 네트(`UNet`)를 사이에 두고 대결
- **조작**:
  - Player1 — `WASD`
  - Player2 — `방향키`
  - 8방향 스파이크(기본/전방/상단/하단/대각 등)와 다이빙·리커버리 동작
- **규칙**: 랠리로 점수를 내고, 먼저 `MaxPoint(=5)`에 도달하면 승리
- **연출**: 공 궤적/트레일, 펀치 이펙트, 파도(`UWave`)·그림자(`UShadow`) 표현
- **부가**: AI(컴퓨터 플레이어) 모드 지원

---

## 기술 스택

| 구분 | 사용 기술 |
|:---|:---|
| 언어 | C++ |
| 플랫폼 | Win32 (Windows) |
| 그래픽스 | DirectX 11 (직교 투영 기반 2D 렌더링) |
| 셰이더 | HLSL — `ShaderW0.hlsl`, `ShaderTexture.hlsl` |
| 오디오 | FMod (`USoundManager`) |
| 디버그 UI | Dear ImGui |

---

## 아키텍처

- **씬 시스템** — `UScene`(추상) → `UMainTitleScene` / `UMainGameScene`,
  `USceneManager`로 씬 전환. `SceneRegistry` / `SceneAutoRegister`로 씬 자동 등록.
- **게임 오브젝트** — `UGameObject`(Update / Physics_Update) 기반.
  `UPikachu`(플레이어), `UBall`, `UNet`, `UWave`, `UShadow`가 이를 상속.
- **충돌** — `UCollider` 기반의 `UCircleCollider`(공·플레이어 범위), `URectCollider`(경계).
- **게임 흐름** — `UGameManager`(싱글톤)가 게임 상태·점수·라운드 리셋 관리
  (Serving → Playing → PointScored → GameOver).
- **렌더/리소스** — `URenderer`(D3D11 디바이스/컨텍스트), `UShader`, `TextureRenderer`,
  `Animator`(스프라이트 애니메이션), `UResourceManager` / `TextureManager`.
- **진입점** — `UEngine`(싱글톤) · `UGameApp` · `UWindow`.

---

## 빌드 & 실행

- **요구사항**: Visual Studio 2022, DirectX 11 지원 환경
- **방법**: `KraftonJungle3_Week1_Team5.sln`을 열어 빌드/실행
- **구성**: `Debug|Win32`, `Release|Win32`, `Debug|x64`, `Release|x64`

> 이후 주차(Week2~)부터 본격적인 `UObject / Actor / Component` 엔진 구조로 전환됩니다.
