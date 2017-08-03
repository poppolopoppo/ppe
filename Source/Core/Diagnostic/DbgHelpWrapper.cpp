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
	_dbghelp_dll = _dbgcore_dll = nullptr;

    static const wchar_t* const GDbgHelpDllPossiblePaths[] = {
        // Windows 10 SDK ships with a dbghelp.dll which handles /DEBUG:fastlink
#ifdef ARCH_X86
        L"C:\\Program Files (x86)\\Windows Kits\\10\\Debuggers\\x86\\dbghelp.dll",
#else
        L"C:\\Program Files (x86)\\Windows Kits\\10\\Debuggers\\x64\\dbghelp.dll",
#endif
        // Should fallback to c:\windows\system32\dbghelp.dll
        L"dbghelp.dll",
    };
	static const wchar_t* const GDbgCoreDllPossiblePaths[] = {
		// Windows 10 SDK ships with a dbgcore.dll which handles /DEBUG:fastlink
#ifdef ARCH_X86
		L"C:\\Program Files (x86)\\Windows Kits\\10\\Debuggers\\x86\\dbgcore.dll",
#else
		L"C:\\Program Files (x86)\\Windows Kits\\10\\Debuggers\\x64\\dbgcore.dll",
#endif
		// Should fallback to c:\windows\system32\dbgcore.dll
		L"dbgcore.dll",
	};

	const DWORD libraryFlags = (LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    const wchar_t* ModuleFilename = nullptr;
    for (const wchar_t* filename : GDbgHelpDllPossiblePaths) {
        ModuleFilename = filename;
        if (_dbghelp_dll = ::LoadLibraryExW(filename, nullptr, libraryFlags))
            break;
    }

    if (_dbghelp_dll) {
        _symInitializeW = (FSymInitializeW)::GetProcAddress(_dbghelp_dll, "SymInitializeW");
        _symCleanup = (FSymCleanup)::GetProcAddress(_dbghelp_dll, "SymCleanup");
        _symGetOptions = (FSymGetOptions)::GetProcAddress(_dbghelp_dll, "SymGetOptions");
        _symSetOptions = (FSymSetOptions)::GetProcAddress(_dbghelp_dll, "SymSetOptions");
        _symLoadModuleExW = (FSymLoadModuleExW)::GetProcAddress(_dbghelp_dll, "SymLoadModuleExW");
        _symFromAddrW = (FSymFromAddrW)::GetProcAddress(_dbghelp_dll, "SymFromAddrW");
        _symGetLineFromAddrW64 = (FSymGetLineFromAddrW64)::GetProcAddress(_dbghelp_dll, "SymGetLineFromAddrW64");
        
        Assert(_symInitializeW);
        Assert(_symCleanup);
        Assert(_symGetOptions);
        Assert(_symSetOptions);
        Assert(_symLoadModuleExW);
        Assert(_symFromAddrW);
        Assert(_symGetLineFromAddrW64);

		_miniDumpWriteDump = (FMiniDumpWriteDump)::GetProcAddress(_dbghelp_dll, "MiniDumpWriteDump");

		if (nullptr == _miniDumpWriteDump) {
			for (const wchar_t* filename : GDbgCoreDllPossiblePaths)
				if (_dbgcore_dll = ::LoadLibraryExW(filename, nullptr, libraryFlags))
					break;

			if (_dbgcore_dll)
				_miniDumpWriteDump = (FMiniDumpWriteDump)::GetProcAddress(_dbgcore_dll, "MiniDumpWriteDump");
		}

        Assert(_miniDumpWriteDump);

#ifdef USE_DEBUG_LOGGER
        wchar_t ModuleFilenameBuffer[1024];
        if (::GetModuleFileName(_dbghelp_dll, ModuleFilenameBuffer, lengthof(ModuleFilenameBuffer)))
            ModuleFilename = ModuleFilenameBuffer;

        LOG(Info, L"[DbgHelp] Successfully loaded '{0}' : callstacks and minidumps are available", ModuleFilename);
#endif
    }
    else {
        LOG(Warning, L"[DbgHelp] Failed to load '{0}' : callstacks and minidumps won't be available !", ModuleFilename);
    }
}
//----------------------------------------------------------------------------
FDbghelpWrapper::~FDbghelpWrapper() {
    if (_dbghelp_dll)
        ::FreeLibrary(_dbghelp_dll);
	if (_dbgcore_dll)
		::FreeLibrary(_dbgcore_dll);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#endif //!PLATFORM_WINDOWS


