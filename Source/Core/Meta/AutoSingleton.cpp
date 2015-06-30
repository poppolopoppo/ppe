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
AutoSingletonBase::AutoSingletonBase() 
:   _pnext(nullptr) {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
AutoSingletonManager::AutoSingletonManager() 
:   _pinstances(nullptr) {
    Assert(IsInMainThread());
    Assert(!gAutoSingletonCreated);
    ONLY_IF_ASSERT(gAutoSingletonCreated = true);
}
//----------------------------------------------------------------------------
AutoSingletonManager::~AutoSingletonManager() {
    Assert(IsInMainThread());
    Assert(gAutoSingletonCreated);
    ONLY_IF_ASSERT(gAutoSingletonCreated = false);

    std::lock_guard<std::mutex> scopelock(_lock);
    {
        for (AutoSingletonBase *p = _pinstances; p; p = p->_pnext)
            checked_delete(p);
    
        _pinstances = nullptr;
    }
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
void AutoSingletonManager::Register(AutoSingletonBase *pinstance) {
    AssertRelease(pinstance);
    Assert(nullptr == pinstance->_pnext);
    Assert(gAutoSingletonCreated);

    AutoSingletonManager& manager = *AutoSingletonManager_();
    std::lock_guard<std::mutex> scopelock(manager._lock);
    {
        pinstance->_pnext = manager._pinstances;
        manager._pinstances = pinstance;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
