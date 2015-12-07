#pragma once

#include "Core/Core.h"

/*
//
// AutoSingleton
// -------------
// Enables lazy instantiation of a singleton.
// This is only allowed when we can't know the type in advance (like in memory pools).
// Use AutoSingletonManager to kill all auto singletons.
//
*/

#include "Core/Meta/OneTimeInitialize.h"

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class AbstractAutoSingleton {
protected:
    AbstractAutoSingleton() : _pnext(nullptr), _pprev(nullptr) {}

public:
    virtual ~AbstractAutoSingleton() { // must be virtual to allow delete()
        Assert(nullptr == _pnext);
        Assert(nullptr == _pprev);
    }

    AbstractAutoSingleton(AbstractAutoSingleton&&) = delete;
    AbstractAutoSingleton& operator =(AbstractAutoSingleton&&) = delete;

    AbstractAutoSingleton(const AbstractAutoSingleton&) = delete;
    AbstractAutoSingleton& operator =(const AbstractAutoSingleton&) = delete;

private:
    friend class AutoSingletonManagerImpl;
    AbstractAutoSingleton *_pnext;
    AbstractAutoSingleton *_pprev;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class AutoSingletonManager {
public:
    static void Start();
    static void Shutdown();

    static void Register(AbstractAutoSingleton *pinstance);
    static void Unregister(AbstractAutoSingleton *pinstance);
};
//----------------------------------------------------------------------------
class ThreadLocalAutoSingletonManager {
public:
    static void Start();
    static void Shutdown();

    static void Register(AbstractAutoSingleton *pinstance);
    static void Unregister(AbstractAutoSingleton *pinstance);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class AutoSingleton : public AbstractAutoSingleton {
protected:
    AutoSingleton() { AutoSingletonManager::Register(this); }
    virtual ~AutoSingleton() { AutoSingletonManager::Unregister(this); }

public:
    static T& Instance() {
        ONE_TIME_INITIALIZE_TPL(T *, sInstance, new T() );
        return *sInstance;
    }
};
//----------------------------------------------------------------------------
template <typename T>
class ThreadLocalAutoSingleton : public AbstractAutoSingleton {
protected:
    ThreadLocalAutoSingleton() { ThreadLocalAutoSingletonManager::Register(this); }
    virtual ~ThreadLocalAutoSingleton() { ThreadLocalAutoSingletonManager::Unregister(this); }

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
