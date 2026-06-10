#include "Math/Rotator.h"

const FRotator FRotator::Zero = {0.f, 0.f, 0.f};

FRotator::FRotator() : Pitch(0.f), Yaw(0.f), Roll(0.f) {}
FRotator::FRotator(float InPitch, float InYaw, float InRoll)
    : Pitch(InPitch), Yaw(InYaw), Roll(InRoll)
{
}
