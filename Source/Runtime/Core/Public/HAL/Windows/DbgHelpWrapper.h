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
    friend class Meta::TSingleton<FDbghelpWrapper>;
    using singleton_type = Meta::TSingleton<FDbghelpWrapper>;
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

    struct FAPI {
        FSymInitializeW SymInitializeW;
        FSymCleanup SymCleanup;

        FSymGetOptions SymGetOptions;
        FSymSetOptions SymSetOptions;
        FSymLoadModuleExW SymLoadModuleExW;

        FSymGetModuleInfoW64 SymGetModuleInfoW64;
        FSymFromAddrW SymFromAddrW;
        FSymGetLineFromAddrW64 SymGetLineFromAddrW64;

        FMiniDumpWriteDump MiniDumpWriteDump;
    };

    class FLocked : std::lock_guard<std::mutex> {
    public:
        FLocked() : FLocked(FDbghelpWrapper::Get()) {}
        explicit FLocked(const FDbghelpWrapper& owner)
            : std::lock_guard<std::mutex>(owner._barrier)
            , _owner(owner)
        {}

        bool Available() const { return _owner.Available(); }

        const FAPI* operator ->() const { return (&_owner._api); }

    private:
        const FDbghelpWrapper& _owner;
    };

    ~FDbghelpWrapper();

    bool Available() const { return _available; }

#if USE_PPE_ASSERT
    using singleton_type::HasInstance;
#endif
    using singleton_type::Get;

    static void Create() { singleton_type::Create(); }
    using singleton_type::Destroy;

private:
    FDbghelpWrapper();

    mutable std::mutex _barrier;

    bool _available;

    FDynamicLibrary _dbgcore_dll;
    FDynamicLibrary _dbghelp_dll;

    FAPI _api;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
