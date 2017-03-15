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
        _symInitializeW = (SymInitializeW_t)::GetProcAddress(_library, "SymInitializeW");
        _symCleanup = (SymCleanup_t)::GetProcAddress(_library, "SymCleanup");
        _symGetOptions = (SymGetOptions_t)::GetProcAddress(_library, "SymGetOptions");
        _symSetOptions = (SymSetOptions_t)::GetProcAddress(_library, "SymSetOptions");
        _symLoadModuleExW = (SymLoadModuleExW_t)::GetProcAddress(_library, "SymLoadModuleExW");
        _symFromAddrW = (SymFromAddrW_t)::GetProcAddress(_library, "SymFromAddrW");
        _symGetLineFromAddrW64 = (SymGetLineFromAddrW64_t)::GetProcAddress(_library, "SymGetLineFromAddrW64");
        _miniDumpWriteDump = (MiniDumpWriteDump_t)::GetProcAddress(_library, "MiniDumpWriteDump");

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
