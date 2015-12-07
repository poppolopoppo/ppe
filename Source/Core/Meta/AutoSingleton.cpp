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
    void Unregister(AbstractAutoSingleton *singleton);

private:
    AbstractAutoSingleton *_head;
#ifdef WITH_CORE_ASSERT
    bool _isStarted = false;
#endif
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

    while (_head)
        checked_delete(_head);
}
//----------------------------------------------------------------------------
void AutoSingletonManagerImpl::Register(AbstractAutoSingleton *singleton) {
    Assert(singleton);
    Assert(nullptr == singleton->_pnext);
    Assert(nullptr == singleton->_pprev);

    singleton->_pnext = _head;
    if (_head)
        _head->_pprev = singleton;

    _head = singleton;
}
//----------------------------------------------------------------------------
void AutoSingletonManagerImpl::Unregister(AbstractAutoSingleton *singleton) {
    Assert(singleton);
    Assert(_head);

    if (singleton->_pnext)
        singleton->_pnext->_pprev = singleton->_pprev;
    if (singleton->_pprev)
        singleton->_pprev->_pnext = singleton->_pnext;

    if (singleton == _head) {
        Assert(nullptr == singleton->_pprev);
        _head = singleton->_pnext;
    }

    singleton->_pnext = singleton->_pprev = nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class AutoSingletonManagerImplThreadSafe : public AutoSingletonManagerImpl {
public:
    void Start();
    void Shutdown();

    void Register(AbstractAutoSingleton *singleton);
    void Unregister(AbstractAutoSingleton *singleton);

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
void AutoSingletonManagerImplThreadSafe::Unregister(AbstractAutoSingleton *singleton) {
    std::lock_guard<std::mutex> scopeLock(_barrier);
    AutoSingletonManagerImpl::Unregister(singleton);
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
void AutoSingletonManager::Unregister(AbstractAutoSingleton *pinstance) {
    AutoSingletonManager_().Unregister(pinstance);
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
void ThreadLocalAutoSingletonManager::Unregister(AbstractAutoSingleton *pinstance) {
    ThreadLocalAutoSingletonManager_().Unregister(pinstance);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
