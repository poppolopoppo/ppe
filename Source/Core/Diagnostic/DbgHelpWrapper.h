#pragma once

#include "Core/Core.h"

#ifndef PLATFORM_WINDOWS
#   error "invalid include !"
#endif

#include "Meta/Singleton.h"

#include <mutex>

#pragma warning(push)
#pragma warning(disable: 4091) // 'typedef '�: ignor� � gauche de '' quand aucune variable n'est d�clar�e

#include "Misc/DLLWrapper.h"
#include "Misc/Platform_Windows.h"
#include <DbgHelp.h>

#pragma warning(pop)

namespace Core {
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

    class FLocked : std::unique_lock<std::mutex> {
    public:
        explicit FLocked(const FDbghelpWrapper& owner)
            : std::unique_lock<std::mutex>(owner._barrier)
            , _owner(&owner) {
            Assert(_owner->Available());
        }

        FSymInitializeW SymInitializeW() const { return _owner->_symInitializeW; }
        FSymCleanup SymCleanup() const { return _owner->_symCleanup; }

        FSymGetOptions SymGetOptions() const { return _owner->_symGetOptions; }
        FSymSetOptions SymSetOptions() const { return _owner->_symSetOptions; }

        FSymLoadModuleExW SymLoadModuleExW() const { return _owner->_symLoadModuleExW; }

        FSymFromAddrW SymFromAddrW() const { return _owner->_symFromAddrW; }
        FSymGetLineFromAddrW64 SymGetLineFromAddrW64() const { return _owner->_symGetLineFromAddrW64; }

        FMiniDumpWriteDump MiniDumpWriteDump() const { return _owner->_miniDumpWriteDump; }

    private:
        const FDbghelpWrapper* const _owner;
    };

    ~FDbghelpWrapper();

    bool Available() const { return (_dbghelp_dll.IsValid()); }
    FLocked Lock() const { return FLocked(*this); }

#ifdef WITH_CORE_ASSERT
    using Meta::TSingleton<FDbghelpWrapper>::HasInstance;
#endif
    using Meta::TSingleton<FDbghelpWrapper>::Instance;

    static void Create() { Meta::TSingleton<FDbghelpWrapper>::Create(); }
    using Meta::TSingleton<FDbghelpWrapper>::Destroy;

private:
    friend Meta::TSingleton<FDbghelpWrapper>;

    FDbghelpWrapper();

    mutable std::mutex _barrier;

    FDLLWrapper _dbgcore_dll;
    FDLLWrapper _dbghelp_dll;

    FSymInitializeW _symInitializeW;
    FSymCleanup _symCleanup;

    FSymGetOptions _symGetOptions;
    FSymSetOptions _symSetOptions;
    FSymLoadModuleExW _symLoadModuleExW;

    FSymFromAddrW _symFromAddrW;
    FSymGetLineFromAddrW64 _symGetLineFromAddrW64;

    FMiniDumpWriteDump _miniDumpWriteDump;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
