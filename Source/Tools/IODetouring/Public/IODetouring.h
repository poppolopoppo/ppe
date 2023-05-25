#pragma once

#include "Meta/Aliases.h"

#include "HAL/PlatformIncludes.h"

#ifdef EXPORT_PPE_TOOLS_IODETOURING
#   define PPE_IODETOURING_API DLL_EXPORT
#else
#   define PPE_IODETOURING_API DLL_IMPORT
#endif

// Shared state payload guid.
// {33EF82B6-42EA-4e9c-81AC-DE68C5ED87C6}
constexpr GUID GIODetouringGuid = {
    0x33ef82b6, 0x42ea, 0x4e9c,
    { 0x81, 0xac, 0xde, 0x68, 0xc5, 0xed, 0x87, 0xc6 }
};

// File Access Encoding
constexpr CHAR IODetouring_FileAccessToChar(bool bRead, bool bWrite, bool bExecute) {
    return ('0' + ((bRead?1:0)|(bWrite?2:0)|(bExecute?4:0)));
}
constexpr VOID IODetouring_CharToFileAccess(CHAR nChar, bool* pRead, bool* pWrite, bool* pExecute) {
    nChar -= '0';
    *pRead = !!(nChar&1);
    *pWrite = !!(nChar&2);
    *pExecute = !!(nChar&4);
}

// Named pipe conventions
constexpr PCWSTR GIODetougingGlobalPrefix = TEXT("IOWrapper");