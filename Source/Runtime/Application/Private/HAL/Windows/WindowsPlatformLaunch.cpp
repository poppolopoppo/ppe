#include "stdafx.h"

#ifdef PLATFORM_WINDOWS

#include "HAL/Windows/WindowsPlatformLaunch.h"

#if USE_PPE_MEMORYDOMAINS
#   include "Memory/MemoryDomain.h"
#   include "Meta/Singleton.h"
#   include "HAL/Windows/WindowsPlatformMisc.h"
#   define USE_PPE_VIRTUALALLOC_TRAMPOLINE 1
#else
#   define USE_PPE_VIRTUALALLOC_TRAMPOLINE 0
#endif

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Virtual allocation hooks
//----------------------------------------------------------------------------
#if USE_PPE_VIRTUALALLOC_TRAMPOLINE
namespace {
//----------------------------------------------------------------------------
class FVirtualAllocTrampoline_ : Meta::TStaticSingleton<FVirtualAllocTrampoline_> {
    friend Meta::TStaticSingleton<FVirtualAllocTrampoline_>;
    using singleton_type = Meta::TStaticSingleton<FVirtualAllocTrampoline_>;
public: // TSingleton<>

    using singleton_type::Get;
#if USE_PPE_ASSERT
    using singleton_type::HasInstance;
#endif

    using singleton_type::Create;
    using singleton_type::Destroy;

    ~FVirtualAllocTrampoline_() {

    }

private:
    static ::LPVOID HOOK_VirtualAlloc_(
        _In_opt_::LPVOID lpAddress,
        _In_::SIZE_T dwSize,
        _In_::DWORD flAllocationType,
        _In_::DWORD flProtect
    ) {
        return FVirtualAllocTrampoline_::Get()._virtualAlloc(lpAddress, dwSize, flAllocationType, flProtect);
    }
    //using PFN_VirtualAlloc2 =
    //    WINBASEAPI ::LPVOID (*)(
    //        _In_opt_ ::HANDLE Process,
    //        _In_opt_ ::PVOID BaseAddress,
    //        _In_ ::SIZE_T Size,
    //        _In_ ::ULONG AllocationType,
    //        _In_ ::ULONG PageProtection,
    //        _Inout_updates_opt_(ParameterCount) ::MEM_EXTENDED_PARAMETER* ExtendedParameters,
    //        _In_ ::ULONG ParameterCount
    //        );
    //using PFN_VirtualAlloc2FromApp =
    //    WINBASEAPI ::LPVOID (*)(
    //        _In_opt_ ::HANDLE Process,
    //        _In_opt_ ::PVOID BaseAddress,
    //        _In_ ::SIZE_T Size,
    //        _In_ ::ULONG AllocationType,
    //        _In_ ::ULONG PageProtection,
    //        _Inout_updates_opt_(ParameterCount) ::MEM_EXTENDED_PARAMETER* ExtendedParameters,
    //        _In_ ::ULONG ParameterCount
    //        );
    //using PFN_VirtualAllocEx =
    //    WINBASEAPI ::LPVOID (*)(
    //        _In_ ::HANDLE hProcess,
    //        _In_opt_ ::LPVOID lpAddress,
    //        _In_ ::SIZE_T dwSize,
    //        _In_ ::DWORD flAllocationType,
    //        _In_ ::WORD flProtect
    //        );
    //using PFN_VirtualAllocExNuma =
    //    WINBASEAPI ::LPVOID (*)(
    //        _In_ ::HANDLE hProcess,
    //        _In_opt_ ::LPVOID lpAddress,
    //        _In_ ::SIZE_T dwSize,
    //        _In_ ::WORD flAllocationType,
    //        _In_ ::DWORD flProtect
    //        );
    //using PFN_VirtualAllocFromApp =
    //    WINBASEAPI ::LPVOID (*)(
    //        _In_ ::HANDLE hProcess,
    //        _In_opt_ ::LPVOID lpAddress,
    //        _In_ ::SIZE_T dwSize,
    //        _In_ ::DWORD flAllocationType,
    //        _In_ ::DWORD flProtect
    //        );
    //using PFN_VirtualFree =
    //    WINBASEAPI ::BOOL (*)(
    //        _Pre_notnull_ _When_(dwFreeType == MEM_DECOMMIT, _Post_invalid_) _When_(dwFreeType == MEM_RELEASE, _Post_ptr_invalid_) ::LPVOID lpAddress,
    //        _In_ ::SIZE_T dwSize,
    //        _In_ ::DWORD dwFreeType
    //        );
    //using PFN_VirtualFreeEx =
    //    WINBASEAPI ::BOOL (*)(
    //        _In_ ::HANDLE hProcess,
    //        _Pre_notnull_ _When_(dwFreeType == MEM_DECOMMIT, _Post_invalid_) _When_(dwFreeType == MEM_RELEASE, _Post_ptr_invalid_) ::LPVOID lpAddress,
    //        _In_ ::SIZE_T dwSize,
    //        _In_ ::DWORD dwFreeType
    //        );

    //using PFN_VirtualAlloc =
    //    WINBASEAPI::LPVOID(*)(
    //        _In_opt_::LPVOID lpAddress,
    //        _In_::SIZE_T dwSize,
    //        _In_::DWORD flAllocationType,
    //        _In_::DWORD flProtect
    //        );
    //using PFN_VirtualAlloc2 =
    //    WINBASEAPI::LPVOID(*)(
    //        _In_opt_::HANDLE Process,
    //        _In_opt_::PVOID BaseAddress,
    //        _In_::SIZE_T Size,
    //        _In_::ULONG AllocationType,
    //        _In_::ULONG PageProtection,
    //        _Inout_updates_opt_(ParameterCount) ::MEM_EXTENDED_PARAMETER* ExtendedParameters,
    //        _In_::ULONG ParameterCount
    //        );
    //using PFN_VirtualAlloc2FromApp =
    //    WINBASEAPI::LPVOID(*)(
    //        _In_opt_::HANDLE Process,
    //        _In_opt_::PVOID BaseAddress,
    //        _In_::SIZE_T Size,
    //        _In_::ULONG AllocationType,
    //        _In_::ULONG PageProtection,
    //        _Inout_updates_opt_(ParameterCount) ::MEM_EXTENDED_PARAMETER* ExtendedParameters,
    //        _In_::ULONG ParameterCount
    //        );
    //using PFN_VirtualAllocEx =
    //    WINBASEAPI::LPVOID(*)(
    //        _In_::HANDLE hProcess,
    //        _In_opt_::LPVOID lpAddress,
    //        _In_::SIZE_T dwSize,
    //        _In_::DWORD flAllocationType,
    //        _In_::WORD flProtect
    //        );
    //using PFN_VirtualAllocExNuma =
    //    WINBASEAPI::LPVOID(*)(
    //        _In_::HANDLE hProcess,
    //        _In_opt_::LPVOID lpAddress,
    //        _In_::SIZE_T dwSize,
    //        _In_::WORD flAllocationType,
    //        _In_::DWORD flProtect
    //        );
    //using PFN_VirtualAllocFromApp =
    //    WINBASEAPI::LPVOID(*)(
    //        _In_::HANDLE hProcess,
    //        _In_opt_::LPVOID lpAddress,
    //        _In_::SIZE_T dwSize,
    //        _In_::DWORD flAllocationType,
    //        _In_::DWORD flProtect
    //        );
    //using PFN_VirtualFree =
    //    WINBASEAPI::BOOL(*)(
    //        _Pre_notnull_ _When_(dwFreeType == MEM_DECOMMIT, _Post_invalid_) _When_(dwFreeType == MEM_RELEASE, _Post_ptr_invalid_) ::LPVOID lpAddress,
    //        _In_::SIZE_T dwSize,
    //        _In_::DWORD dwFreeType
    //        );
    //using PFN_VirtualFreeEx =
    //    WINBASEAPI::BOOL(*)(
    //        _In_::HANDLE hProcess,
    //        _Pre_notnull_ _When_(dwFreeType == MEM_DECOMMIT, _Post_invalid_) _When_(dwFreeType == MEM_RELEASE, _Post_ptr_invalid_) ::LPVOID lpAddress,
    //        _In_::SIZE_T dwSize,
    //        _In_::DWORD dwFreeType
    //        );

    FVirtualAllocTrampoline_() {

    }

    const FWindowsPlatformMisc::TAutoDetour<&HOOK_VirtualAlloc_> _virtualAlloc{ L"Kernel32.dll", "VirtualAlloc" };

    //FWindowsPlatformMisc::FTrampoline _virtualAlloc;
    //FWindowsPlatformMisc::FTrampoline _virtualAlloc2;
    //FWindowsPlatformMisc::FTrampoline _virtualAlloc2FromApp;
    //FWindowsPlatformMisc::FTrampoline _virtualAllocEx;
    //FWindowsPlatformMisc::FTrampoline _virtualAllocExNuma;
    //FWindowsPlatformMisc::FTrampoline _virtualAllocFromApp;
    //FWindowsPlatformMisc::FTrampoline _virtualFree;
    //FWindowsPlatformMisc::FTrampoline _virtualFreeEx;
};
//----------------------------------------------------------------------------
} //!namespace
#endif //!USE_PPE_VIRTUALALLOC_TRAMPOLINE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FWindowsPlatformLaunch::OnPlatformLaunch(void* appHandle, int nShowCmd, const wchar_t* filename, size_t argc, const wchar_t* const* argv) {
    VerifyRelease(SUCCEEDED(::CoInitialize(NULL))); // ASAP

#if USE_PPE_VIRTUALALLOC_TRAMPOLINE
    FVirtualAllocTrampoline_::Create();
#endif

    FGenericPlatformLaunch::OnPlatformLaunch(appHandle, nShowCmd, filename, argc, argv);
}
//----------------------------------------------------------------------------
void FWindowsPlatformLaunch::OnPlatformShutdown() {
    FGenericPlatformLaunch::OnPlatformShutdown();

#if USE_PPE_VIRTUALALLOC_TRAMPOLINE
    FVirtualAllocTrampoline_::Destroy();
#endif

    ::CoUninitialize(); // ALAP
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
