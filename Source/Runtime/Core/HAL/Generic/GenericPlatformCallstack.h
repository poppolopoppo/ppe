#pragma once

#include "Core/HAL/TargetPlatform.h"
#include "Core/IO/String.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FGenericProgramCounterSymbol {
    FWString Function;
    FWString Filename;
    size_t Line;
};
//----------------------------------------------------------------------------
struct CORE_API FGenericPlatformCallstack {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasSymbols, false);
    STATIC_CONST_INTEGRAL(size_t, MaxStackDepth, 0);

    using FProgramCounter = void*;
    using FProgramCounterSymbolInfo = FGenericProgramCounterSymbol;

    static void OnLoadModule() = delete; // signaled when a module is loaded to eventually refresh symbol infos
    static void LoadSymbolInfos() = delete;

    static size_t CaptureCallstack(const TMemoryView<FProgramCounter>& backtrace, size_t framesToSkip) = delete;

    static bool ProgramCounterToModuleName(FWString* moduleName, FProgramCounter pc) = delete;
    static bool ProgramCounterToSymbolInfo(FProgramCounterSymbolInfo* symbolInfo, FProgramCounter pc) = delete;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
