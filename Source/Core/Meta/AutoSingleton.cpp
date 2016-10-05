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
class FAutoSingletonManagerImpl {
public:
    FAutoSingletonManagerImpl();
    ~FAutoSingletonManagerImpl();

    void Start();
    void Shutdown();

    void Register(FAbstractAutoSingleton *singleton);
    void Unregister(FAbstractAutoSingleton *singleton);

private:
    FAbstractAutoSingleton *_head;
#ifdef WITH_CORE_ASSERT
    bool _isStarted = false;
#endif
};
//----------------------------------------------------------------------------
FAutoSingletonManagerImpl::FAutoSingletonManagerImpl()
:   _head(nullptr) {}
//----------------------------------------------------------------------------
FAutoSingletonManagerImpl::~FAutoSingletonManagerImpl() {
    Assert(nullptr == _head);
}
//----------------------------------------------------------------------------
void FAutoSingletonManagerImpl::Start() {
    Assert(!_isStarted);
    ONLY_IF_ASSERT(_isStarted = true;);
    Assert(nullptr == _head);
}
//----------------------------------------------------------------------------
void FAutoSingletonManagerImpl::Shutdown() {
    Assert(_isStarted);
    ONLY_IF_ASSERT(_isStarted = false;);

    while (_head)
        checked_delete(_head);
}
//----------------------------------------------------------------------------
void FAutoSingletonManagerImpl::Register(FAbstractAutoSingleton *singleton) {
    Assert(singleton);
    Assert(nullptr == singleton->_pnext);
    Assert(nullptr == singleton->_pprev);

    singleton->_pnext = _head;
    if (_head)
        _head->_pprev = singleton;

    _head = singleton;
}
//----------------------------------------------------------------------------
void FAutoSingletonManagerImpl::Unregister(FAbstractAutoSingleton *singleton) {
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
class FAutoSingletonManagerImplThreadSafe : public FAutoSingletonManagerImpl {
public:
    void Start();
    void Shutdown();

    void Register(FAbstractAutoSingleton *singleton);
    void Unregister(FAbstractAutoSingleton *singleton);

private:
    std::mutex _barrier;
};
//----------------------------------------------------------------------------
void FAutoSingletonManagerImplThreadSafe::Start() {
    std::lock_guard<std::mutex> scopeLock(_barrier);
    FAutoSingletonManagerImpl::Start();
}
//----------------------------------------------------------------------------
void FAutoSingletonManagerImplThreadSafe::Shutdown() {
    std::lock_guard<std::mutex> scopeLock(_barrier);
    FAutoSingletonManagerImpl::Shutdown();
}
//----------------------------------------------------------------------------
void FAutoSingletonManagerImplThreadSafe::Register(FAbstractAutoSingleton *singleton) {
    std::lock_guard<std::mutex> scopeLock(_barrier);
    FAutoSingletonManagerImpl::Register(singleton);
}
//----------------------------------------------------------------------------
void FAutoSingletonManagerImplThreadSafe::Unregister(FAbstractAutoSingleton *singleton) {
    std::lock_guard<std::mutex> scopeLock(_barrier);
    FAutoSingletonManagerImpl::Unregister(singleton);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static FAutoSingletonManagerImpl& AutoSingletonManager_() {
    ONE_TIME_DEFAULT_INITIALIZE(FAutoSingletonManagerImplThreadSafe, sInstance);
    return sInstance;
}
//----------------------------------------------------------------------------
void FAutoSingletonManager::Start() {
    AutoSingletonManager_().Start();
}
//----------------------------------------------------------------------------
void FAutoSingletonManager::Shutdown() {
    AutoSingletonManager_().Shutdown();
}
//----------------------------------------------------------------------------
void FAutoSingletonManager::Register(FAbstractAutoSingleton *pinstance) {
    AutoSingletonManager_().Register(pinstance);
}
//----------------------------------------------------------------------------
void FAutoSingletonManager::Unregister(FAbstractAutoSingleton *pinstance) {
    AutoSingletonManager_().Unregister(pinstance);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static FAutoSingletonManagerImpl& ThreadLocalAutoSingletonManager_() {
    ONE_TIME_DEFAULT_INITIALIZE_THREAD_LOCAL(FAutoSingletonManagerImpl, sTlsInstance);
    return sTlsInstance;
}
//----------------------------------------------------------------------------
void FThreadLocalAutoSingletonManager::Start() {
    ThreadLocalAutoSingletonManager_().Start();
}
//----------------------------------------------------------------------------
void FThreadLocalAutoSingletonManager::Shutdown() {
    ThreadLocalAutoSingletonManager_().Shutdown();
}
//----------------------------------------------------------------------------
void FThreadLocalAutoSingletonManager::Register(FAbstractAutoSingleton *pinstance) {
    ThreadLocalAutoSingletonManager_().Register(pinstance);
}
//----------------------------------------------------------------------------
void FThreadLocalAutoSingletonManager::Unregister(FAbstractAutoSingleton *pinstance) {
    ThreadLocalAutoSingletonManager_().Unregister(pinstance);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
