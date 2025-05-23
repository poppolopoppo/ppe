#pragma once

#include "HAL/TargetPlatform.h"
#include "IO/String.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FGenericProgramCounterSymbol {
    FWString Function;
    FWString Filename;
    size_t Line;
};
//----------------------------------------------------------------------------
struct PPE_CORE_API FGenericPlatformCallstack {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasSymbols, false);
    STATIC_CONST_INTEGRAL(size_t, MaxStackDepth, 0);

    using FProgramCounter = void*;
    using FProgramCounterSymbolInfo = FGenericProgramCounterSymbol;

    static void InitializeSymbolInfos() = delete;
    static void LoadAllSymbolInfos() = delete;

    static void OnLoadModule() = delete; // signaled when a module is loaded to eventually refresh symbol infos

    static size_t CaptureCallstack(const TMemoryView<FProgramCounter>& backtrace, size_t framesToSkip) = delete;

    static bool ProgramCounterToModuleName(FWString* moduleName, FProgramCounter pc) = delete;
    static bool ProgramCounterToSymbolInfo(FProgramCounterSymbolInfo* symbolInfo, FProgramCounter pc) = delete;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
