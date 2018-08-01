#pragma once

#include <stdio.h>
#include <tchar.h>

#include <algorithm>
#include <atomic>
#include <memory>
#include <type_traits>

#ifdef EXPORT_PPE
#   define PPE_API DLL_EXPORT
#else
#   define PPE_API DLL_IMPORT
#endif

#include "Meta/Config.h"

#include "Meta/Aliases.h"
#include "Meta/Alignment.h"
#include "Meta/Assert.h"
#include "Meta/Cast.h"
#include "Meta/Delete.h"
#include "Meta/Enum.h"
#include "Meta/ForRange.h"
#include "Meta/Hash_fwd.h"
#include "Meta/Iterator.h"
#include "Meta/NumericLimits.h"
#include "Meta/OneTimeInitialize.h"
#include "Meta/StronglyTyped.h"
#include "Meta/ThreadResource.h"
#include "Meta/TypeTraits.h"
#include "Meta/Warnings.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// FCoreModule is the entry and exit point encapsulating every call to Core::.
// Constructed with the same lifetime than the program (or application if segregated).
//----------------------------------------------------------------------------
class PPE_API FCoreModule {
public:
    static void Start(void *applicationHandle, int nShowCmd, const wchar_t* filename, size_t argc, const wchar_t** argv);
    static void Shutdown();

    static void ClearAll_UnusedMemory();

    FCoreModule(void *applicationHandle, int nShowCmd, const wchar_t* filename, size_t argc, const wchar_t** argv) {
        Start(applicationHandle, nShowCmd, filename, argc, argv);
    }

    ~FCoreModule() {
        Shutdown();
    }
};
//----------------------------------------------------------------------------
// Called for each module on start
#ifndef FINAL_RELEASE
struct PPE_API OnModuleStart {
    const wchar_t* const ModuleName;
    OnModuleStart(const wchar_t* moduleName);
    ~OnModuleStart();
};
#define PPE_MODULE_START(_Name) \
    const Core::OnModuleStart onModuleStart(WSTRINGIZE(_Name))
#else
#define PPE_MODULE_START(_Name) NOOP()
#endif
//----------------------------------------------------------------------------
// Called for each module on shutdown
#ifndef FINAL_RELEASE
struct PPE_API OnModuleShutdown {
    const wchar_t* const ModuleName;
    OnModuleShutdown(const wchar_t* moduleName);
    ~OnModuleShutdown();
};
#define PPE_MODULE_SHUTDOWN(_Name) \
    const Core::OnModuleShutdown onModuleShutdown(WSTRINGIZE(_Name))
#else
#define PPE_MODULE_SHUTDOWN(_Name) NOOP()
#endif
//----------------------------------------------------------------------------
// Called for each module on ClearAll_UnusedMemory
#ifndef FINAL_RELEASE
struct PPE_API OnModuleClearAll {
    const wchar_t* const ModuleName;
    OnModuleClearAll(const wchar_t* moduleName);
    ~OnModuleClearAll();
};
#define PPE_MODULE_CLEARALL(_Name) \
    const Core::OnModuleClearAll onModuleClearAll(WSTRINGIZE(_Name))
#else
#define PPE_MODULE_CLEARALL(_Name) NOOP()
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
