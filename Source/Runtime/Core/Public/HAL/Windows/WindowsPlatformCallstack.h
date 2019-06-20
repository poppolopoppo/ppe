#pragma once

#include "HAL/Generic/GenericPlatformCallstack.h"

#ifdef PLATFORM_WINDOWS

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FWindowsPlatformCallstack : FGenericPlatformCallstack {
public:
    STATIC_CONST_INTEGRAL(bool, HasSymbols, true);
    STATIC_CONST_INTEGRAL(size_t, MaxStackDepth, 62);

    using FGenericPlatformCallstack::FProgramCounter;
    using FGenericPlatformCallstack::FProgramCounterSymbolInfo;

    static void InitializeSymbolInfos();
    static void LoadAllSymbolInfos();

    static void OnLoadModule();

    static size_t CaptureCallstack(const TMemoryView<FProgramCounter>& backtrace, size_t framesToSkip);

    static bool ProgramCounterToModuleName(FWString* moduleName, FProgramCounter pc);
    static bool ProgramCounterToSymbolInfo(FProgramCounterSymbolInfo* symbolInfo, FProgramCounter pc);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
