#pragma once

#include "HAL/Platform.h"
#include <cstdint>

enum class EKey
{
    Invalid,

    W,
    A,
    S,
    D,
    Q,
    E,
    F,
    L,
    LeftShift,

    LeftMouseButton,
    RightMouseButton,
    MiddleMouseButton,
    SpaceBar,

    MouseWheelAxis
};

enum class EInputEvent
{
    Pressed,
    Released,
    Repeat,
    DoubleClick,
    Axis
};