#include "stdafx.h"

#include "AutoSingleton.h"

#include <type_traits>

#include "Diagnostic/Logger.h"
#include "Singleton.h"
#include "Thread/ThreadContext.h"

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
typedef POD_STORAGE(AutoSingletonManager) AutoSingletonManagerPod;
//----------------------------------------------------------------------------
#ifdef WITH_CORE_ASSERT
static bool gAutoSingletonCreated = false;
#endif
static AutoSingletonManagerPod gAutoSingletonPOD;
//----------------------------------------------------------------------------
static AutoSingletonManager *AutoSingletonManager_() {
    return reinterpret_cast<AutoSingletonManager *>(&gAutoSingletonPOD);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
AutoSingletonManager::AutoSingletonManager() {
    Assert(IsInMainThread());
    Assert(!gAutoSingletonCreated);
    ONLY_IF_ASSERT(gAutoSingletonCreated = true);

    _instances.reserve(32);
}
//----------------------------------------------------------------------------
AutoSingletonManager::~AutoSingletonManager() {
    Assert(IsInMainThread());
    Assert(gAutoSingletonCreated);
    ONLY_IF_ASSERT(gAutoSingletonCreated = false);

    for (AutoSingletonBase *singleton : _instances)
        checked_delete(singleton);

    _instances.clear();
}
//----------------------------------------------------------------------------
void AutoSingletonManager::Start() {
    new (AutoSingletonManager_()) AutoSingletonManager();
}
//----------------------------------------------------------------------------
void AutoSingletonManager::Shutdown() {
    AutoSingletonManager_()->~AutoSingletonManager();
}
//----------------------------------------------------------------------------
void AutoSingletonManager::Register(AutoSingletonBase *instance) {
    AssertRelease(instance);
    Assert(gAutoSingletonCreated);

    AutoSingletonManager *const manager = AutoSingletonManager_();
    std::lock_guard<std::mutex> scopelock(manager->_lock);
    {
        manager->_instances.emplace_back(instance);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
