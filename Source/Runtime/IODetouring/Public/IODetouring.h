#pragma once

#include "Meta/Aliases.h"

#include "HAL/PlatformIncludes.h"

#ifdef EXPORT_PPE_RUNTIME_IODETOURING
#   define PPE_IODETOURING_API DLL_EXPORT
#else
#   define PPE_IODETOURING_API DLL_IMPORT
#endif

#define TBLOG_PIPE_NAMEA       "\\\\.\\pipe\\IODetouringInjection"
#define TBLOG_PIPE_NAMEW       L"\\\\.\\pipe\\IODetouringInjection"

#ifdef UNICODE
#define TBLOG_PIPE_NAME        TBLOG_PIPE_NAMEW
#else
#define TBLOG_PIPE_NAME        TBLOG_PIPE_NAMEA
#endif

// Shared state payload guid.
// {33EF82B6-42EA-4e9c-81AC-DE68C5ED87C6}
constexpr GUID GIODetouringGuid = {
    0x33ef82b6, 0x42ea, 0x4e9c,
    { 0x81, 0xac, 0xde, 0x68, 0xc5, 0xed, 0x87, 0xc6 }
};
