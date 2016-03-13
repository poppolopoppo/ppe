#pragma once

#include "Core/Core.h"

#ifndef OS_WINDOWS
#   error "invalid include !"
#endif

#include "Meta/Singleton.h"

#include <mutex>

#include <DbgHelp.h>
#include <Windows.h>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DbghelpWrapper : Meta::Singleton<DbghelpWrapper> {
public:
    typedef BOOL (*SymInitializeW_t)(
        _In_ HANDLE hProcess,
        _In_opt_ PCWSTR UserSearchPath,
        _In_ BOOL fInvadeProcess
        );
    typedef BOOL (*SymCleanup_t)(_In_ HANDLE hProcess);

    typedef DWORD (*SymGetOptions_t)(VOID);
    typedef DWORD (*SymSetOptions_t)(_In_ DWORD SymOptions);

    typedef DWORD64 (*SymLoadModuleExW_t)(
        _In_ HANDLE hProcess,
        _In_opt_ HANDLE hFile,
        _In_opt_ PCWSTR ImageName,
        _In_opt_ PCWSTR ModuleName,
        _In_ DWORD64 BaseOfDll,
        _In_ DWORD DllSize,
        _In_opt_ PMODLOAD_DATA Data,
        _In_opt_ DWORD Flags
        );

    typedef BOOL (*SymGetLineFromAddrW64_t)(
        _In_ HANDLE hProcess,
        _In_ DWORD64 dwAddr,
        _Out_ PDWORD pdwDisplacement,
        _Out_ PIMAGEHLP_LINEW64 Line
        );
    typedef BOOL (*SymFromAddrW_t)(
        _In_ HANDLE hProcess,
        _In_ DWORD64 Address,
        _Out_opt_ PDWORD64 Displacement,
        _Inout_ PSYMBOL_INFOW Symbol
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

    class Locked : std::unique_lock<std::mutex> {
    public:
        explicit Locked(const DbghelpWrapper& owner)
            : std::unique_lock<std::mutex>(owner._barrier)
            , _owner(owner) {}

        Locked(const Locked& ) = delete;
        Locked& operator =(const Locked& ) = delete;

        Locked(Locked&& ) = default;
        Locked& operator =(Locked&& ) = default;

        SymInitializeW_t SymInitializeW() const { return _owner._symInitializeW; }
        SymCleanup_t SymCleanup() const { return _owner._symCleanup; }

        SymGetOptions_t SymGetOptions() const { return _owner._symGetOptions; }
        SymSetOptions_t SymSetOptions() const { return _owner._symSetOptions; }

        SymLoadModuleExW_t SymLoadModuleExW() const { return _owner._symLoadModuleExW; }

        SymFromAddrW_t SymFromAddrW() const { return _owner._symFromAddrW; }
        SymGetLineFromAddrW64_t SymGetLineFromAddrW64() const { return _owner._symGetLineFromAddrW64; }

        MiniDumpWriteDump_t MiniDumpWriteDump() const { return _owner._miniDumpWriteDump; }

    private:
        const DbghelpWrapper& _owner;
    };

    ~DbghelpWrapper();

    bool Available() const { return nullptr != _library; }
    Locked Lock() const { return Locked(*this); }

    using Meta::Singleton<DbghelpWrapper>::HasInstance;
    using Meta::Singleton<DbghelpWrapper>::Instance;

    static void Create() { Meta::Singleton<DbghelpWrapper>::Create(); }
    using Meta::Singleton<DbghelpWrapper>::Destroy;

private:
    friend Meta::Singleton<DbghelpWrapper>;

    DbghelpWrapper();

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
