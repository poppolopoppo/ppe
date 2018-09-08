#pragma once

#include "Core.h"

#include "ModuleManager.h"

#ifdef PPE_STATICMODULES_STARTUP
   error "Runtime/Core/targetmodule.h must always be included first !"
#endif

struct FCoreStartup : PPE::IModuleStartup {
    PPE::FModuleManager& Manager;
    PPE::FCoreModule Core;
    explicit FCoreStartup(PPE::FModuleManager& manager)
    :   Manager(manager) {
        Manager.PreInit(*this);
        Manager.Start(Core);
    }
    virtual ~FCoreStartup() {
        Manager.Shutdown(Core);
        Manager.PostDestroy(*this);
    }
    virtual void ReleaseMemory() override {
        Manager.ReleaseMemory(Core);
    }
};

#define PPE_STATICMODULES_STARTUP FCoreStartup

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
