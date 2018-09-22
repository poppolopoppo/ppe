#include "stdafx.h"

#include "Core.h"

#ifdef PLATFORM_WINDOWS

#include "Diagnostic/Logger.h"

#include "HAL/Windows/DbghelpWrapper.h"

namespace PPE {
LOG_CATEGORY(, DbgHelp)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDbghelpWrapper::FDbghelpWrapper()
:   _available(false)
,   _symInitializeW(nullptr)
,   _symCleanup(nullptr)
,   _symSetOptions(nullptr)
,   _symLoadModuleExW(nullptr)
,   _symGetModuleInfoW64(nullptr)
,   _symFromAddrW(nullptr)
,   _symGetLineFromAddrW64(nullptr)
,   _miniDumpWriteDump(nullptr) {
    static const wchar_t* GDbgHelpDllPossiblePaths[] = {
        // Windows 10 SDK ships with a dbghelp.dll which handles /DEBUG:fastlink
#ifdef ARCH_X86
        L"C:\\Program Files (x86)\\Windows Kits\\10\\Debuggers\\x86\\dbghelp.dll",
#else
        L"C:\\Program Files (x86)\\Windows Kits\\10\\Debuggers\\x64\\dbghelp.dll",
#endif
        // Should fallback to c:\windows\system32\dbghelp.dll
        L"dbghelp.dll",
    };
    static const wchar_t* GDbgCoreDllPossiblePaths[] = {
        // Windows 10 SDK ships with a dbgcore.dll which handles /DEBUG:fastlink
#ifdef ARCH_X86
        L"C:\\Program Files (x86)\\Windows Kits\\10\\Debuggers\\x86\\dbgcore.dll",
#else
        L"C:\\Program Files (x86)\\Windows Kits\\10\\Debuggers\\x64\\dbgcore.dll",
#endif
        // Should fallback to c:\windows\system32\dbgcore.dll
        L"dbgcore.dll",
    };

    for (const wchar_t* filename : GDbgHelpDllPossiblePaths) {
        if (_dbghelp_dll.AttachOrLoad(filename))
            break;
    }

    if (_dbghelp_dll) {
        _symInitializeW = (FSymInitializeW)_dbghelp_dll.FunctionAddr("SymInitializeW");
        _symCleanup = (FSymCleanup)_dbghelp_dll.FunctionAddr("SymCleanup");
        _symGetOptions = (FSymGetOptions)_dbghelp_dll.FunctionAddr("SymGetOptions");
        _symSetOptions = (FSymSetOptions)_dbghelp_dll.FunctionAddr("SymSetOptions");
        _symLoadModuleExW = (FSymLoadModuleExW)_dbghelp_dll.FunctionAddr("SymLoadModuleExW");
        _symGetModuleInfoW64 = (FSymGetModuleInfoW64)_dbghelp_dll.FunctionAddr("SymGetModuleInfoW64");
        _symFromAddrW = (FSymFromAddrW)_dbghelp_dll.FunctionAddr("SymFromAddrW");
        _symGetLineFromAddrW64 = (FSymGetLineFromAddrW64)_dbghelp_dll.FunctionAddr("SymGetLineFromAddrW64");

        Assert(_symInitializeW);
        Assert(_symCleanup);
        Assert(_symGetOptions);
        Assert(_symSetOptions);
        Assert(_symLoadModuleExW);
        Assert(_symGetModuleInfoW64);
        Assert(_symFromAddrW);
        Assert(_symGetLineFromAddrW64);

        _miniDumpWriteDump = (FMiniDumpWriteDump)_dbghelp_dll.FunctionAddr("MiniDumpWriteDump");

        if (nullptr == _miniDumpWriteDump) {
            for (const wchar_t* filename : GDbgCoreDllPossiblePaths)
                if (_dbgcore_dll.AttachOrLoad(filename))
                    break;

            if (_dbgcore_dll)
                _miniDumpWriteDump = (FMiniDumpWriteDump)_dbgcore_dll.FunctionAddr("MiniDumpWriteDump");
        }

        Assert(_miniDumpWriteDump);

        LOG(DbgHelp, Info, L"dbghelp successfully loaded : callstacks and minidumps are available");

        _available = true;
    }
    else {
        LOG(DbgHelp, Warning, L"dbghelp failed to load : callstacks and minidumps won't be available !");
    }
}
//----------------------------------------------------------------------------
FDbghelpWrapper::~FDbghelpWrapper() {
    LOG(DbgHelp, Info, L"destroying dbghelp wrapper");
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS


