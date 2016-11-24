#pragma once

#include "Core/Core.h"

/*
//
// FAutoSingleton
// -------------
// Enables lazy instantiation of a singleton.
// This is only allowed when we can't know the type in advance (like in memory pools).
// Use FAutoSingletonManager to kill all auto singletons.
//
*/

#include "Core/Meta/OneTimeInitialize.h"

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FAbstractAutoSingleton {
protected:
    FAbstractAutoSingleton() : _pnext(nullptr), _pprev(nullptr) {}

public:
    virtual ~FAbstractAutoSingleton() { // must be virtual to allow delete()
        Assert(nullptr == _pnext);
        Assert(nullptr == _pprev);
    }

    FAbstractAutoSingleton(FAbstractAutoSingleton&&) = delete;
    FAbstractAutoSingleton& operator =(FAbstractAutoSingleton&&) = delete;

    FAbstractAutoSingleton(const FAbstractAutoSingleton&) = delete;
    FAbstractAutoSingleton& operator =(const FAbstractAutoSingleton&) = delete;

private:
    friend class FAutoSingletonManagerImpl;
    FAbstractAutoSingleton *_pnext;
    FAbstractAutoSingleton *_pprev;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FAutoSingletonManager {
public:
    static void Start();
    static void Shutdown();

    static void Register(FAbstractAutoSingleton *pinstance);
    static void Unregister(FAbstractAutoSingleton *pinstance);
};
//----------------------------------------------------------------------------
class FThreadLocalAutoSingletonManager {
public:
    static void Start();
    static void Shutdown();

    static void Register(FAbstractAutoSingleton *pinstance);
    static void Unregister(FAbstractAutoSingleton *pinstance);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TAutoSingleton : public FAbstractAutoSingleton {
protected:
    TAutoSingleton() { FAutoSingletonManager::Register(this); }
    virtual ~TAutoSingleton() { FAutoSingletonManager::Unregister(this); }

public:
    static T& Instance() {
        ONE_TIME_INITIALIZE_TPL(T *, sInstance, new T() );
        return *sInstance;
    }
};
//----------------------------------------------------------------------------
template <typename T>
class TThreadLocalAutoSingleton : public FAbstractAutoSingleton {
protected:
    TThreadLocalAutoSingleton() { FThreadLocalAutoSingletonManager::Register(this); }
    virtual ~TThreadLocalAutoSingleton() { FThreadLocalAutoSingletonManager::Unregister(this); }

public:
    static T& Instance() {
        ONE_TIME_INITIALIZE_THREAD_LOCAL_TPL(T *, sTlsInstance, new T() );
        return *sTlsInstance;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
