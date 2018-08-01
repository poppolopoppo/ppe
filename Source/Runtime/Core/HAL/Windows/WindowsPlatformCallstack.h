#pragma once

#include "Core/HAL/Generic/GenericPlatformCallstack.h"

#ifdef PLATFORM_WINDOWS

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct CORE_API FWindowsPlatformCallstack : FGenericPlatformCallstack {
public:
    STATIC_CONST_INTEGRAL(bool, HasSymbols, true);
    STATIC_CONST_INTEGRAL(size_t, MaxStackDepth, 62);

    using FGenericPlatformCallstack::FProgramCounter;
    using FGenericPlatformCallstack::FProgramCounterSymbolInfo;

    static void OnLoadModule();
    static void LoadSymbolInfos();

    static size_t CaptureCallstack(const TMemoryView<FProgramCounter>& backtrace, size_t framesToSkip);

    static bool ProgramCounterToModuleName(FWString* moduleName, FProgramCounter pc);
    static bool ProgramCounterToSymbolInfo(FProgramCounterSymbolInfo* symbolInfo, FProgramCounter pc);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#endif //!PLATFORM_WINDOWS
