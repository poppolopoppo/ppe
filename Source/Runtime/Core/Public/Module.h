#pragma once

#include "Core_fwd.h"

#ifndef PPE_STATICMODULES_STARTUP
#   define PPE_STATICMODULES_STARTUP ::PPE::FBaseModuleStartup
#endif

#define PPE_STATICMODULE_STARTUP_NAME(_NAME) CONCAT3(F, _NAME, Startup)
#define PPE_STATICMODULE_STARTUP_DEF(_NAME) \
    struct PPE_STATICMODULE_STARTUP_NAME(_NAME) : PPE_STATICMODULES_STARTUP { \
        CONCAT3(F, _NAME, Module) _NAME; \
        void Start(PPE::FModuleManager& manager) { \
            PPE_STATICMODULES_STARTUP::Start(manager); \
            PPE::FBaseModuleStartup::StartModule(manager, _NAME); \
        } \
        void Shutdown(PPE::FModuleManager& manager) { \
            PPE::FBaseModuleStartup::ShutdownModule(manager, _NAME); \
            PPE_STATICMODULES_STARTUP::Shutdown(manager); \
        } \
        virtual void ReleaseMemory(PPE::FModuleManager& manager) override { \
            PPE::FBaseModuleStartup::ReleaseMemoryInModule(manager, _NAME); \
            PPE_STATICMODULES_STARTUP::Shutdown(manager); \
        } \
    }

namespace PPE {
class FModuleManager;
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

    virtual void Start();
    virtual void Shutdown();
    virtual void ReleaseMemory();

private:
    const char* _name;
    EModuleStatus _status;
};
//----------------------------------------------------------------------------
PPE_CORE_API void ReleaseMemoryInModules();
//----------------------------------------------------------------------------
struct PPE_CORE_API FBaseModuleStartup {
public:
    virtual ~FBaseModuleStartup() = default;

    void Start(FModuleManager& manager);
    void Shutdown(FModuleManager& manager);
    virtual void ReleaseMemory(FModuleManager& manager);

protected:
    static void StartModule(FModuleManager& manager, FModule& m);
    static void ShutdownModule(FModuleManager& manager, FModule& m);
    static void ReleaseMemoryInModule(FModuleManager& manager, FModule& m);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
