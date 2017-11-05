#pragma once

#include <stdio.h>
#include <tchar.h>

#include <algorithm>
#include <atomic>
#include <memory>
#include <type_traits>

#ifdef EXPORT_CORE
#   define CORE_API DLL_EXPORT
#else
#   define CORE_API DLL_IMPORT
#endif

#include "Core/Meta/Config.h"

#include "Core/Meta/Aliases.h"
#include "Core/Meta/Alignment.h"
#include "Core/Meta/Assert.h"
#include "Core/Meta/BitCount.h"
#include "Core/Meta/Cast.h"
#include "Core/Meta/Delete.h"
#include "Core/Meta/Enum.h"
#include "Core/Meta/ForRange.h"
#include "Core/Meta/Hash_fwd.h"
#include "Core/Meta/Iterator.h"
#include "Core/Meta/NumericLimits.h"
#include "Core/Meta/OneTimeInitialize.h"
#include "Core/Meta/StronglyTyped.h"
#include "Core/Meta/ThreadResource.h"
#include "Core/Meta/TypeTraits.h"
#include "Core/Meta/Warnings.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// FCoreModule is the entry and exit point encapsulating every call to Core::.
// Constructed with the same lifetime than the program (or application if segregated).
//----------------------------------------------------------------------------
class CORE_API FCoreModule {
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
struct CORE_API OnModuleStart {
    const wchar_t* const ModuleName;
    OnModuleStart(const wchar_t* moduleName);
    ~OnModuleStart();
};
#define CORE_MODULE_START(_Name) \
    const Core::OnModuleStart onModuleStart(WSTRINGIZE(_Name))
#else
#define CORE_MODULE_START(_Name) NOOP()
#endif
//----------------------------------------------------------------------------
// Called for each module on shutdown
#ifndef FINAL_RELEASE
struct CORE_API OnModuleShutdown {
    const wchar_t* const ModuleName;
    OnModuleShutdown(const wchar_t* moduleName);
    ~OnModuleShutdown();
};
#define CORE_MODULE_SHUTDOWN(_Name) \
    const Core::OnModuleShutdown onModuleShutdown(WSTRINGIZE(_Name))
#else
#define CORE_MODULE_SHUTDOWN(_Name) NOOP()
#endif
//----------------------------------------------------------------------------
// Called for each module on ClearAll_UnusedMemory
#ifndef FINAL_RELEASE
struct CORE_API OnModuleClearAll {
    const wchar_t* const ModuleName;
    OnModuleClearAll(const wchar_t* moduleName);
    ~OnModuleClearAll();
};
#define CORE_MODULE_CLEARALL(_Name) \
    const Core::OnModuleClearAll onModuleClearAll(WSTRINGIZE(_Name))
#else
#define CORE_MODULE_CLEARALL(_Name) NOOP()
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
