# jungle-techlab-week2-team3

The simple graphics engine

## Project Structure

```cpp
.
├── Engine
│   ├── Documentation
│   ├── Extras
│   ├── Shaders
│   └── Source
│       ├── Editor
│       ├── Runtime
│       │   ├── ApplicationCore
│       │   ├── Core
│       │   │   ├── Containers
│       │   │   ├── HAL
│       │   │   ├── Logging
│       │   │   ├── Math
│       │   │   ├── Misc
│       │   │   └── Templates
│       │   ├── CoreUObject
│       │   ├── Engine
│       │   │   ├── Camera
│       │   │   ├── Components
│       │   │   ├── Engine
│       │   │   └── GameFramework
│       │   ├── InputCore
│       │   ├── Json
│       │   ├── JsonUtilites
│       │   ├── RHI
│       │   ├── RenderCore
│       │   └── Renderer
│       └── ThirdParty
└── README.md
```

## Convention

### Graphics Convention

- 좌표계: 왼손 좌표계
- 행렬
    - Row-major
    - 곱 순서: 왼쪽에서 우측으로
- 회전 방식: 오일러 각(Pitch, Yaw, Roll)
- 회전 단위: Degree
- 면 노말 벡터 구하는 기준: CCW
- NDC 범위: [-1, -1, 0] ~ [1, 1, 1]
- 오일러 각의 적용 및 연산 순서: Yaw → Pitch → Roll

### Code

- Upper Camel Case (ex. `UserLoginLog`)
- 접두사
    - Genric Class 의 접두사 C
    - `F` : 일반 struct / 값 타입
    - `U` : `UObject` 계열 클래스
    - `A` : `AActor` 계열 클래스
    - `T` : 템플릿 클래스
    - `S` : Slate 위젯
    - `I` : 인터페이스
    - `E` : enum
- 용어
    - Non-Skeleton Mesh - Static Mesh
    - Skeleton Mesh - Skeletal Mesh
    - Effect - Particle
    - 3D Vector - FVector
    - 4D Vector - FVector4
    - 4x4 Matrix - FMatrix
- Object와 Class의 이름은 UE를 따른다.
    - Unreal Engine - Actor, UObject
- 클래스 이름에는 접두사를 붙이지 않는다.
  -  UObject 클래스가 있는 파일의 이름은 "Object.h" 이다.
