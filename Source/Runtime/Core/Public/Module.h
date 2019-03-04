#pragma once

#include "Core_fwd.h"

#ifndef PPE_STATICMODULES_STARTUP
#   define PPE_STATICMODULES_STARTUP ::PPE::FBaseModuleStartup
#endif

#define PPE_STATICMODULE_STARTUP_NAME(_NAME) CONCAT3(F, _NAME, Startup)
#define PPE_STATICMODULE_STARTUP_DEF(_NAME) \
    struct PPE_STATICMODULE_STARTUP_NAME(_NAME) : PPE_STATICMODULES_STARTUP { \
        CONCAT3(F, _NAME, Module) _NAME; \
        explicit PPE_STATICMODULE_STARTUP_NAME(_NAME)(PPE::FModuleManager& manager) \
        :   PPE_STATICMODULES_STARTUP(manager) { \
            Manager.Start(_NAME); \
        } \
        virtual ~PPE_STATICMODULE_STARTUP_NAME(_NAME)() { \
            Manager.Shutdown(_NAME); \
        } \
        virtual void ReleaseMemory() override { \
            Manager.ReleaseMemory(_NAME); \
            PPE_STATICMODULES_STARTUP::ReleaseMemory(); \
        } \
    }

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
struct PPE_CORE_API IModuleStartup {
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
struct PPE_CORE_API FBaseModuleStartup : public IModuleStartup {
    PPE::FModuleManager& Manager;
    explicit FBaseModuleStartup(PPE::FModuleManager& manager);
    virtual ~FBaseModuleStartup();
    virtual void ReleaseMemory() override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
