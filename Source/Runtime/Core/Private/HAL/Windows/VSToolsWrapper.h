﻿#pragma once

#include "HAL/Windows/WindowsPlatformDebug.h"

#if USE_PPE_PLATFORM_DEBUG

#   include "External/vstools/Public/vstools.h"

#   include "HAL/Windows/WindowsPlatformIncludes.h"

#   include "Diagnostic/Logger.h"
#   include "IO/String.h"
#   include "Meta/Singleton.h"
#   include "Misc/DynamicLibrary.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FVSToolsWrapper : Meta::TSingleton<FVSToolsWrapper> {
public:
    using FHeapTracker = vstools_FHeapTracker;

    typedef FHeapTracker (*FCreateHeapTracker)(const char* name);
    typedef void (*FDestroyHeapTracker)(FHeapTracker heap);
    typedef bool (*FIsValidHeapTracker)(FHeapTracker heap) NOEXCEPT;
    typedef bool (*FAllocateEvent)(FHeapTracker heap, void* ptr, size_t sz);
    typedef bool (*FReallocateEvent)(FHeapTracker heap, void* newp, size_t newsz, void* oldp);
    typedef bool (*FDeallocateEvent)(FHeapTracker heap, void* ptr);

    struct FAPI {
        FCreateHeapTracker CreateHeapTracker;
        FDestroyHeapTracker DestroyHeapTracker;
        FIsValidHeapTracker IsValidHeapTracker;
        FAllocateEvent AllocateEvent;
        FReallocateEvent ReallocateEvent;
        FDeallocateEvent DeallocateEvent;
    };

    static CONSTEXPR const FAPI GDummyAPI{
        [](const char*) CONSTEXPR -> FHeapTracker { return nullptr; },
        [](FHeapTracker) CONSTEXPR {},
        [](FHeapTracker) CONSTEXPR NOEXCEPT -> bool { return true; },
        [](FHeapTracker, void*, size_t) CONSTEXPR { return true; },
        [](FHeapTracker, void*, size_t, void*) CONSTEXPR { return true; },
        [](FHeapTracker, void*) CONSTEXPR { return true; }
    };

    static FAPI API;

    static FHeapTracker HEAP_Alloca;
    static FHeapTracker HEAP_Malloc;
    static FHeapTracker HEAP_Linear;

    ~FVSToolsWrapper();

    bool Available() const { return (_dll.IsValid()); }

    using Meta::TSingleton<FVSToolsWrapper>::Get;
    using Meta::TSingleton<FVSToolsWrapper>::Destroy;
    static void Create() { Meta::TSingleton<FVSToolsWrapper>::Create(); }

private:
    friend class Meta::TSingleton<FVSToolsWrapper>;

    FVSToolsWrapper();
    FDynamicLibrary _dll;
};
//----------------------------------------------------------------------------
inline FVSToolsWrapper::FAPI FVSToolsWrapper::API{ GDummyAPI };
//----------------------------------------------------------------------------
inline FVSToolsWrapper::FHeapTracker FVSToolsWrapper::HEAP_Alloca = nullptr;
inline FVSToolsWrapper::FHeapTracker FVSToolsWrapper::HEAP_Malloc = nullptr;
inline FVSToolsWrapper::FHeapTracker FVSToolsWrapper::HEAP_Linear = nullptr;
//----------------------------------------------------------------------------
inline FVSToolsWrapper::FVSToolsWrapper() {
    static const wchar_t* GVSToolsDLL[] = {
        L"vstools-" WSTRINGIZE(BUILDCONFIG) L".dll"
    };

    for (const wchar_t* filename : GVSToolsDLL) {
        if (_dll.AttachOrLoad(filename))
            break;
    }

    if (_dll) {
        FAPI& api = API;

        Verify((api.CreateHeapTracker = (FCreateHeapTracker)_dll.FunctionAddr("vstools_CreateHeapTracker")) != nullptr);
        Verify((api.DestroyHeapTracker= (FDestroyHeapTracker)_dll.FunctionAddr("vstools_DestroyHeapTracker")) != nullptr);
        Verify((api.IsValidHeapTracker = (FIsValidHeapTracker)_dll.FunctionAddr("vstools_IsValidHeapTracker")) != nullptr);
        Verify((api.AllocateEvent = (FAllocateEvent)_dll.FunctionAddr("vstools_AllocateEvent")) != nullptr);
        Verify((api.ReallocateEvent = (FReallocateEvent)_dll.FunctionAddr("vstools_ReallocateEvent")) != nullptr);
        Verify((api.DeallocateEvent = (FDeallocateEvent)_dll.FunctionAddr("vstools_DeallocateEvent")) != nullptr);

        LOG(HAL, Info, L"successfully loaded vstools dll from");
    }
    else {
        API = GDummyAPI;

        LOG(HAL, Warning, L"failed to load vstools dll, fallbacking to dummies !");
    }

    HEAP_Alloca = API.CreateHeapTracker("Alloca");
    HEAP_Malloc = API.CreateHeapTracker("Malloc");
    HEAP_Linear = API.CreateHeapTracker("Linear");

    Assert_NoAssume(API.IsValidHeapTracker(HEAP_Alloca));
    Assert_NoAssume(API.IsValidHeapTracker(HEAP_Malloc));
    Assert_NoAssume(API.IsValidHeapTracker(HEAP_Linear));
}
//----------------------------------------------------------------------------
inline FVSToolsWrapper::~FVSToolsWrapper() {
    FHeapTracker localAlloca = nullptr;
    FHeapTracker localMalloc = nullptr;
    FHeapTracker localLinear = nullptr;

    std::swap(localAlloca, HEAP_Alloca);
    std::swap(localMalloc, HEAP_Malloc);
    std::swap(localLinear, HEAP_Linear);

    API.DestroyHeapTracker(localAlloca);
    API.DestroyHeapTracker(localMalloc);
    API.DestroyHeapTracker(localLinear);

    if (_dll) {
        LOG(HAL, Info, L"unloading vstools dll");

        API = GDummyAPI;
        _dll.Unload();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PPE_PLATFORM_DEBUG