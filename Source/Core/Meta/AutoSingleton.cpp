#include "stdafx.h"

#include "AutoSingleton.h"

#include "Singleton.h"

#include <mutex>
#include <type_traits>

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class AutoSingletonManagerImpl {
public:
    AutoSingletonManagerImpl();
    ~AutoSingletonManagerImpl();

    void Start();
    void Shutdown();
    void Register(AbstractAutoSingleton *singleton);

private:
    AbstractAutoSingleton *_head;
    ONLY_IF_ASSERT(bool _isStarted = false;)
};
//----------------------------------------------------------------------------
AutoSingletonManagerImpl::AutoSingletonManagerImpl()
:   _head(nullptr) {}
//----------------------------------------------------------------------------
AutoSingletonManagerImpl::~AutoSingletonManagerImpl() {
    Assert(nullptr == _head);
}
//----------------------------------------------------------------------------
void AutoSingletonManagerImpl::Start() {
    Assert(!_isStarted);
    ONLY_IF_ASSERT(_isStarted = true;);
    Assert(nullptr == _head);
}
//----------------------------------------------------------------------------
void AutoSingletonManagerImpl::Shutdown() {
    Assert(_isStarted);
    ONLY_IF_ASSERT(_isStarted = false;);

    while (_head) {
        AbstractAutoSingleton *const node = _head;
        _head = _head->_pnext;
    }
}
//----------------------------------------------------------------------------
void AutoSingletonManagerImpl::Register(AbstractAutoSingleton *singleton) {
    Assert(singleton);
    Assert(nullptr == singleton->_pnext);

    singleton->_pnext = _head;
    _head = singleton;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class AutoSingletonManagerImplThreadSafe : public AutoSingletonManagerImpl {
public:
    void Start();
    void Shutdown();

    void Register(AbstractAutoSingleton *singleton);

private:
    std::mutex _barrier;
};
//----------------------------------------------------------------------------
void AutoSingletonManagerImplThreadSafe::Start() {
    std::lock_guard<std::mutex> scopeLock(_barrier);
    AutoSingletonManagerImpl::Start();
}
//----------------------------------------------------------------------------
void AutoSingletonManagerImplThreadSafe::Shutdown() {
    std::lock_guard<std::mutex> scopeLock(_barrier);
    AutoSingletonManagerImpl::Shutdown();
}
//----------------------------------------------------------------------------
void AutoSingletonManagerImplThreadSafe::Register(AbstractAutoSingleton *singleton) {
    std::lock_guard<std::mutex> scopeLock(_barrier);
    AutoSingletonManagerImpl::Register(singleton);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static AutoSingletonManagerImpl& AutoSingletonManager_() {
    ONE_TIME_DEFAULT_INITIALIZE(AutoSingletonManagerImplThreadSafe, sInstance);
    return sInstance;
}
//----------------------------------------------------------------------------
void AutoSingletonManager::Start() {
    AutoSingletonManager_().Start();
}
//----------------------------------------------------------------------------
void AutoSingletonManager::Shutdown() {
    AutoSingletonManager_().Shutdown();
}
//----------------------------------------------------------------------------
void AutoSingletonManager::Register(AbstractAutoSingleton *pinstance) {
    AutoSingletonManager_().Register(pinstance);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static AutoSingletonManagerImpl& ThreadLocalAutoSingletonManager_() {
    ONE_TIME_DEFAULT_INITIALIZE_THREAD_LOCAL(AutoSingletonManagerImpl, sTlsInstance);
    return sTlsInstance;
}
//----------------------------------------------------------------------------
void ThreadLocalAutoSingletonManager::Start() {
    ThreadLocalAutoSingletonManager_().Start();
}
//----------------------------------------------------------------------------
void ThreadLocalAutoSingletonManager::Shutdown() {
    ThreadLocalAutoSingletonManager_().Shutdown();
}
//----------------------------------------------------------------------------
void ThreadLocalAutoSingletonManager::Register(AbstractAutoSingleton *pinstance) {
    ThreadLocalAutoSingletonManager_().Register(pinstance);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
