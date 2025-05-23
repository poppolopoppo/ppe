#pragma once

#include "Core.h"

/*
//
// FAutoSingleton
// -------------
// Enables lazy instantiation of a singleton.
// This is only allowed when we can't know the type in advance (like in memory pools).
// Use FAutoSingletonManager to kill all auto singletons.
//
*/

#include "Allocator/Malloc.h"
#include "Container/IntrusiveList.h"
#include "Meta/OneTimeInitialize.h"

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FAbstractAutoSingleton {
protected:
    FAbstractAutoSingleton() NOEXCEPT
    :   _node{ nullptr, nullptr }
    {}

public:
    virtual ~FAbstractAutoSingleton() { // must be virtual to allow delete()
        Assert(nullptr == _node.Prev);
        Assert(nullptr == _node.Next);
    }

    FAbstractAutoSingleton(FAbstractAutoSingleton&&) = delete;
    FAbstractAutoSingleton& operator =(FAbstractAutoSingleton&&) = delete;

    FAbstractAutoSingleton(const FAbstractAutoSingleton&) = delete;
    FAbstractAutoSingleton& operator =(const FAbstractAutoSingleton&) = delete;

private:
    friend class FAutoSingletonManagerImpl;
    TIntrusiveListNode<FAbstractAutoSingleton> _node;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FAutoSingletonManager {
public:
    static void Start();
    static void Shutdown();

    static void Register(FAbstractAutoSingleton *pinstance);
    static void Unregister(FAbstractAutoSingleton *pinstance);
};
//----------------------------------------------------------------------------
class PPE_CORE_API FThreadLocalAutoSingletonManager {
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
    static T& Get() {
        ONE_TIME_INITIALIZE_TPL(T*, GInstance, CreateInstance_());
        return *GInstance;
    }

private:
    static NO_INLINE T* CreateInstance_() {
        PPE_LEAKDETECTOR_WHITELIST_SCOPE();
        return new T();
    }
};
//----------------------------------------------------------------------------
template <typename T>
class TThreadLocalAutoSingleton : public FAbstractAutoSingleton {
protected:
    TThreadLocalAutoSingleton() { FThreadLocalAutoSingletonManager::Register(this); }
    virtual ~TThreadLocalAutoSingleton() { FThreadLocalAutoSingletonManager::Unregister(this); }

public:
    static T& Get() {
        ONE_TIME_INITIALIZE_THREAD_LOCAL_TPL(T*, GInstanceTLS, CreateInstance_());
        return *GInstanceTLS;
    }

private:
    static NO_INLINE T* CreateInstance_() {
        PPE_LEAKDETECTOR_WHITELIST_SCOPE();
        return new T();
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
