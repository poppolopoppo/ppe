#include "stdafx.h"

#include "Core/Core.h"

#ifdef PLATFORM_WINDOWS

#include "DbghelpWrapper.h"
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
    static const FStringView GDbgHelpDllPossiblePaths[] = {
        // Windows 10 SDK ships with a dbghelp.dll which handles /DEBUG:fastlink
#ifdef ARCH_X86
        "C:\\Program Files (x86)\\Windows Kits\\10\\Debuggers\\x86\\dbghelp.dll",
#else
        "C:\\Program Files (x86)\\Windows Kits\\10\\Debuggers\\x64\\dbghelp.dll",
#endif
        // Should fallback to c:\windows\system32\dbghelp.dll
        "dbghelp.dll",
    };
    static const FStringView GDbgCoreDllPossiblePaths[] = {
        // Windows 10 SDK ships with a dbgcore.dll which handles /DEBUG:fastlink
#ifdef ARCH_X86
        "C:\\Program Files (x86)\\Windows Kits\\10\\Debuggers\\x86\\dbgcore.dll",
#else
        "C:\\Program Files (x86)\\Windows Kits\\10\\Debuggers\\x64\\dbgcore.dll",
#endif
        // Should fallback to c:\windows\system32\dbgcore.dll
        "dbgcore.dll",
    };

    for (const FStringView& filename : GDbgHelpDllPossiblePaths) {
        if (_dbghelp_dll.AttachOrLoad(filename))
            break;
    }

    if (_dbghelp_dll) {
        _symInitializeW = (FSymInitializeW)_dbghelp_dll.FunctionAddr("SymInitializeW");
        _symCleanup = (FSymCleanup)_dbghelp_dll.FunctionAddr("SymCleanup");
        _symGetOptions = (FSymGetOptions)_dbghelp_dll.FunctionAddr("SymGetOptions");
        _symSetOptions = (FSymSetOptions)_dbghelp_dll.FunctionAddr("SymSetOptions");
        _symLoadModuleExW = (FSymLoadModuleExW)_dbghelp_dll.FunctionAddr("SymLoadModuleExW");
        _symFromAddrW = (FSymFromAddrW)_dbghelp_dll.FunctionAddr("SymFromAddrW");
        _symGetLineFromAddrW64 = (FSymGetLineFromAddrW64)_dbghelp_dll.FunctionAddr("SymGetLineFromAddrW64");

        Assert(_symInitializeW);
        Assert(_symCleanup);
        Assert(_symGetOptions);
        Assert(_symSetOptions);
        Assert(_symLoadModuleExW);
        Assert(_symFromAddrW);
        Assert(_symGetLineFromAddrW64);

        _miniDumpWriteDump = (FMiniDumpWriteDump)_dbghelp_dll.FunctionAddr("MiniDumpWriteDump");

        if (nullptr == _miniDumpWriteDump) {
            for (const FStringView& filename : GDbgCoreDllPossiblePaths)
                if (_dbgcore_dll.AttachOrLoad(filename))
                    break;

            if (_dbgcore_dll)
                _miniDumpWriteDump = (FMiniDumpWriteDump)_dbgcore_dll.FunctionAddr("MiniDumpWriteDump");
        }

        Assert(_miniDumpWriteDump);

        LOG(Info, L"[DbgHelp] Successfully loaded : callstacks and minidumps are available");
    }
    else {
        LOG(Warning, L"[DbgHelp] Failed to load : callstacks and minidumps won't be available !");
    }
}
//----------------------------------------------------------------------------
FDbghelpWrapper::~FDbghelpWrapper() {
    LOG(Info, L"[DbgHelp] Destroying wrapper");
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#endif //!PLATFORM_WINDOWS


