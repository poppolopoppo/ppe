#include "stdafx.h"

#ifdef PLATFORM_WINDOWS

#include "HAL/Windows/VirtualAllocDetour.h"

#ifndef USE_PPE_VIRTUALALLOC_DETOUR
#   define USE_PPE_VIRTUALALLOC_DETOUR 0
#endif

#if USE_PPE_VIRTUALALLOC_DETOUR && not (defined(PLATFORM_WINDOWS) && defined(ARCH_X64))
#   undef USE_PPE_VIRTUALALLOC_DETOUR
#   define USE_PPE_VIRTUALALLOC_DETOUR 0
#endif

#if USE_PPE_VIRTUALALLOC_DETOUR

#include "minhook-external.h"

#include "Diagnostic/Logger.h"
#include "HAL/Windows/LastError.h"
#include "HAL/Windows/WindowsPlatformIncludes.h"
#include "HAL/Windows/WindowsPlatformMisc.h"
#include "IO/FormatHelpers.h"
#include "Memory/MemoryDomain.h"
#include "Meta/Singleton.h"
#include "Meta/Utility.h"
#include "Thread/CriticalSection.h"

namespace PPE {
LOG_CATEGORY(, VirtualAllocDetour)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
// https://github.com/khalladay/hooking-by-example/
//----------------------------------------------------------------------------
#if 0
//----------------------------------------------------------------------------
template <typename _Operand, size_t _OperandOffset, auto... _OpCodes>
struct TOpCodes_ {
    STATIC_CONST_INTEGRAL(size_t, OperandOffset, _OperandOffset);
    STATIC_CONST_INTEGRAL(size_t, Size, sizeof...(_OpCodes));
    STATIC_ASSERT(_OperandOffset + sizeof(_Operand) <= Size);
    static CONSTEXPR u8 Bytes[Size] = { _OpCodes... };

    static size_t Write(void* target, _Operand operand) NOEXCEPT {
        u8 opcodes[Size];
        ::memcpy(opcodes, Bytes, Size);
        ::memcpy(opcodes + _OperandOffset, &operand, sizeof(operand));
        ::memcpy(target, opcodes, Size);
        return Size;
    }

    static _Operand Relative(void* from, void* to) NOEXCEPT {
        return checked_cast<_Operand>(reinterpret_cast<intptr_t>(to) - (reinterpret_cast<intptr_t>(from) + Size));
    }
};
using TJump32_Rel_Imm_ = TOpCodes_<i32, 1,
    0xE9, 0xAA, 0xAA, 0xAA, 0xAA>;
using TJump32_Rel_REX_w_ = TOpCodes_<i32, 3,
    0x48, 0xFF, 0x25, 0xAA, 0xAA, 0xAA, 0xAA>;
using TJump64_Abs_ = TOpCodes_<u64, 2,
    0x49, 0xBA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, //mov 64 bit value into r10
    0x41, 0xFF, 0xE2>; //
//----------------------------------------------------------------------------
struct FWinAPIDetour_ {
    STATIC_CONST_INTEGRAL(u32, TrampolineSize, TJump32_Rel_Imm_::Size);

    ::LPCSTR FunctionName{ nullptr };
    ::FARPROC OriginalFunc{ nullptr };
    ::LPVOID TrampolineAddress{ nullptr };
    ::BYTE PrologueBackup[TrampolineSize];

    NODISCARD bool Valid() const NOEXCEPT {
        return (!!OriginalFunc && !!TrampolineAddress);
    }

    NODISCARD ::LPVOID TrampolineTarget() const NOEXCEPT {
        return (reinterpret_cast<u8*>(TrampolineAddress) + TJump64_Abs_::Size);
    }

    NODISCARD bool Create(::LPCWSTR libraryName, ::LPCSTR functionName, ::LPVOID proxyFunc) {
        Assert(libraryName);
        Assert(functionName);

        const ::HMODULE hLibrary = ::LoadLibraryW(libraryName);
        if (NULL == hLibrary) {
            LOG_LASTERROR(HAL, L"LoadLibraryW");
            return false;
        }

        const ::FARPROC originalFunc = ::GetProcAddress(hLibrary, functionName);
        if (NULL == originalFunc) {
            LOG_LASTERROR(HAL, L"GetProcAddress");
            return false;
        }

        return Create(functionName, originalFunc, proxyFunc);
    }

    NODISCARD bool Create(::LPCSTR functionName, ::FARPROC originalFunc, ::LPVOID proxyFunc) {
        FunctionName = functionName;
        OriginalFunc = originalFunc;

        ::memcpy(PrologueBackup, OriginalFunc, TrampolineSize);

        // skip jump tables
        while (0 == ::memcmp(PrologueBackup, TJump32_Rel_REX_w_::Bytes, TJump32_Rel_REX_w_::OperandOffset)) {
            u8 jumpTable[TJump32_Rel_REX_w_::Size];
            ::memcpy(jumpTable, OriginalFunc, TJump32_Rel_REX_w_::Size);

            void* const jumpTarget = (reinterpret_cast<u8*>(OriginalFunc) + TJump32_Rel_REX_w_::Size +
                *reinterpret_cast<const i32*>(&jumpTable[TJump32_Rel_REX_w_::OperandOffset]));

            OriginalFunc = (::FARPROC)jumpTarget;
            ::memcpy(PrologueBackup, OriginalFunc, TrampolineSize);
        }

        // backup original instructions
        ::DWORD oldProtect;
        LOG_CHECK(HAL, ::VirtualProtect(OriginalFunc, TrampolineSize, PAGE_EXECUTE_READWRITE, &oldProtect));

        // we need to use JMP rel32 even on x64 to fit in 5 bytes, so the offset between the trampoline
        // and the original function must fit inside a 32 bits integer: so we try to allocate the page
        // for the trampoline near the original function
        TrampolineAddress = FWindowsPlatformMisc::AllocateExecutablePageNearAddress(OriginalFunc);
        LOG_CHECK(HAL, !!TrampolineAddress);

        // the trampoline consists of the stolen bytes from the target function, following by a jump back
        // to the target function + 5 bytes, in order to continue the execution of that function. This continues like
        // a normal function call
        void* const trampolineJumpTarget = reinterpret_cast<u8*>(OriginalFunc) + TrampolineSize;

        auto dst = static_cast<u8*>(TrampolineAddress);
        dst += TJump64_Abs_::Write(dst, reinterpret_cast<uintptr_t>(proxyFunc));
        dst += TJump64_Abs_::Write(dst, reinterpret_cast<uintptr_t>(trampolineJumpTarget));

        // the last operation is to finally overwrite the first 5 bytes of the original function with a jump
        // to our proxy function
        TJump32_Rel_Imm_::Write(OriginalFunc, TJump32_Rel_Imm_::Relative(OriginalFunc, TrampolineAddress));
        return true;
    }

    void Destroy() {
        if (not TrampolineAddress)
            return;

        // same code for both x86 and x64
        LOG(HAL, Info, L"destroy detour hook on WinAPI function {0}()",
            MakeCStringView(FunctionName));

        // release virtual memory used by trampoline
        ::VirtualFree(TrampolineAddress, 0, MEM_RELEASE);
        TrampolineAddress = nullptr;

        // restore function prologue backup
        LOG_CHECKVOID(HAL, ::WriteProcessMemory(
            ::GetCurrentProcess(),
            (::LPVOID)OriginalFunc,
            PrologueBackup,
            TrampolineSize,
            nullptr));
    }
};
//----------------------------------------------------------------------------
template <auto _Func>
struct TAutoWinAPIDetour_ : FWinAPIDetour_ {
    TAutoWinAPIDetour_(::LPCWSTR libraryName, ::LPCSTR functionName) {
        if (not Create(libraryName, functionName, _Func)) {
            OriginalFunc = nullptr;
        }
    }

    ~TAutoWinAPIDetour_() {
        Destroy();
    }

    TAutoWinAPIDetour_(const TAutoWinAPIDetour_&) = delete;
    TAutoWinAPIDetour_& operator =(const TAutoWinAPIDetour_&) = delete;

    // call inside the hook to give back execution to the original function
    template <typename... _Args>
    auto operator ()(_Args&&... args) const {
        return reinterpret_cast<decltype(_Func)>(TrampolineTarget())(
            std::forward<_Args>(args)...);
    }
};
//----------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------
// https://github.com/TsudaKageyu/minhook
//----------------------------------------------------------------------------
template <typename _Char>
TBasicStringView<_Char> MH_Status_Message_(::MH_STATUS status) {
    switch (status) {
    case ::MH_UNKNOWN: return STRING_LITERAL(_Char, "MinHook unknown error, should not happen.");
    case ::MH_ERROR_ALREADY_INITIALIZED: return STRING_LITERAL(_Char, "MinHook is already initialized.");
    case ::MH_ERROR_NOT_INITIALIZED: return STRING_LITERAL(_Char, "MinHook is not initialized yet, or already uninitialized.");
    case ::MH_ERROR_ALREADY_CREATED: return STRING_LITERAL(_Char, "The hook for the specified target function is already created.");
    case ::MH_ERROR_NOT_CREATED: return STRING_LITERAL(_Char, "The hook for the specified target function is not created yet.");
    case ::MH_ERROR_ENABLED: return STRING_LITERAL(_Char, "The hook for the specified target function is already enabled.");
    case ::MH_ERROR_DISABLED: return STRING_LITERAL(_Char, "The hook for the specified target function is not enabled yet, or already disabled.");
    case ::MH_ERROR_NOT_EXECUTABLE: return STRING_LITERAL(_Char, "The specified pointer is invalid. It points the address of non-allocated and/or non-executable region.");
    case ::MH_ERROR_UNSUPPORTED_FUNCTION: return STRING_LITERAL(_Char, "The specified target function cannot be hooked.");
    case ::MH_ERROR_MEMORY_ALLOC: return STRING_LITERAL(_Char, "Failed to allocate memory.");
    case ::MH_ERROR_MEMORY_PROTECT: return STRING_LITERAL(_Char, "Failed to change the memory protection.");
    case ::MH_ERROR_MODULE_NOT_FOUND: return STRING_LITERAL(_Char, "The specified module is not loaded.");
    case ::MH_ERROR_FUNCTION_NOT_FOUND: return STRING_LITERAL(_Char, "specified function is not found.");
    default:
        AssertNotImplemented();
    }
}
struct FMinHookStatus_ {
    ::MH_STATUS Value;
};
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, FMinHookStatus_ status) {
    return oss << MH_Status_Message_<_Char>(status.Value);
}
//----------------------------------------------------------------------------
template <auto _Func>
struct TAutoWinAPIDetour_ {
    ::LPVOID Original{ NULL };
    ::LPVOID Target{ NULL };

    TAutoWinAPIDetour_(::LPCWSTR moduleName, ::LPCSTR functionName) {
        ::MH_STATUS status;

        status = ::MH_CreateHookApiEx(moduleName, functionName, _Func, &Original, &Target);
        if (::MH_OK == status) {
            status = ::MH_QueueEnableHook(Target);

            if (::MH_OK == status)
                LOG(VirtualAllocDetour, Info, L"MH_CreateHookApiEx: hooked function {0} in module {1}", MakeCStringView(functionName), MakeCStringView(moduleName));
            else
                LOG(VirtualAllocDetour, Error, L"MH_QueueEnableHook({0}): {1}", Fmt::Pointer(Target), FMinHookStatus_(status));
        }
        else {
            LOG(VirtualAllocDetour, Error, L"MH_CreateHookApiEx({0}, {1}): {2}", MakeCStringView(moduleName), MakeCStringView(functionName), FMinHookStatus_(status));
        }
    }

    ~TAutoWinAPIDetour_() {
        if (Target) {
            const ::MH_STATUS status = ::MH_RemoveHook(Target);
            if (::MH_OK != status)
                LOG(VirtualAllocDetour, Error, L"MH_RemoveHook({0}): {1}", Fmt::Pointer(Target), FMinHookStatus_(status));
        }
    }

    TAutoWinAPIDetour_(const TAutoWinAPIDetour_&) = delete;
    TAutoWinAPIDetour_& operator =(const TAutoWinAPIDetour_&) = delete;

    bool Valid() const { return (!!Target); }

    // call inside the hook to give back execution to the original function
    template <typename... _Args>
    auto operator ()(_Args&&... args) const {
        Assert_Lightweight(Target);
        return reinterpret_cast<decltype(_Func)>(Original)(std::forward<_Args>(args)...);
    }
};
//----------------------------------------------------------------------------
template <typename _Func>
::SIZE_T EachBlockInVirtualAlloc_(void* p, _Func&& each) {
    ::MEMORY_BASIC_INFORMATION info{};
    Verify(::VirtualQuery(p, &info, sizeof(info)) == sizeof(info));
    if (info.State & MEM_FREE)
        return 0;

    ::SIZE_T dwReservedSize = 0;
    char* const pAllocationBase = static_cast<char*>(info.AllocationBase);
    for (char* pBlock = pAllocationBase;;) {
        info.AllocationBase = nullptr;
        Verify(::VirtualQuery(pBlock, &info, sizeof(info)) == sizeof(info));
        if (info.AllocationBase != pAllocationBase)
            break;

        Assert_NoAssume(info.BaseAddress == pBlock);
        Assert_NoAssume(!(info.State & MEM_FREE));

        each(pBlock, info.RegionSize, info.State);

        pBlock += info.RegionSize;
        dwReservedSize += info.RegionSize;
    }

    return dwReservedSize;
}
//----------------------------------------------------------------------------
class FVirtualAllocDetourImpl_ : Meta::TStaticSingleton<FVirtualAllocDetourImpl_> {
    friend Meta::TStaticSingleton<FVirtualAllocDetourImpl_>;
    using singleton_type = Meta::TStaticSingleton<FVirtualAllocDetourImpl_>;
public: // TSingleton<>

    using singleton_type::Get;
#if USE_PPE_ASSERT
    using singleton_type::HasInstance;
#endif

    static void Create() {
        const ::MH_STATUS status = ::MH_Initialize();
        if (::MH_OK != status)
            LOG(VirtualAllocDetour, Error, L"MH_Initialize: {0}", FMinHookStatus_(status));

        singleton_type::Create();
    }

    static void Destroy() {
        singleton_type::Destroy();

        const ::MH_STATUS status = ::MH_Uninitialize();
        if (::MH_OK != status)
            LOG(VirtualAllocDetour, Error, L"MH_Uninitialize: {0}", FMinHookStatus_(status));
    }

    bool Valid() const NOEXCEPT {
        return (_virtualAlloc.Valid());
    }

private:
    ::DWORD dwAllocationGranularity;
    ::DWORD dwPageSize;

    FVirtualAllocDetourImpl_() {
        ::SYSTEM_INFO info;
        ::GetSystemInfo(&info);
        dwAllocationGranularity = info.dwAllocationGranularity;
        dwPageSize = info.dwPageSize;

        const ::MH_STATUS status = ::MH_ApplyQueued();
        if (::MH_OK != status)
            LOG(VirtualAllocDetour, Error, L"MH_ApplyQueued: {0}", FMinHookStatus_(status));
    }

    static FMemoryTracking& TrackingData() NOEXCEPT {
        return MEMORYDOMAIN_TRACKING_DATA(VirtualAlloc);
    }

    // tracking committed virtual memory is not trivial: allocations are not always symmetrical, and
    // VirtualQuery does not give all informations needed. So we need to parse virtual addresses every time
    struct FAllocationScope_ {
        FCriticalSection& Barrier;
        ::LPVOID lpAddress;
        ::SIZE_T dwReservedSize{ 0 };
        ::BOOL bEnabled;

        FAllocationScope_(FCriticalSection& barrier, ::LPVOID lp, ::SIZE_T dwSize, ::DWORD flAlloctionType, ::HANDLE hProcess)
        :   Barrier(barrier), lpAddress(lp), bEnabled(::GetCurrentProcess() == hProcess) {
            Unused(dwSize, flAlloctionType);
            if (Likely(bEnabled)) {
                Barrier.Lock();

                // deallocate every user tracking (aka committed memory) before alloc/free
                dwReservedSize = EachBlockInVirtualAlloc_(lpAddress, [this](::LPVOID, ::SIZE_T dwSize, ::DWORD flAllocationType) {
                    if (flAllocationType & MEM_COMMIT)
                        TrackingData().ReleaseBatchUser(1, dwSize);
                    });
            }
        }

        ~FAllocationScope_() {
            if (Likely(bEnabled)) {
                ON_SCOPE_EXIT([this]() NOEXCEPT {
                    Barrier.Unlock();
                });

                // need to retrieve all reserved memory to log system allocation *before* user allocations (aka committed memory)
                const ::SIZE_T dwNewReservedSize = EachBlockInVirtualAlloc_(lpAddress, NoFunction);

                // deduce system allocations (aka reserved memory) from previous parsing
                if (dwNewReservedSize != dwReservedSize) {
                    if (dwReservedSize)
                        TrackingData().ReleaseBatchSystem(1, dwReservedSize);
                    if (dwNewReservedSize)
                        TrackingData().AllocateSystem(dwNewReservedSize);
                }

                // (re)allocate all user tracking (aka committed memory) after alloc/free, need to be done *after* system memory
                const ::SIZE_T dwNewReservedSize2 = EachBlockInVirtualAlloc_(lpAddress, [](::LPVOID, ::SIZE_T dwSize, ::DWORD flAllocationType) {
                    if (flAllocationType & MEM_COMMIT)
                        TrackingData().AllocateUser(dwSize);
                    });

                Assert_NoAssume(dwNewReservedSize2 == dwNewReservedSize);
            }
        }
    };

    template <typename _FnAllocate>
    static ::LPVOID OnVirtualAlloc_(::HANDLE hProcess, ::LPVOID lpAddress, ::SIZE_T dwSize, ::DWORD flAllocationType, _FnAllocate&& fnAllocate) NOEXCEPT {
        FAllocationScope_ allocTracking(Get()._barrier, lpAddress, dwSize, flAllocationType, hProcess);
        allocTracking.lpAddress = fnAllocate();
        return allocTracking.lpAddress;
    }

    template <typename _FnDeallocate>
    static ::BOOL OnVirtualFree_(::HANDLE hProcess, ::LPVOID lpAddress, ::SIZE_T dwSize, ::DWORD flAllocationType, _FnDeallocate&& fnDeallocate) NOEXCEPT {
        Assert(lpAddress);
        const FAllocationScope_ freeTracking(Get()._barrier, lpAddress, dwSize, flAllocationType, hProcess);
        return fnDeallocate();
    }

    static ::LPVOID WINAPI VirtualAlloc_(
        _In_opt_::LPVOID lpAddress,
        _In_::SIZE_T dwSize,
        _In_::DWORD flAllocationType,
        _In_::DWORD flProtect ) {
        return OnVirtualAlloc_(::GetCurrentProcess(), lpAddress, dwSize, flAllocationType, [=]() {
            return Get()._virtualAlloc(lpAddress, dwSize, flAllocationType, flProtect);
        });
    }
    static ::LPVOID WINAPI VirtualAllocEx_(
        _In_::HANDLE hProcess,
        _In_opt_::LPVOID lpAddress,
        _In_::SIZE_T dwSize,
        _In_::DWORD flAllocationType,
        _In_::WORD flProtect ) {
        return OnVirtualAlloc_(hProcess, lpAddress, dwSize, flAllocationType, [=]() {
            return Get()._virtualAllocEx(hProcess, lpAddress, dwSize, flAllocationType, flProtect);
        });
    }
    static ::LPVOID WINAPI VirtualAllocExNuma_(
        _In_::HANDLE hProcess,
        _In_opt_::LPVOID lpAddress,
        _In_::SIZE_T dwSize,
        _In_::WORD flAllocationType,
        _In_::DWORD flProtect ) {
        return OnVirtualAlloc_(hProcess, lpAddress, dwSize, flAllocationType, [=]() {
            return Get()._virtualAllocExNuma(hProcess, lpAddress, dwSize, flAllocationType, flProtect);
        });
    }
    static ::LPVOID WINAPI VirtualAllocFromApp_(
        _In_::HANDLE hProcess,
        _In_opt_::LPVOID lpAddress,
        _In_::SIZE_T dwSize,
        _In_::DWORD flAllocationType,
        _In_::DWORD flProtect ) {
        return OnVirtualAlloc_(hProcess, lpAddress, dwSize, flAllocationType, [=]() {
            return Get()._virtualAllocFromApp(hProcess, lpAddress, dwSize, flAllocationType, flProtect);
        });
    }
    static ::LPVOID WINAPI VirtualAlloc2_(
        _In_opt_::HANDLE Process,
        _In_opt_::PVOID BaseAddress,
        _In_::SIZE_T Size,
        _In_::ULONG AllocationType,
        _In_::ULONG PageProtection,
        _Inout_updates_opt_(ParameterCount) ::MEM_EXTENDED_PARAMETER* ExtendedParameters,
        _In_::ULONG ParameterCount) {
        return OnVirtualAlloc_(Process, BaseAddress, Size, AllocationType, [=]() {
            return Get()._virtualAlloc2(Process, BaseAddress, Size, AllocationType, PageProtection, ExtendedParameters, ParameterCount);
        });
    }
    static ::LPVOID WINAPI VirtualAlloc2FromApp_(
        _In_opt_::HANDLE Process,
        _In_opt_::PVOID BaseAddress,
        _In_::SIZE_T Size,
        _In_::ULONG AllocationType,
        _In_::ULONG PageProtection,
        _Inout_updates_opt_(ParameterCount) ::MEM_EXTENDED_PARAMETER* ExtendedParameters,
        _In_::ULONG ParameterCount) {
        return OnVirtualAlloc_(Process, BaseAddress, Size, AllocationType, [=]() {
            return Get()._virtualAlloc2FromApp(Process, BaseAddress, Size, AllocationType, PageProtection, ExtendedParameters, ParameterCount);
        });
    }

    static ::BOOL WINAPI VirtualFree_(
        _Pre_notnull_ _When_(dwFreeType == MEM_DECOMMIT, _Post_invalid_) _When_(dwFreeType == MEM_RELEASE, _Post_ptr_invalid_) ::LPVOID lpAddress,
        _In_::SIZE_T dwSize,
        _In_::DWORD dwFreeType) {
        return OnVirtualFree_(::GetCurrentProcess(), lpAddress, dwSize, dwFreeType, [=]() {
            return Get()._virtualFree(lpAddress, dwSize, dwFreeType);
        });
    }
    static ::BOOL WINAPI VirtualFreeEx_(
        _In_::HANDLE hProcess,
        _Pre_notnull_ _When_(dwFreeType == MEM_DECOMMIT, _Post_invalid_) _When_(dwFreeType == MEM_RELEASE, _Post_ptr_invalid_) ::LPVOID lpAddress,
        _In_::SIZE_T dwSize,
        _In_::DWORD dwFreeType ) {
        return OnVirtualFree_(::GetCurrentProcess(), lpAddress, dwSize, dwFreeType, [=]() {
            return Get()._virtualFreeEx(hProcess, lpAddress, dwSize, dwFreeType);
        });
    }

    FCriticalSection _barrier;

    const TAutoWinAPIDetour_<&VirtualAlloc_> _virtualAlloc{ L"Kernel32.dll", "VirtualAlloc" };
    const TAutoWinAPIDetour_<&VirtualAllocEx_> _virtualAllocEx{ L"Kernel32.dll", "VirtualAllocEx" };
    const TAutoWinAPIDetour_<&VirtualAllocExNuma_> _virtualAllocExNuma{ L"Kernel32.dll", "VirtualAllocExNuma" };
    const TAutoWinAPIDetour_<&VirtualAllocFromApp_> _virtualAllocFromApp{ L"Kernel32.dll", "VirtualAllocFromApp" };
    const TAutoWinAPIDetour_<&VirtualAlloc2_> _virtualAlloc2{ L"Kernel32.dll", "VirtualAlloc2" };
    const TAutoWinAPIDetour_<&VirtualAlloc2FromApp_> _virtualAlloc2FromApp{ L"Kernel32.dll", "VirtualAlloc2FromApp" };

    const TAutoWinAPIDetour_<&VirtualFree_> _virtualFree{ L"Kernel32.dll", "VirtualFree" };
    const TAutoWinAPIDetour_<&VirtualFreeEx_> _virtualFreeEx{ L"Kernel32.dll", "VirtualFreeEx" };
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FVirtualAllocDetour::StartHooks() {
    LOG(VirtualAllocDetour, Info, L"hooking windows virtual memory for diagnostic");

    Assert_NoAssume(FWindowsPlatformMisc::Is64bitOperatingSystem());
    FVirtualAllocDetourImpl_::Create();
    return FVirtualAllocDetourImpl_::Get().Valid();
}
//----------------------------------------------------------------------------
void FVirtualAllocDetour::ShutdownHooks() {
    LOG(VirtualAllocDetour, Info, L"removing hooks on windows virtual memory");

    FVirtualAllocDetourImpl_::Destroy();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#else

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FVirtualAllocDetour::StartHooks() { return false; }
//----------------------------------------------------------------------------
void FVirtualAllocDetour::ShutdownHooks() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PPE_VIRTUALALLOC_DETOUR
#endif //!PLATFORM_WINDOWS