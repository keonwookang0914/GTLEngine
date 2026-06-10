#pragma once

#include "LogOutputDevice.h"
#include <cstdio>

/**
 * LogOutput Macro
 *
 * Category  (LogEngine, LogRenderer) no enroll
 * Verbosity : Log / Warning / Error
 * fmt, ...  : printf style format string
 *
 * 예시) UE_LOG(LogEngine, Warning, "Value: %d", 42);
 */

#define UE_LOG(Category, Verbosity, fmt, ...)                                                      \
    do                                                                                             \
    {                                                                                              \
        if (GLog)                                                                                  \
        {                                                                                          \
            char _buf[1024];                                                                       \
            snprintf(_buf, sizeof(_buf), "[" #Category "] " fmt, ##__VA_ARGS__);                   \
            GLog->Log(ELogVerbosity::Verbosity, _buf);                                             \
        }                                                                                          \
    } while (0)