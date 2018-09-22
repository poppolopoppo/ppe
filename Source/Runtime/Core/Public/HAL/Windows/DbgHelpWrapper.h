#pragma once

#include "Core.h"

#ifndef PLATFORM_WINDOWS
#   error "invalid include !"
#endif

#include "HAL/Windows/WindowsPlatformIncludes.h"

#include "Meta/Singleton.h"

#include <mutex>

PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4091) // 'typedef : ignored left of '' when not variable is declared

#include "Misc/DynamicLibrary.h"

#include <DbgHelp.h>

PRAGMA_MSVC_WARNING_POP()

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDbghelpWrapper : Meta::TSingleton<FDbghelpWrapper> {
public:
    typedef BOOL (WINAPI *FSymInitializeW)(
        _In_ HANDLE hProcess,
        _In_opt_ PCWSTR UserSearchPath,
        _In_ BOOL fInvadeProcess
        );
    typedef BOOL (WINAPI *FSymCleanup)(_In_ HANDLE hProcess);

    typedef DWORD (WINAPI *FSymGetOptions)(VOID);
    typedef DWORD (WINAPI *FSymSetOptions)(_In_ DWORD SymOptions);

    typedef DWORD64 (WINAPI *FSymLoadModuleExW)(
        _In_ HANDLE hProcess,
        _In_opt_ HANDLE hFile,
        _In_opt_ PCWSTR ImageName,
        _In_opt_ PCWSTR ModuleName,
        _In_ DWORD64 BaseOfDll,
        _In_ DWORD DllSize,
        _In_opt_ PMODLOAD_DATA Data,
        _In_opt_ DWORD EFlags
        );

    typedef BOOL (WINAPI *FSymGetModuleInfoW64)(
        _In_  HANDLE hProcess,
        _In_  DWORD64 dwAddr,
        _Out_ PIMAGEHLP_MODULEW64 ModuleInfo
    );
    typedef BOOL (WINAPI *FSymGetLineFromAddrW64)(
        _In_ HANDLE hProcess,
        _In_ DWORD64 dwAddr,
        _Out_ PDWORD pdwDisplacement,
        _Out_ PIMAGEHLP_LINEW64 Line
        );
    typedef BOOL (WINAPI *FSymFromAddrW)(
        _In_ HANDLE hProcess,
        _In_ DWORD64 FAddress,
        _Out_opt_ PDWORD64 Displacement,
        _Inout_ PSYMBOL_INFOW FSymbol
        );

    typedef BOOL (WINAPI *FMiniDumpWriteDump)(
        HANDLE hProcess,
        DWORD ProcessId,
        HANDLE hFile,
        MINIDUMP_TYPE DumpType,
        PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
        PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
        PMINIDUMP_CALLBACK_INFORMATION CallbackParam
        );

    class FLocked : std::lock_guard<std::mutex> {
    public:
        FLocked() : FLocked(FDbghelpWrapper::Get()) {}
        explicit FLocked(const FDbghelpWrapper& owner)
            : std::lock_guard<std::mutex>(owner._barrier)
            , _owner(&owner)
        {}

        bool Available() const { return _owner->Available(); }

        FSymInitializeW SymInitializeW() const { return _owner->_symInitializeW; }
        FSymCleanup SymCleanup() const { return _owner->_symCleanup; }

        FSymGetOptions SymGetOptions() const { return _owner->_symGetOptions; }
        FSymSetOptions SymSetOptions() const { return _owner->_symSetOptions; }

        FSymLoadModuleExW SymLoadModuleExW() const { return _owner->_symLoadModuleExW; }

        FSymGetModuleInfoW64 SymGetModuleInfoW64() const { return _owner->_symGetModuleInfoW64; }
        FSymFromAddrW SymFromAddrW() const { return _owner->_symFromAddrW; }
        FSymGetLineFromAddrW64 SymGetLineFromAddrW64() const { return _owner->_symGetLineFromAddrW64; }

        FMiniDumpWriteDump MiniDumpWriteDump() const { return _owner->_miniDumpWriteDump; }

    private:
        const FDbghelpWrapper* const _owner;
    };

    ~FDbghelpWrapper();

    bool Available() const { return _available; }

#ifdef WITH_PPE_ASSERT
    using Meta::TSingleton<FDbghelpWrapper>::HasInstance;
#endif
    using Meta::TSingleton<FDbghelpWrapper>::Get;

    static void Create() { Meta::TSingleton<FDbghelpWrapper>::Create(); }
    using Meta::TSingleton<FDbghelpWrapper>::Destroy;

private:
    friend Meta::TSingleton<FDbghelpWrapper>;

    FDbghelpWrapper();

    mutable std::mutex _barrier;

    bool _available;

    FDynamicLibrary _dbgcore_dll;
    FDynamicLibrary _dbghelp_dll;

    FSymInitializeW _symInitializeW;
    FSymCleanup _symCleanup;

    FSymGetOptions _symGetOptions;
    FSymSetOptions _symSetOptions;
    FSymLoadModuleExW _symLoadModuleExW;

    FSymGetModuleInfoW64 _symGetModuleInfoW64;
    FSymFromAddrW _symFromAddrW;
    FSymGetLineFromAddrW64 _symGetLineFromAddrW64;

    FMiniDumpWriteDump _miniDumpWriteDump;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
