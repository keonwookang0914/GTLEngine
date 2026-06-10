#pragma once

/**
 * @brief FRotator
 *
 * Composition order: Yaw -> Pitch -> Roll
 */
struct FRotator
{
    float Pitch;
    float Yaw;
    float Roll;

    static const FRotator Zero;

    FRotator();
    FRotator(float InPitch, float InYaw, float InRoll);
};