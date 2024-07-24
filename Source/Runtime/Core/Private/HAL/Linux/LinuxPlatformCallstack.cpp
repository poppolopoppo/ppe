// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "HAL/Linux/LinuxPlatformCallstack.h"

#ifdef PLATFORM_LINUX

#include "Diagnostic/Logger.h"
#include "IO/Format.h"
#include "IO/StringBuilder.h"
#include "Meta/Utility.h"

#include "HAL/Linux/Errno.h"
#include "HAL/Linux/LinuxPlatformIncludes.h"
#include "HAL/TargetPlatform.h" // HAL log category

// #TODO: need to manually generate a debug symbol file ?
// https://github.com/EpicGames/UnrealEngine/blob/6c20d9831a/Engine/Source/Runtime/Core/Private/Unix/UnixPlatformStackWalk.cpp

namespace PPE {
EXTERN_LOG_CATEGORY(PPE_CORE_API, HAL)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FLinuxPlatformCallstack::InitializeSymbolInfos() {
    // PPE_LOG_UNSUPPORTED_FUNCTION(HAL);
}
//----------------------------------------------------------------------------
void FLinuxPlatformCallstack::LoadAllSymbolInfos() {
    // PPE_LOG_UNSUPPORTED_FUNCTION(HAL);
}
//----------------------------------------------------------------------------
void FLinuxPlatformCallstack::OnLoadModule() {
    // PPE_LOG_UNSUPPORTED_FUNCTION(HAL);
}
//----------------------------------------------------------------------------
size_t FLinuxPlatformCallstack::CaptureCallstack(const TMemoryView<FProgramCounter>& backtrace, size_t framesToSkip) {
    Assert(not backtrace.empty());

    Unused(backtrace);
    Unused(framesToSkip);
    PPE_LOG_UNSUPPORTED_FUNCTION(HAL);

    return 0;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformCallstack::ProgramCounterToModuleName(FWString* moduleName, FProgramCounter pc) {
    Assert(moduleName);
    Assert(pc);

    Unused(moduleName);
    Unused(pc);
    PPE_LOG_UNSUPPORTED_FUNCTION(HAL);

    return false;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformCallstack::ProgramCounterToSymbolInfo(FProgramCounterSymbolInfo* symbolInfo, FProgramCounter pc) {
    Assert(symbolInfo);
    Assert(pc);

    Unused(symbolInfo);
    Unused(pc);
    PPE_LOG_UNSUPPORTED_FUNCTION(HAL);

    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_LINUX
