#pragma once

#include "Core_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FModule;
struct FBaseModuleStartup;
//----------------------------------------------------------------------------
class PPE_CORE_API FModuleManager : Meta::FNonCopyableNorMovable {
public:
    ~FModuleManager();

    void PreInit(FBaseModuleStartup& startup);
    void PostDestroy(FBaseModuleStartup& startup);

    void Start(FModule& m);
    void Shutdown(FModule& m);
    void ReleaseMemory(FModule& m);

    void ReleaseMemoryInModules();

    static FModuleManager& Get() NOEXCEPT;

private:
    FBaseModuleStartup* _startup;

    FModuleManager() NOEXCEPT;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
