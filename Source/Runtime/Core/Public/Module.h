#pragma once

#include "Core.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EModuleStatus {
    Initialized,
    Started,
    Shutdown,
    Destroyed
};
//----------------------------------------------------------------------------
class FModuleManager;
//----------------------------------------------------------------------------
struct IModuleStartup {
    virtual ~IModuleStartup() {}
    virtual void ReleaseMemory() = 0;
};
//----------------------------------------------------------------------------
class PPE_CORE_API FModule {
public:
    virtual ~FModule();

    FModule(const FModule&) = delete;
    FModule& operator =(const FModule&) = delete;

    FModule(FModule&&) = delete;
    FModule& operator =(FModule&&) = delete;

    const char* Name() const;
    EModuleStatus Status() const;

protected:
    explicit FModule(const char* name);

    friend class FModuleManager;

    virtual void Start(FModuleManager& manager);
    virtual void Shutdown();
    virtual void ReleaseMemory();

private:
    const char* _name;
    EModuleStatus _status;
};
//----------------------------------------------------------------------------
PPE_CORE_API void ReleaseMemoryInModules();
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
