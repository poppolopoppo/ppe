#pragma once

#include "Core/Core.h"

#ifndef PLATFORM_WINDOWS
#   error "invalid include !"
#endif

#include "Meta/Singleton.h"

#include <mutex>

#pragma warning(push)
#pragma warning(disable: 4091) // 'typedef '�: ignor� � gauche de '' quand aucune variable n'est d�clar�e


#include <Windows.h>
#include <DbgHelp.h>

#pragma warning(pop)

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDbghelpWrapper : Meta::TSingleton<FDbghelpWrapper> {
public:
    typedef BOOL (WINAPI *SymInitializeW_t)(
        _In_ HANDLE hProcess,
        _In_opt_ PCWSTR UserSearchPath,
        _In_ BOOL fInvadeProcess
        );
    typedef BOOL (WINAPI *SymCleanup_t)(_In_ HANDLE hProcess);

    typedef DWORD (WINAPI *SymGetOptions_t)(VOID);
    typedef DWORD (WINAPI *SymSetOptions_t)(_In_ DWORD SymOptions);

    typedef DWORD64 (WINAPI *SymLoadModuleExW_t)(
        _In_ HANDLE hProcess,
        _In_opt_ HANDLE hFile,
        _In_opt_ PCWSTR ImageName,
        _In_opt_ PCWSTR ModuleName,
        _In_ DWORD64 BaseOfDll,
        _In_ DWORD DllSize,
        _In_opt_ PMODLOAD_DATA Data,
        _In_opt_ DWORD EFlags
        );

    typedef BOOL (WINAPI *SymGetLineFromAddrW64_t)(
        _In_ HANDLE hProcess,
        _In_ DWORD64 dwAddr,
        _Out_ PDWORD pdwDisplacement,
        _Out_ PIMAGEHLP_LINEW64 Line
        );
    typedef BOOL (WINAPI *SymFromAddrW_t)(
        _In_ HANDLE hProcess,
        _In_ DWORD64 FAddress,
        _Out_opt_ PDWORD64 Displacement,
        _Inout_ PSYMBOL_INFOW FSymbol
        );

    typedef BOOL (WINAPI *MiniDumpWriteDump_t)(
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

        SymInitializeW_t SymInitializeW() const { return _owner->_symInitializeW; }
        SymCleanup_t SymCleanup() const { return _owner->_symCleanup; }

        SymGetOptions_t SymGetOptions() const { return _owner->_symGetOptions; }
        SymSetOptions_t SymSetOptions() const { return _owner->_symSetOptions; }

        SymLoadModuleExW_t SymLoadModuleExW() const { return _owner->_symLoadModuleExW; }

        SymFromAddrW_t SymFromAddrW() const { return _owner->_symFromAddrW; }
        SymGetLineFromAddrW64_t SymGetLineFromAddrW64() const { return _owner->_symGetLineFromAddrW64; }

        MiniDumpWriteDump_t MiniDumpWriteDump() const { return _owner->_miniDumpWriteDump; }

    private:
        const FDbghelpWrapper* const _owner;
    };

    ~FDbghelpWrapper();

    bool Available() const { return nullptr != _library; }
    FLocked Lock() const { return FLocked(*this); }

    using Meta::TSingleton<FDbghelpWrapper>::HasInstance;
    using Meta::TSingleton<FDbghelpWrapper>::Instance;

    static void Create() { Meta::TSingleton<FDbghelpWrapper>::Create(); }
    using Meta::TSingleton<FDbghelpWrapper>::Destroy;

private:
    friend Meta::TSingleton<FDbghelpWrapper>;

    FDbghelpWrapper();

    mutable std::mutex _barrier;

    HMODULE _library;

    SymInitializeW_t _symInitializeW;
    SymCleanup_t _symCleanup;

    SymGetOptions_t _symGetOptions;
    SymSetOptions_t _symSetOptions;
    SymLoadModuleExW_t _symLoadModuleExW;

    SymFromAddrW_t _symFromAddrW;
    SymGetLineFromAddrW64_t _symGetLineFromAddrW64;

    MiniDumpWriteDump_t _miniDumpWriteDump;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
