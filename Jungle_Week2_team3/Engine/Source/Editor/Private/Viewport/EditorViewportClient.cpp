#include "Viewport/EditorViewportClient.h"
#include "EngineGlobals.h"
#include "GameFramework/Actor.h"
#include "Logging/LogMacros.h"
#include "Math/MathConstants.h"
#include "Math/Matrix.h"
#include "Math/Ray.h"
#include "Math/Intersection.h"
#include "RendererModule.h"
#include "Runtime/Core/CoreGlobals.h"
#include "Runtime/CoreEngine/Public/Components/PrimitiveComponent.h"
#include "Runtime/CoreEngine/Public/Engine/World.h"
#include "SceneView.h"
#include <algorithm>
#include <cmath>

namespace
{
    constexpr int32  ClickThresholdPixels = 4;
    constexpr uint32 NoHitUUID = 0xFFFFFFFFu;

    float ClampFloat(float InValue, float InMinValue, float InMaxValue)
    {
        return (InValue < InMinValue) ? InMinValue : (InValue > InMaxValue ? InMaxValue : InValue);
    }

    void SetDigitalAxis(float &InOutAxis, EInputEvent Event, float PositiveValue)
    {
        switch (Event)
        {
        case EInputEvent::Pressed:
        case EInputEvent::Repeat:
            InOutAxis = PositiveValue;
            break;

        case EInputEvent::Released:
            if ((PositiveValue > 0.0f && InOutAxis > 0.0f) ||
                (PositiveValue < 0.0f && InOutAxis < 0.0f))
            {
                InOutAxis = 0.0f;
            }
            break;

        default:
            break;
        }
    }

    /**
     * Computes the normalized forward direction from the camera rotation.
     *
     * \param InCameraTransform Camera transform that provides the view rotation.
     * \return Normalized forward vector.
     */
    FVector GetForwardVector(const FViewportCameraTransform &InCameraTransform)
    {
        const FRotator &Rotation = InCameraTransform.GetRotation();
        const float     YawRad = DegreesToRadians(Rotation.Yaw);
        const float     PitchRad = DegreesToRadians(Rotation.Pitch);

        const float SY = std::sin(YawRad);
        const float CY = std::cos(YawRad);
        const float SP = std::sin(PitchRad);
        const float CP = std::cos(PitchRad);

        // Y-Up System: Pitch 0 rotates in X-Z plane, Pitch 90 looks at +Y (UP)
        FVector Forward(CP * SY, SP, CP * CY);
        Forward.Normalize();
        return Forward;
    }

    /**
     * Computes the normalized right direction from the camera orientation.
     *
     * \param InCameraTransform Camera transform used to derive the basis.
     * \return Normalized right vector.
     */
    FVector GetRightVector(const FViewportCameraTransform &InCameraTransform)
    {
        const FVector WorldUp(0.0f, 1.0f, 0.0f);
        const FVector Forward = GetForwardVector(InCameraTransform);

        FVector Right = WorldUp.Cross(Forward);
        if (Right.LengthSquare() <= 0.000001f)
        {
            Right = FVector(1.0f, .0f, 0.0f);
        }

        Right.Normalize();
        return Right;
    }

    /**
     * Computes the normalized up direction from the camera basis.
     *
     * \param InCameraTransform Camera transform used to derive the basis.
     * \return Normalized up vector.
     */
    FVector GetUpVector(const FViewportCameraTransform &InCameraTransform)
    {
        const FVector Forward = GetForwardVector(InCameraTransform);
        const FVector Right = GetRightVector(InCameraTransform);

        FVector Up = Forward.Cross(Right);
        if (Up.LengthSquare() <= 0.000001f)
        {
            Up = FVector(0.0f, 1.0f, 0.0f);
        }

        Up.Normalize();
        return Up;
    }

     /**
     * Computes the direction of the requested gizmo axis, applying local rotation when needed.
     *
     * \param InAxis
     * \param InGizmoState
     * \return
     */
    FVector GetAxisDirection(EGizmoAxis InAxis, const FGizmoState &InGizmoState)
    {
        FVector Dir = FVector::Zero;
        switch (InAxis)
        {
        case EGizmoAxis::X:
            Dir = FVector(1.0f, 0.0f, 0.0f);
            break;
        case EGizmoAxis::Y:
            Dir = FVector(0.0f, 1.0f, 0.0f);
            break;
        case EGizmoAxis::Z:
            Dir = FVector(0.0f, 0.0f, 1.0f);
            break;
        default:
            return FVector::Zero;
        }

        if (InGizmoState.Space == EGizmoSpace::Local)
        {
            const float PitchRad = DegreesToRadians(InGizmoState.Rotation.Pitch);
            const float YawRad = DegreesToRadians(InGizmoState.Rotation.Yaw);
            const float RollRad = DegreesToRadians(InGizmoState.Rotation.Roll);

            const float sp = std::sin(PitchRad);
            const float cp = std::cos(PitchRad);
            const float sy = std::sin(YawRad);
            const float cy = std::cos(YawRad);
            const float sr = std::sin(RollRad);
            const float cr = std::cos(RollRad);

            if (InAxis == EGizmoAxis::X)
            {
                return FVector(cy * cr + sy * sp * sr, -cp * sr, -sy * cr + cy * sp * sr);
            }
            else if (InAxis == EGizmoAxis::Y)
            {
                return FVector(cy * sr - sy * sp * cr, cp * cr, -sy * sr - cy * sp * cr);
            }
            else if (InAxis == EGizmoAxis::Z)
            {
                return FVector(cp * sy, sp, cp * cy);
            }
        }

        return Dir;
    }

    /**
     * Projects a world-space position to screen-space pixel coordinates.
     *
     * Returns false if the view size is invalid or the projected point cannot be
     * converted to normalized device coordinates safely.
     */
    bool ProjectToScreen(const FSceneView &InSceneView, const FVector &InWorld, float &OutX,
                         float &OutY)
    {
        const float ViewWidth = InSceneView.GetCameraWidth();
        const float ViewHeight = InSceneView.GetCameraHeight();
        if (ViewWidth <= 1.0f || ViewHeight <= 1.0f)
        {
            return false;
        }

        const FVector4 Clip = InSceneView.ProjectWorldToClip(InWorld);
        if (std::abs(Clip.W) <= 0.000001f)
        {
            return false;
        }

        const float InvW = 1.0f / Clip.W;
        const float NdcX = Clip.X * InvW;
        const float NdcY = Clip.Y * InvW;

        OutX = (NdcX * 0.5f + 0.5f) * ViewWidth;
        OutY = (1.0f - (NdcY * 0.5f + 0.5f)) * ViewHeight;
        return true;
    }

    /**
     * Estimates a stable world-units-per-pixel value around a world-space origin
     * along the given axis direction.
     *
     * This is useful for keeping gizmo hit bounds or visual sizes consistent in
     * screen space as the camera distance and view angle change.
     */
    float ComputeStableWorldUnitsPerPixel(const FSceneView &InSceneView, const FVector &InOrigin,
                                          const FVector &InAxisDir)
    {
        constexpr float SampleWorldDistance = 100.0f;
        constexpr float MinScreenSamplePixels = 12.0f;
        constexpr float MinWorldUnitsPerPixel = 0.05f;
        constexpr float MaxWorldUnitsPerPixel = 8.0f;

        float OriginScreenX = 0.0f;
        float OriginScreenY = 0.0f;
        float AxisEndScreenX = 0.0f;
        float AxisEndScreenY = 0.0f;

        if (!ProjectToScreen(InSceneView, InOrigin, OriginScreenX, OriginScreenY) ||
            !ProjectToScreen(InSceneView, InOrigin + InAxisDir * SampleWorldDistance,
                             AxisEndScreenX, AxisEndScreenY))
        {
            return 1.0f;
        }

        const float ScreenDx = AxisEndScreenX - OriginScreenX;
        const float ScreenDy = AxisEndScreenY - OriginScreenY;
        const float ScreenLen = std::sqrt(ScreenDx * ScreenDx + ScreenDy * ScreenDy);

        const float SafeScreenLen = (std::max)(ScreenLen, MinScreenSamplePixels);
        const float WorldUnitsPerPixel = SampleWorldDistance / SafeScreenLen;
        return ClampFloat(WorldUnitsPerPixel, MinWorldUnitsPerPixel, MaxWorldUnitsPerPixel);
    }

    float ClampScaleValue(float InScale)
    {
        constexpr float MinScale = 0.01f;
        return (std::max)(InScale, MinScale);
    }
} // namespace

namespace
{
    constexpr uint32 GizmoPickMask = 0x80000000u;
    constexpr uint32 GizmoModeShift = 8u;
    constexpr uint32 GizmoAxisMask = 0xFFu;

    uint32 MakeGizmoPickId(EGizmoMode InMode, EGizmoAxis InAxis)
    {
        return GizmoPickMask | (static_cast<uint32>(InMode) << GizmoModeShift) |
               static_cast<uint32>(InAxis);
    }

    bool IsGizmoPickId(uint32 UUID) { return (UUID & GizmoPickMask) != 0u; }

    EGizmoAxis DecodeGizmoAxis(uint32 UUID)
    {
        return static_cast<EGizmoAxis>(UUID & GizmoAxisMask);
    }
} // namespace

// TODO : refactor (with math.h) and move to utility header
namespace
{
    bool BuildMouseRay(const FSceneView &InSceneView, int32 InMouseX, int32 InMouseY, FRay &OutRay)
    {
        const float ViewWidth = InSceneView.GetCameraWidth();
        const float ViewHeight = InSceneView.GetCameraHeight();
        if (ViewWidth <= 1.0f || ViewHeight <= 1.0f)
        {
            return false;
        }

        const float NdcX = (2.0f * static_cast<float>(InMouseX) / ViewWidth) - 1.0f;
        const float NdcY = 1.0f - (2.0f * static_cast<float>(InMouseY) / ViewHeight);

        const FMatrix ViewProj = InSceneView.GetProjectionMatrix() * InSceneView.GetViewMatrix();
        FMatrix       InvViewProj = ViewProj;
        InvViewProj = InvViewProj.Inverse();

        const FVector4 NearClip(NdcX, NdcY, 0.0f, 1.0f);
        const FVector4 FarClip(NdcX, NdcY, 1.0f, 1.0f);

        const FVector4 NearWorld4 = InvViewProj * NearClip;
        const FVector4 FarWorld4 = InvViewProj * FarClip;

        if (std::abs(NearWorld4.W) <= 0.000001f || std::abs(FarWorld4.W) <= 0.000001f)
        {
            return false;
        }

        const FVector NearWorld = NearWorld4.PerspectiveDivide();
        const FVector FarWorld = FarWorld4.PerspectiveDivide();

        FVector Dir = FarWorld - NearWorld;
        if (Dir.LengthSquare() <= 0.000001f)
        {
            return false;
        }
        Dir.Normalize();

        OutRay.Origin = InSceneView.GetEyePosition();
        OutRay.Direction = Dir;
        return true;
    }

    bool IntersectRayTriangleMT(const FRay &InRay, const FVector &V0, const FVector &V1,
                                const FVector &V2, float &OutT)
    {
        constexpr float Epsilon = 0.000001f;

        const FVector E1 = V1 - V0;
        const FVector E2 = V2 - V0;
        const FVector P = InRay.Direction.Cross(E2);
        const float   Det = E1.Dot(P);

        if (std::abs(Det) < Epsilon)
        {
            return false;
        }

        const float   InvDet = 1.0f / Det;
        const FVector T = InRay.Origin - V0;
        const float   U = T.Dot(P) * InvDet;
        if (U < 0.0f || U > 1.0f)
        {
            return false;
        }

        const FVector Q = T.Cross(E1);
        const float   V = InRay.Direction.Dot(Q) * InvDet;
        if (V < 0.0f || (U + V) > 1.0f)
        {
            return false;
        }

        const float HitT = E2.Dot(Q) * InvDet;
        if (HitT <= Epsilon)
        {
            return false;
        }

        OutT = HitT;
        return true;
    }

    void BuildPerpendicularBasis(const FVector &InDirection, FVector &OutTangent,
                                 FVector &OutBitangent)
    {
        FVector Dir = InDirection;
        Dir.Normalize();

        FVector Up =
            (std::fabs(Dir.Y) < 0.99f) ? FVector(0.0f, 1.0f, 0.0f) : FVector(0.0f, 0.0f, 1.0f);

        OutTangent = Dir.Cross(Up);
        OutTangent.Normalize();

        OutBitangent = Dir.Cross(OutTangent);
        OutBitangent.Normalize();
    }

    void ExpandAABB(const FVector &InPoint, FVector &InOutMin, FVector &InOutMax)
    {
        InOutMin.X = (std::min)(InOutMin.X, InPoint.X);
        InOutMin.Y = (std::min)(InOutMin.Y, InPoint.Y);
        InOutMin.Z = (std::min)(InOutMin.Z, InPoint.Z);

        InOutMax.X = (std::max)(InOutMax.X, InPoint.X);
        InOutMax.Y = (std::max)(InOutMax.Y, InPoint.Y);
        InOutMax.Z = (std::max)(InOutMax.Z, InPoint.Z);
    }


    void TestAxisBox(const FRay &InRay, const FVector &InOrigin, const FVector &InAxisDir,
                     float InLength, float InThickness, float &InOutClosestT, EGizmoAxis InAxis,
                     EGizmoAxis &OutAxis)
    {
        FVector Dir = InAxisDir;
        Dir.Normalize();

        FVector T, B;
        BuildPerpendicularBasis(Dir, T, B);

        const float   Half = InThickness * 0.5f;
        const FVector S = InOrigin;
        const FVector E = InOrigin + Dir * InLength;

        const FVector P0 = S - T * Half - B * Half;
        const FVector P1 = S + T * Half - B * Half;
        const FVector P2 = S + T * Half + B * Half;
        const FVector P3 = S - T * Half + B * Half;
        const FVector P4 = E - T * Half - B * Half;
        const FVector P5 = E + T * Half - B * Half;
        const FVector P6 = E + T * Half + B * Half;
        const FVector P7 = E - T * Half + B * Half;

        FVector BoxMin = P0;
        FVector BoxMax = P0;
        ExpandAABB(P1, BoxMin, BoxMax);
        ExpandAABB(P2, BoxMin, BoxMax);
        ExpandAABB(P3, BoxMin, BoxMax);
        ExpandAABB(P4, BoxMin, BoxMax);
        ExpandAABB(P5, BoxMin, BoxMax);
        ExpandAABB(P6, BoxMin, BoxMax);
        ExpandAABB(P7, BoxMin, BoxMax);

        float BoxHitT = 0.0f;
        if (!IntersectRayAABB(InRay, BoxMin, BoxMax, BoxHitT) || BoxHitT > InOutClosestT)
        {
            return;
        }

        const FVector Tris[12][3] = {{P0, P2, P1}, {P0, P3, P2}, {P4, P5, P6}, {P4, P6, P7},
                                     {P0, P1, P5}, {P0, P5, P4}, {P1, P2, P6}, {P1, P6, P5},
                                     {P2, P3, P7}, {P2, P7, P6}, {P3, P0, P4}, {P3, P4, P7}};

        for (int32 i = 0; i < 12; ++i)
        {
            float HitT = 0.0f;
            if (IntersectRayTriangleMT(InRay, Tris[i][0], Tris[i][1], Tris[i][2], HitT) &&
                HitT < InOutClosestT)
            {
                InOutClosestT = HitT;
                OutAxis = InAxis;
            }
        }
    }

    /**
     * Tests a ray against a gizmo rotation ring approximated by segmented box geometry
     * 
     * \param InRay
     * \param InCenter
     * \param InNormal
     * \param InRadius
     * \param InThickness
     * \param InOutClosestT
     * \param InAxis
     * \param OutAxis
     */
    void TestAxisRing(const FRay &InRay, const FVector &InCenter, const FVector &InNormal,
                      float InRadius, float InThickness, float &InOutClosestT, EGizmoAxis InAxis,
                      EGizmoAxis &OutAxis)
    {
        constexpr int32 Segments = 32;
        const float     Half = InThickness * 0.5f;

        FVector N = InNormal;
        N.Normalize();

        FVector T, B;
        BuildPerpendicularBasis(N, T, B);

        for (int32 i = 0; i < Segments; ++i)
        {
            const float A0 =
                (2.0f * Math::Pi * static_cast<float>(i)) / static_cast<float>(Segments);
            const float A1 =
                (2.0f * Math::Pi * static_cast<float>(i + 1)) / static_cast<float>(Segments);

            const FVector P0 = InCenter + (T * std::cos(A0) + B * std::sin(A0)) * InRadius;
            const FVector P1 = InCenter + (T * std::cos(A1) + B * std::sin(A1)) * InRadius;

            FVector     D = P1 - P0;
            const float Len = D.Length();
            if (Len <= 0.0001f)
            {
                continue;
            }
            D.Normalize();

            FVector TT, BB;
            BuildPerpendicularBasis(D, TT, BB);

            const FVector S0 = P0 - TT * Half - BB * Half;
            const FVector S1 = P0 + TT * Half - BB * Half;
            const FVector S2 = P0 + TT * Half + BB * Half;
            const FVector S3 = P0 - TT * Half + BB * Half;
            const FVector E0 = P1 - TT * Half - BB * Half;
            const FVector E1 = P1 + TT * Half - BB * Half;
            const FVector E2 = P1 + TT * Half + BB * Half;
            const FVector E3 = P1 - TT * Half + BB * Half;

            const FVector Tris[12][3] = {{S0, S2, S1}, {S0, S3, S2}, {E0, E1, E2}, {E0, E2, E3},
                                         {S0, S1, E1}, {S0, E1, E0}, {S1, S2, E2}, {S1, E2, E1},
                                         {S2, S3, E3}, {S2, E3, E2}, {S3, S0, E0}, {S3, E0, E3}};

            for (int32 t = 0; t < 12; ++t)
            {
                float HitT = 0.0f;
                if (IntersectRayTriangleMT(InRay, Tris[t][0], Tris[t][1], Tris[t][2], HitT) &&
                    HitT < InOutClosestT)
                {
                    InOutClosestT = HitT;
                    OutAxis = InAxis;
                }
            }
        }
    }
} // namespace

void FEditorViewportClient::SetCameraLocation(const FVector &InLocation)
{
    CameraTransform.SetLocation(InLocation);
    ResetVelocity();
}

void FEditorViewportClient::SetCameraRotation(const FRotator &InRotation)
{
    FRotator ClampedRotation = InRotation;
    ClampedRotation.Pitch =
        Clamp(ClampedRotation.Pitch, Config.MinPitchDegrees, Config.MaxPitchDegrees);
    ClampedRotation.Roll = 0.0f;
    CameraTransform.SetRotation(InRotation);
    ResetVelocity();
}

void FEditorViewportClient::SetCameraRotation(float InYawDegrees, float InPitchDegrees,
                                              float InRollDegrees)
{
    (void)InRollDegrees; //non-use

    FRotator Rotation;
    Rotation.Yaw = InYawDegrees;
    Rotation.Pitch = Clamp(InPitchDegrees, Config.MinPitchDegrees, Config.MaxPitchDegrees);
    Rotation.Roll = 0.0f;

    CameraTransform.SetRotation(Rotation);
    ResetVelocity();
}

void FEditorViewportClient::GetCameraRotation(float &OutYawDegrees, float &OutPitchDegrees,
                                              float &OutRollDegrees) const
{
    const FRotator &Rotation = CameraTransform.GetRotation();
    OutYawDegrees = Rotation.Yaw;
    OutPitchDegrees = Rotation.Pitch;
    OutRollDegrees = Rotation.Roll;
}

void FEditorViewportClient::SetCameraFov(float InVerticalFovDegrees)
{
    ViewState.VerticalFovDegrees = Clamp(InVerticalFovDegrees, 5.0f, 170.0f);
}

void FEditorViewportClient::SetProjectionMode(EEditorProjectionMode InProjectionMode)
{
    ViewState.ProjectionMode = InProjectionMode;
}

float FEditorViewportClient::GetCameraFov() const { return ViewState.VerticalFovDegrees; }

void FEditorViewportClient::SetSelectedActor(AActor *InActor)
{
    SelectedActor = InActor;
    UpdateGizmoFromSelection();
}

void FEditorViewportClient::UpdateGizmoFromSelection()
{
    if (SelectedActor == nullptr)
    {
        GizmoState.bVisible = false;
        GizmoState.Origin = FVector(0.0f, 0.0f, 0.0f);
        GizmoState.Rotation = FRotator(0.0f, 0.0f, 0.0f);
        return;
    }

    if (World != nullptr)
    {
        bool bFound = false;
        for (AActor *Actor : World->GetActors())
        {
            if (Actor == SelectedActor)
            {
                bFound = true;
                break;
            }
        }

        if (!bFound)
        {
            SelectedActor = nullptr;
            GizmoState.bVisible = false;
            GizmoState.Origin = FVector(0.0f, 0.0f, 0.0f);
            GizmoState.Rotation = FRotator(0.0f, 0.0f, 0.0f);
            return;
        }
    }

    GizmoState.bVisible = bShowGizmo;
    GizmoState.Origin = SelectedActor->GetActorLocation();
    GizmoState.Rotation = SelectedActor->GetActorRotation();
}

bool FEditorViewportClient::InputKey(EKey Key, EInputEvent Event)
{
    switch (Key)
    {
    case EKey::W:
        SetDigitalAxis(Input.MoveForward, Event, +1.0f);
        break;

    case EKey::S:
        SetDigitalAxis(Input.MoveForward, Event, -1.0f);
        break;

    case EKey::D:
        SetDigitalAxis(Input.MoveRight, Event, +1.0f);
        break;

    case EKey::A:
        SetDigitalAxis(Input.MoveRight, Event, -1.0f);
        break;

    case EKey::E:
        SetDigitalAxis(Input.MoveUpWorld, Event, +1.0f);
        break;

    case EKey::Q:
        SetDigitalAxis(Input.MoveUpWorld, Event, -1.0f);
        break;

    case EKey::SpaceBar:
        if (Event == EInputEvent::Pressed)
        {
            // Translate(1), Rotate(2), Scale(3) modular cycle
            const int32 Current = static_cast<int32>(GizmoState.Mode) - 1;
            const int32 Next = (Current + 1) % 3;
            GizmoState.Mode = static_cast<EGizmoMode>(Next + 1);

            EndGizmoDrag();
        }
        break;

    case EKey::L:
        if (Event == EInputEvent::Pressed)
        {
            GizmoState.Space =
                (GizmoState.Space == EGizmoSpace::World) ? EGizmoSpace::Local : EGizmoSpace::World;
            UE_LOG(LogEditor, Log, "Gizmo Space: %s",
                   (GizmoState.Space == EGizmoSpace::World) ? "World" : "Local");
        }
        break;

    case EKey::RightMouseButton:
        if (Event == EInputEvent::Pressed || Event == EInputEvent::DoubleClick)
        {
            bRightMouseDown = true;
        }
        else if (Event == EInputEvent::Released)
        {
            bRightMouseDown = false;
        }
        break;

    case EKey::MiddleMouseButton:
        if (Event == EInputEvent::Pressed || Event == EInputEvent::DoubleClick)
        {
            bMiddleMouseDown = true;
        }
        else if (Event == EInputEvent::Released)
        {
            bMiddleMouseDown = false;
        }
        break;

    case EKey::LeftMouseButton:
        if (Event == EInputEvent::Pressed || Event == EInputEvent::DoubleClick)
        {
            bLeftMouseDown = true;
            MouseDownX = LastMouseX;
            MouseDownY = LastMouseY;

            bPendingLeftPressPick = true;
            if (GRenderer != nullptr)
            {
                GRenderer->RequestPick(SceneView, LastMouseX, LastMouseY, GizmoState);
            }
        }
        else if (Event == EInputEvent::Released)
        {
            bLeftMouseDown = false;
            EndGizmoDrag();
        }
        break;

    case EKey::LeftShift:
        if (Event == EInputEvent::Pressed || Event == EInputEvent::Repeat)
        {
            bScaleUniformModifier = true;
        }
        else if (Event == EInputEvent::Released)
        {
            bScaleUniformModifier = false;
        }
        break;

    default:
        return false;
    }
    return true;
}

bool FEditorViewportClient::InputAxis(EKey Key, float Delta)
{
    if (Key == EKey::MouseWheelAxis)
    {
        Input.Zoom += Delta;
        return true;
    }

    return false;
}

bool FEditorViewportClient::MouseMove(int32 X, int32 Y)
{
    LastMouseX = X;
    LastMouseY = Y;

    if (bLeftMouseDown && bIsDraggingGizmo)
    {
        UpdateGizmoDrag(X, Y);
        return true;
    }

    return bLeftMouseDown || bRightMouseDown || bMiddleMouseDown;
}

bool FEditorViewportClient::CapturedMouseMove(int32 DeltaX, int32 DeltaY)
{
    if (bRightMouseDown)
    {
        Input.ControlMode = EEditorCameraControlMode::Rotate;
        Input.YawDelta += static_cast<float>(DeltaX);
        Input.PitchDelta += static_cast<float>(-DeltaY);
        return true;
    }

    if (bMiddleMouseDown)
    {
        Input.ControlMode = EEditorCameraControlMode::Pan;
        Input.PanX += static_cast<float>(DeltaX);
        Input.PanY += static_cast<float>(DeltaY);
        return true;
    }

    return false;
}

void FEditorViewportClient::ProcessClick(int32 X, int32 Y)
{
    UE_LOG(LogEditor, Log, "Pick Request: Mouse (%d, %d)", X, Y);

    if (GRenderer)
    {
        GRenderer->RequestPick(SceneView, X, Y, GizmoState);
    }
    else
    {
        UE_LOG(LogEditor, Warning, "Pick Request failed: Renderer is null");
    }
}

EPointerReleaseType FEditorViewportClient::GetReleaseType(int32 X, int32 Y) const
{
    const int32 DX = X - MouseDownX;
    const int32 DY = Y - MouseDownY;
    const int32 DistSq = DX * DX + DY * DY;
    const int32 ThresholdSq = ClickThresholdPixels * ClickThresholdPixels;

    return (DistSq <= ThresholdSq) ? EPointerReleaseType::Click : EPointerReleaseType::Drag;
}

void FEditorViewportClient::Tick(float DeltaTime)
{
    UpdateCamera(DeltaTime);
    UpdateGizmoHoverByRaycast();
    if (GRenderer != nullptr)
    {
        uint32 PickedUUID = 0;
        if (GRenderer->TryConsumePickResult(PickedUUID))
        {
            if (PickedUUID == NoHitUUID)
            {
                UE_LOG(LogEditor, Log, "No object picked (Background/Sky)");
                EndGizmoDrag();
                SelectedActor = nullptr;
                SelectedComponent = nullptr;
                GizmoState.ActiveAxis = EGizmoAxis::None;
                GizmoState.HighlightedAxis = EGizmoAxis::None;
            }
            else if (IsGizmoPickId(PickedUUID))
            {
                GizmoState.ActiveAxis = DecodeGizmoAxis(PickedUUID);
                UE_LOG(LogEditor, Log, "Gizmo picked!!!");

                if (bLeftMouseDown)
                {
                    bIsDraggingGizmo = true;
                    DragStartMouseX = LastMouseX;
                    DragStartMouseY = LastMouseY;
                    DragStartActorLocation = (SelectedActor != nullptr)
                                                 ? SelectedActor->GetActorLocation()
                                                 : GizmoState.Origin;
                    DragStartActorRotation = (SelectedActor != nullptr)
                                                 ? SelectedActor->GetActorRotation()
                                                 : GizmoState.Rotation;
                    DragStartActorScale = (SelectedActor != nullptr)
                                              ? SelectedActor->GetActorScale3D()
                                              : FVector(1.0f, 1.0f, 1.0f);
                }
            }
            else
            {
                if (!bLeftMouseDown)
                {
                    GizmoState.ActiveAxis = EGizmoAxis::None;
                    SelectedActor = nullptr;
                    SelectedComponent = nullptr;

                    if (World != nullptr)
                    {
                        SelectedComponent = World->FindComponentByUUID(PickedUUID);
                        if (SelectedComponent != nullptr)
                        {
                            SelectedActor = SelectedComponent->GetOwner();
                            if (SelectedActor != nullptr)
                            {
                                UE_LOG(LogEditor, Log,
                                       "Successfully Picked Actor! [UUID: %u] [Address: %p]",
                                       SelectedActor->UUID, SelectedActor);
                            }
                        }
                        else
                        {
                            UE_LOG(LogEditor, Warning,
                                   "UUID %u received, but component not found in World map",
                                   PickedUUID);
                        }
                    }
                    else
                    {
                        UE_LOG(LogEditor, Log, "No object picked (Background/Sky)");
                        EndGizmoDrag();
                        SelectedActor = nullptr;
                        SelectedComponent = nullptr;
                        GizmoState.ActiveAxis = EGizmoAxis::None;
                        GizmoState.HighlightedAxis = EGizmoAxis::None;
                    }
                }
            }

            bPendingLeftPressPick = false;
        }
    }

    UpdateGizmoFromSelection();

    const FVector &L = CameraTransform.GetLocation();
    Input.ResetFrameInput();
}

void FEditorViewportClient::UpdateGizmoDrag(int32 InMouseX, int32 InMouseY)
{
    if (!bIsDraggingGizmo || SelectedActor == nullptr || SceneView == nullptr)
    {
        return;
    }

    if (GizmoState.Mode == EGizmoMode::Rotate)
    {
        float OriginScreenX = 0.0f;
        float OriginScreenY = 0.0f;
        if (!ProjectToScreen(*SceneView, DragStartActorLocation, OriginScreenX, OriginScreenY))
        {
            return;
        }

        const float DXStart = static_cast<float>(DragStartMouseX) - OriginScreenX;
        const float DYStart = static_cast<float>(DragStartMouseY) - OriginScreenY;
        const float DXCurrent = static_cast<float>(InMouseX) - OriginScreenX;
        const float DYCurrent = static_cast<float>(InMouseY) - OriginScreenY;

        const float StartAngle = std::atan2(DYStart, DXStart);
        const float CurrentAngle = std::atan2(DYCurrent, DXCurrent);
        float       DeltaAngle = CurrentAngle - StartAngle;

        // Wrap angle
        while (DeltaAngle > Math::Pi)
            DeltaAngle -= 2.0f * Math::Pi;
        while (DeltaAngle < -Math::Pi)
            DeltaAngle += 2.0f * Math::Pi;

        float RotationDelta = DeltaAngle;

        // Determine rotation sign based on axis orientation relative to camera
        FVector Normal = GetAxisDirection(GizmoState.ActiveAxis, GizmoState);
        const FVector ViewDir = GetForwardVector(CameraTransform);

        const float   Dot = Normal.Dot(ViewDir);
        if (Dot > 0.0f)
        {
            RotationDelta *= -1.0f;
        }

        FMatrix DeltaRotationMatrix;
        switch (GizmoState.ActiveAxis)
        {
        case EGizmoAxis::X:
            DeltaRotationMatrix = FMatrix::MakeRotationX(RotationDelta);
            break;
        case EGizmoAxis::Y:
            DeltaRotationMatrix = FMatrix::MakeRotationY(RotationDelta);
            break;
        case EGizmoAxis::Z:
            DeltaRotationMatrix = FMatrix::MakeRotationZ(RotationDelta);
            break;
        default:
            DeltaRotationMatrix = FMatrix::Identity;
            break;
        }

        FMatrix CurrentRotationMatrix = FMatrix::MakeFromEuler(DragStartActorRotation);
        FMatrix NewRotationMatrix;

        if (GizmoState.Space == EGizmoSpace::Local)
        {
            // Local rotation: Apply delta after current rotation
            NewRotationMatrix = CurrentRotationMatrix * DeltaRotationMatrix;
        }
        else
        {
            // World rotation: Apply delta before current rotation
            NewRotationMatrix = DeltaRotationMatrix * CurrentRotationMatrix;
        }

        SelectedActor->SetActorRotation(NewRotationMatrix.ToRotator());
        GizmoState.Rotation = SelectedActor->GetActorRotation();
        return;
    }

    if (GizmoState.Mode == EGizmoMode::Scale)
    {
        const FVector AxisDir = GetAxisDirection(GizmoState.ActiveAxis, GizmoState);
        if (AxisDir.LengthSquare() <= 0.0f)
        {
            return;
        }

        float OriginScreenX = 0.0f;
        float OriginScreenY = 0.0f;
        float AxisEndScreenX = 0.0f;
        float AxisEndScreenY = 0.0f;

        if (!ProjectToScreen(*SceneView, DragStartActorLocation, OriginScreenX, OriginScreenY) ||
            !ProjectToScreen(*SceneView, DragStartActorLocation + AxisDir, AxisEndScreenX,
                             AxisEndScreenY))
        {
            return;
        }

        const float ScreenAxisX = AxisEndScreenX - OriginScreenX;
        const float ScreenAxisY = AxisEndScreenY - OriginScreenY;
        const float ScreenAxisLenSq = ScreenAxisX * ScreenAxisX + ScreenAxisY * ScreenAxisY;
        if (ScreenAxisLenSq <= 0.0001f)
        {
            return;
        }

        const float InvScreenAxisLen = 1.0f / std::sqrt(ScreenAxisLenSq);
        const float ScreenAxisNX = ScreenAxisX * InvScreenAxisLen;
        const float ScreenAxisNY = ScreenAxisY * InvScreenAxisLen;

        const float MouseDeltaX = static_cast<float>(InMouseX - DragStartMouseX);
        const float MouseDeltaY = static_cast<float>(InMouseY - DragStartMouseY);
        const float PixelAlongAxis = MouseDeltaX * ScreenAxisNX + MouseDeltaY * ScreenAxisNY;

        const float WorldUnitsPerPixel =
            ComputeStableWorldUnitsPerPixel(*SceneView, DragStartActorLocation, AxisDir);
        const float WorldDistance = PixelAlongAxis * WorldUnitsPerPixel;

        constexpr float ScaleSensitivity = 0.05f;
        const float     ScaleDelta = WorldDistance * ScaleSensitivity;

        FVector NewScale = DragStartActorScale;

        if (bScaleUniformModifier)
        {
            NewScale.X = ClampScaleValue(DragStartActorScale.X + ScaleDelta);
            NewScale.Y = ClampScaleValue(DragStartActorScale.Y + ScaleDelta);
            NewScale.Z = ClampScaleValue(DragStartActorScale.Z + ScaleDelta);
        }
        else
        {
            switch (GizmoState.ActiveAxis)
            {
            case EGizmoAxis::X:
                NewScale.X = ClampScaleValue(DragStartActorScale.X + ScaleDelta);
                break;
            case EGizmoAxis::Y:
                NewScale.Y = ClampScaleValue(DragStartActorScale.Y + ScaleDelta);
                break;
            case EGizmoAxis::Z:
                NewScale.Z = ClampScaleValue(DragStartActorScale.Z + ScaleDelta);
                break;
            default:
                return;
            }
        }

        SelectedActor->SetActorScale3D(NewScale);
        return;
    }

    const FVector AxisDir = GetAxisDirection(GizmoState.ActiveAxis, GizmoState);
    if (AxisDir.LengthSquare() <= 0.0f)
    {
        return;
    }

    float OriginScreenX = 0.0f;
    float OriginScreenY = 0.0f;
    float AxisEndScreenX = 0.0f;
    float AxisEndScreenY = 0.0f;

    if (!ProjectToScreen(*SceneView, DragStartActorLocation, OriginScreenX, OriginScreenY) ||
        !ProjectToScreen(*SceneView, DragStartActorLocation + AxisDir, AxisEndScreenX,
                         AxisEndScreenY))
    {
        return;
    }

    const float ScreenAxisX = AxisEndScreenX - OriginScreenX;
    const float ScreenAxisY = AxisEndScreenY - OriginScreenY;
    const float ScreenAxisLenSq = ScreenAxisX * ScreenAxisX + ScreenAxisY * ScreenAxisY;
    if (ScreenAxisLenSq <= 0.0001f)
    {
        return;
    }

    const float InvScreenAxisLen = 1.0f / std::sqrt(ScreenAxisLenSq);
    const float ScreenAxisNX = ScreenAxisX * InvScreenAxisLen;
    const float ScreenAxisNY = ScreenAxisY * InvScreenAxisLen;

    const float MouseDeltaX = static_cast<float>(InMouseX - DragStartMouseX);
    const float MouseDeltaY = static_cast<float>(InMouseY - DragStartMouseY);
    const float PixelAlongAxis = MouseDeltaX * ScreenAxisNX + MouseDeltaY * ScreenAxisNY;

    const float WorldUnitsPerPixel =
        ComputeStableWorldUnitsPerPixel(*SceneView, DragStartActorLocation, AxisDir);
    const float WorldDistance = PixelAlongAxis * WorldUnitsPerPixel;

    SelectedActor->SetActorLocation(DragStartActorLocation + AxisDir * WorldDistance);
    GizmoState.Origin = SelectedActor->GetActorLocation();
}

void FEditorViewportClient::EndGizmoDrag()
{
    bIsDraggingGizmo = false;
    bPendingLeftPressPick = false;
    GizmoState.ActiveAxis = EGizmoAxis::None;
}

void FEditorViewportClient::UpdateCamera(float InDeltaTime)
{
    UpdateRotation(InDeltaTime);
    UpdateFreeMove(InDeltaTime);
    UpdatePan(InDeltaTime);
    UpdateZoom(InDeltaTime);
}

void FEditorViewportClient::ResetVelocity()
{
    LinearVelocity = FVector(0.0f, 0.0f, 0.0f);
    FovVelocity = 0.0f;
}

void FEditorViewportClient::UpdateRotation(float InDeltaTime)
{
    (void)InDeltaTime;

    if (Input.ControlMode != EEditorCameraControlMode::Rotate)
    {
        return;
    }

    FRotator Rotation = CameraTransform.GetRotation();

    Rotation.Yaw += Input.YawDelta * Config.RotationSpeed;
    Rotation.Pitch += Input.PitchDelta * Config.RotationSpeed;
    Rotation.Pitch = Clamp(Rotation.Pitch, Config.MinPitchDegrees, Config.MaxPitchDegrees);
    Rotation.Roll = 0.0f;

    CameraTransform.SetRotation(Rotation);
}

void FEditorViewportClient::UpdateFreeMove(float InDeltaTime)
{
    const FVector Forward = GetForwardVector(CameraTransform);
    const FVector Right = GetRightVector(CameraTransform);
    const FVector UpLocal = GetUpVector(CameraTransform);
    const FVector UpWorld(0.0f, 1.0f, 0.0f);

    FVector Acceleration(0.0f, 0.0f, 0.0f);
    Acceleration += Forward * Input.MoveForward;
    Acceleration += Right * Input.MoveRight;
    Acceleration += UpLocal * Input.MoveUpLocal;
    Acceleration += UpWorld * Input.MoveUpWorld;

    const FVector DeltaVelocity = Acceleration * Config.TranslationAcceleration * InDeltaTime;
    LinearVelocity += DeltaVelocity;

    const float DampingScale = (std::max)(0.0f, 1.0f - (Config.TranslationDamping * InDeltaTime));
    LinearVelocity *= DampingScale;

    FVector       Location = CameraTransform.GetLocation();
    const FVector DeltaLocation = LinearVelocity * InDeltaTime;
    Location += DeltaLocation;
    CameraTransform.SetLocation(Location);
}

void FEditorViewportClient::UpdatePan(float InDeltaTime)
{
    (void)InDeltaTime;

    if (Input.ControlMode != EEditorCameraControlMode::Pan)
    {
        return;
    }

    const float   PanScale = Config.PanSpeed;
    const FVector RightOffset = GetRightVector(CameraTransform) * (-Input.PanX * PanScale);
    const FVector UpOffset = GetUpVector(CameraTransform) * (Input.PanY * PanScale);

    FVector Location = CameraTransform.GetLocation();
    Location += RightOffset + UpOffset;
    CameraTransform.SetLocation(Location);
}

void FEditorViewportClient::UpdateZoom(float InDeltaTime)
{
    if (std::abs(Input.Zoom) <= 0.001f)
    {
        return;
    }

    if (ViewState.ProjectionMode == EEditorProjectionMode::Perspective)
    {
        const float ZoomDistance =
            Input.Zoom * Config.ZoomSpeed * Config.ZoomTranslationScale * InDeltaTime;

        FVector Location = CameraTransform.GetLocation();
        Location += GetForwardVector(CameraTransform) * ZoomDistance;
        CameraTransform.SetLocation(Location);
    }
    else
    {
        ViewState.OrthoWidth = ClampFloat(
            ViewState.OrthoWidth - (Input.Zoom * Config.ZoomSpeed * InDeltaTime), 1.0f, 1000000.0f);
    }
}

float FEditorViewportClient::Clamp(float InValue, float InMinValue, float InMaxValue)
{
    return ClampFloat(InValue, InMinValue, InMaxValue);
}

FSceneView FEditorViewportClient::BuildSceneView(const FViewportCameraTransform &InCameraTransform,
                                                 const FEditorViewportViewState &InViewState,
                                                 float InViewportWidth, float InViewportHeight)
{
    FSceneView Result;

    Result.SetCameraSize(InViewportWidth, InViewportHeight);

    const float SafeWidth = (InViewportWidth > 0.0f) ? InViewportWidth : 1.0f;
    const float SafeHeight = (InViewportHeight > 0.0f) ? InViewportHeight : 1.0f;
    const float AspectRatio = SafeWidth / SafeHeight;

    const FVector Eye = InCameraTransform.GetLocation();
    const FVector Forward = GetForwardVector(InCameraTransform);
    const FVector Right = GetRightVector(InCameraTransform);
    const FVector Up = GetUpVector(InCameraTransform);

    Result.SetEyePosition(Eye);

    const FMatrix ViewMatrix(Right.X, Right.Y, Right.Z, -Right.Dot(Eye), Up.X, Up.Y, Up.Z,
                             -Up.Dot(Eye), Forward.X, Forward.Y, Forward.Z, -Forward.Dot(Eye), 0.0f,
                             0.0f, 0.0f, 1.0f);

    Result.SetViewMatrix(ViewMatrix);

    if (InViewState.ProjectionMode == EEditorProjectionMode::Perspective)
    {
        const float HalfFovRad = DegreesToRadians(InViewState.VerticalFovDegrees) * 0.5f;
        const float TanHalfFov = std::tan(HalfFovRad);

        const float m00 = 1.0f / (AspectRatio * TanHalfFov);
        const float m11 = 1.0f / TanHalfFov;
        const float m22 = InViewState.FarPlane / (InViewState.FarPlane - InViewState.NearPlane);
        const float m23 = -(InViewState.FarPlane * InViewState.NearPlane) /
                          (InViewState.FarPlane - InViewState.NearPlane);
        const float m32 = 1.0f;

        const FMatrix ProjectionMatrix(m00, 0.0f, 0.0f, 0.0f, 0.0f, m11, 0.0f, 0.0f, 0.0f, 0.0f,
                                       m22, m23, 0.0f, 0.0f, m32, 0.0f);

        Result.SetProjectionMatrix(ProjectionMatrix);
    }
    else
    {
        const float OrthoHeight = InViewState.OrthoWidth / AspectRatio;
        const float m00 = 2.0f / InViewState.OrthoWidth;
        const float m11 = 2.0f / OrthoHeight;
        const float m22 = 1.0f / (InViewState.FarPlane - InViewState.NearPlane);
        const float m23 = -InViewState.NearPlane / (InViewState.FarPlane - InViewState.NearPlane);

        const FMatrix ProjectionMatrix(m00, 0.0f, 0.0f, 0.0f, 0.0f, m11, 0.0f, 0.0f, 0.0f, 0.0f,
                                       m22, m23, 0.0f, 0.0f, 0.0f, 1.0f);

        Result.SetProjectionMatrix(ProjectionMatrix);
    }

    return Result;
}

void FEditorViewportClient::Draw(float Width, float Height)
{
    if (SceneView == nullptr || GRenderer == nullptr)
    {
        return;
    }

    *SceneView = BuildSceneView(CameraTransform, ViewState, Width, Height);
    GRenderer->RenderFrame(SceneView, GizmoState);
}

void FEditorViewportClient::ResetInputState()
{
    Input = FEditorCameraInput();
    ResetVelocity();

    bLeftMouseDown = false;
    bRightMouseDown = false;
    bMiddleMouseDown = false;
    bScaleUniformModifier = false;
    EndGizmoDrag();
}

/**
 * Updates the hovered gizmo axis by raycasting from the current mouse position.
 * 
 */
void FEditorViewportClient::UpdateGizmoHoverByRaycast()
{
    if (SceneView == nullptr || !GizmoState.bVisible || SelectedActor == nullptr)
    {
        GizmoState.HighlightedAxis = EGizmoAxis::None;
        return;
    }

    if (bIsDraggingGizmo)
    {
        GizmoState.HighlightedAxis = GizmoState.ActiveAxis;
        return;
    }
    FRay Ray;
    if (!BuildMouseRay(*SceneView, LastMouseX, LastMouseY, Ray))
    {
        GizmoState.HighlightedAxis = EGizmoAxis::None;
        return;
    }

    const FVector Origin = GizmoState.Origin;

    const FVector XAxis = GetAxisDirection(EGizmoAxis::X, GizmoState);
    const FVector YAxis = GetAxisDirection(EGizmoAxis::Y, GizmoState);
    const FVector ZAxis = GetAxisDirection(EGizmoAxis::Z, GizmoState);

    float      ClosestT = 3.4e38f;
    EGizmoAxis HitAxis = EGizmoAxis::None;

    if (GizmoState.Mode == EGizmoMode::Rotate)
    {
        constexpr float Radius = 72.0f;
        constexpr float Thickness = 12.0f;

        TestAxisRing(Ray, Origin, XAxis, Radius, Thickness, ClosestT, EGizmoAxis::X, HitAxis);
        TestAxisRing(Ray, Origin, YAxis, Radius, Thickness, ClosestT, EGizmoAxis::Y, HitAxis);
        TestAxisRing(Ray, Origin, ZAxis, Radius, Thickness, ClosestT, EGizmoAxis::Z, HitAxis);
    }
    else
    {
        constexpr float Length = 120.0f;
        constexpr float Thickness = 12.0f;

        TestAxisBox(Ray, Origin, XAxis, Length, Thickness, ClosestT, EGizmoAxis::X, HitAxis);
        TestAxisBox(Ray, Origin, YAxis, Length, Thickness, ClosestT, EGizmoAxis::Y, HitAxis);
        TestAxisBox(Ray, Origin, ZAxis, Length, Thickness, ClosestT, EGizmoAxis::Z, HitAxis);
    }

    GizmoState.HighlightedAxis = HitAxis;
}