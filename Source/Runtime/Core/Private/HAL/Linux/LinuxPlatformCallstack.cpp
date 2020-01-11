#include "stdafx.h"

#include "HAL/Linux/LinuxPlatformCallstack.h"

#ifdef PLATFORM_LINUX

#include "Diagnostic/Logger.h"
#include "IO/Format.h"
#include "IO/StringBuilder.h"
#include "Meta/Utility.h"

#include "HAL/Linux/Errno.h"
#include "HAL/Linux/LinuxPlatformIncludes.h"

// #TODO: need to manually generate a debug symbol file ?
// https://github.com/EpicGames/UnrealEngine/blob/6c20d9831a/Engine/Source/Runtime/Core/Private/Unix/UnixPlatformStackWalk.cpp

namespace PPE {
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

}
//----------------------------------------------------------------------------
void FLinuxPlatformCallstack::LoadAllSymbolInfos() {

}
//----------------------------------------------------------------------------
void FLinuxPlatformCallstack::OnLoadModule() {

}
//----------------------------------------------------------------------------
size_t FLinuxPlatformCallstack::CaptureCallstack(const TMemoryView<FProgramCounter>& backtrace, size_t framesToSkip) {
    Assert(not backtrace.empty());

    UNUSED(backtrace);
    UNUSED(framesToSkip);
    AssertNotImplemented(); // #TODO

    return 0;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformCallstack::ProgramCounterToModuleName(FWString* moduleName, FProgramCounter pc) {
    Assert(moduleName);
    Assert(pc);

    UNUSED(moduleName);
    UNUSED(pc);
    AssertNotImplemented(); // #TODO

    return false;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformCallstack::ProgramCounterToSymbolInfo(FProgramCounterSymbolInfo* symbolInfo, FProgramCounter pc) {
    Assert(symbolInfo);
    Assert(pc);

    UNUSED(symbolInfo);
    UNUSED(pc);
    AssertNotImplemented(); // #TODO

    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_LINUX
