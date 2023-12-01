// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Core.h"

#ifdef PLATFORM_WINDOWS

#include "Diagnostic/Logger.h"

#include "HAL/Windows/DbgHelpWrapper.h"

namespace PPE {
LOG_CATEGORY(, DbgHelp)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDbghelpWrapper::FDbghelpWrapper()
:   _available(false) {
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
        Verify((_api.SymInitializeW = (FSymInitializeW)_dbghelp_dll.FunctionAddr("SymInitializeW")) != nullptr);
        Verify((_api.SymCleanup = (FSymCleanup)_dbghelp_dll.FunctionAddr("SymCleanup")) != nullptr);
        Verify((_api.SymGetOptions = (FSymGetOptions)_dbghelp_dll.FunctionAddr("SymGetOptions")) != nullptr);
        Verify((_api.SymSetOptions = (FSymSetOptions)_dbghelp_dll.FunctionAddr("SymSetOptions")) != nullptr);
        Verify((_api.SymLoadModuleExW = (FSymLoadModuleExW)_dbghelp_dll.FunctionAddr("SymLoadModuleExW")) != nullptr);
        Verify((_api.SymGetModuleInfoW64 = (FSymGetModuleInfoW64)_dbghelp_dll.FunctionAddr("SymGetModuleInfoW64")) != nullptr);
        Verify((_api.SymFromAddrW = (FSymFromAddrW)_dbghelp_dll.FunctionAddr("SymFromAddrW")) != nullptr);
        Verify((_api.SymGetLineFromAddrW64 = (FSymGetLineFromAddrW64)_dbghelp_dll.FunctionAddr("SymGetLineFromAddrW64")) != nullptr);

        _api.MiniDumpWriteDump = (FMiniDumpWriteDump)_dbghelp_dll.FunctionAddr("MiniDumpWriteDump");
        if (nullptr == _api.MiniDumpWriteDump) {
            for (const wchar_t* filename : GDbgCoreDllPossiblePaths)
                if (_dbgcore_dll.AttachOrLoad(filename))
                    break;

            if (_dbgcore_dll)
                _api.MiniDumpWriteDump = (FMiniDumpWriteDump)_dbgcore_dll.FunctionAddr("MiniDumpWriteDump");
        }
        Assert(_api.MiniDumpWriteDump != nullptr);

        PPE_LOG(DbgHelp, Info, "dbghelp successfully loaded : callstacks and minidumps are available");

        _available = true;
    }
    else {
        PPE_LOG(DbgHelp, Warning, "dbghelp failed to load : callstacks and minidumps won't be available !");
    }
}
//----------------------------------------------------------------------------
FDbghelpWrapper::~FDbghelpWrapper() {
    PPE_LOG(DbgHelp, Info, "destroying dbghelp wrapper");
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS


