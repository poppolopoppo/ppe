#include "stdafx.h"

#include "Core/Core.h"

#ifdef PLATFORM_WINDOWS

#include "DbgHelpWrapper.h"
#include "Logger.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDbghelpWrapper::FDbghelpWrapper()
:   _symInitializeW(nullptr)
,   _symCleanup(nullptr)
,   _symSetOptions(nullptr)
,   _symLoadModuleExW(nullptr)
,   _symFromAddrW(nullptr)
,   _symGetLineFromAddrW64(nullptr)
,   _miniDumpWriteDump(nullptr) {

    _library = ::LoadLibraryA("Dbghelp.dll");
    if (_library) {
        _symInitializeW = (FSymInitializeW)::GetProcAddress(_library, "SymInitializeW");
        _symCleanup = (FSymCleanup)::GetProcAddress(_library, "SymCleanup");
        _symGetOptions = (FSymGetOptions)::GetProcAddress(_library, "SymGetOptions");
        _symSetOptions = (FSymSetOptions)::GetProcAddress(_library, "SymSetOptions");
        _symLoadModuleExW = (FSymLoadModuleExW)::GetProcAddress(_library, "SymLoadModuleExW");
        _symFromAddrW = (FSymFromAddrW)::GetProcAddress(_library, "SymFromAddrW");
        _symGetLineFromAddrW64 = (FSymGetLineFromAddrW64)::GetProcAddress(_library, "SymGetLineFromAddrW64");
        _miniDumpWriteDump = (FMiniDumpWriteDump)::GetProcAddress(_library, "MiniDumpWriteDump");

        Assert(_symInitializeW);
        Assert(_symCleanup);
        Assert(_symGetOptions);
        Assert(_symSetOptions);
        Assert(_symLoadModuleExW);
        Assert(_symFromAddrW);
        Assert(_symGetLineFromAddrW64);
        Assert(_miniDumpWriteDump);
    }
    else {
        LOG(Warning, L"[DbgHelp] Failed to 'dbghelp.dll' callstacks and minidumps won't be available !");
    }
}
//----------------------------------------------------------------------------
FDbghelpWrapper::~FDbghelpWrapper() {
    if (_library) {
        ::FreeLibrary(_library);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#endif //!PLATFORM_WINDOWS
