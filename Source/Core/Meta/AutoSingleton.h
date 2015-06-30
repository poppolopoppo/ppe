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

#include <mutex>

#include "Core/Memory/MemoryDomain.h"
#include "Core/Meta/OneTimeInitialize.h"

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class AutoSingletonManager;
//----------------------------------------------------------------------------
class AutoSingletonBase {
protected:
    AutoSingletonBase();

private:
    friend class AutoSingletonManager;
    AutoSingletonBase *_pnext;

public:
    virtual ~AutoSingletonBase() {}

    AutoSingletonBase(AutoSingletonBase&&) = delete;
    AutoSingletonBase& operator =(AutoSingletonBase&&) = delete;

    AutoSingletonBase(const AutoSingletonBase&) = delete;
    AutoSingletonBase& operator =(const AutoSingletonBase&) = delete;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class AutoSingletonManager {
private:
    AutoSingletonManager();
    ~AutoSingletonManager();

    std::mutex _lock;
    AutoSingletonBase *_pinstances;

public:
    AutoSingletonManager(AutoSingletonManager&&) = delete;
    AutoSingletonManager& operator =(AutoSingletonManager&&) = delete;

    AutoSingletonManager(const AutoSingletonManager&) = delete;
    AutoSingletonManager& operator =(const AutoSingletonManager&) = delete;

    static void Start();
    static void Shutdown();

    static void Register(AutoSingletonBase *pinstance);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class AutoSingleton : public AutoSingletonBase {
private:
    AutoSingleton() {}

    struct AutoCreate_ {
        T *pInstance;
        AutoCreate_() : pInstance(new T()) {}

        AutoCreate_(AutoCreate_&&) = delete;
        AutoCreate_& operator =(AutoCreate_&&) = delete;

        AutoCreate_(const AutoCreate_&) = delete;
        AutoCreate_& operator =(const AutoCreate_&) = delete;
    };

protected:
    explicit AutoSingleton(T *pinstance) { 
        AssertRelease(this == pinstance);
        AutoSingletonManager::Register(pinstance);
    }

public:
    virtual ~AutoSingleton() {}

    static T& Instance() {
        ONE_TIME_DEFAULT_INITIALIZE(AutoCreate_, autocreate);
        return *autocreate.pInstance;
    }
};
//----------------------------------------------------------------------------
template <typename T>
class AutoSingletonThreadLocal : public AutoSingletonBase {
private:
    AutoSingletonThreadLocal() {}

    struct AutoCreate_ {
        AutoCreate_(T **ppinstance) { *ppinstance = new T(); }
    };

protected:
    explicit AutoSingletonThreadLocal(T *pinstance) { 
        AssertRelease(this == pinstance);
        AutoSingletonManager::Register(pinstance);
    }

public:
    virtual ~AutoSingletonThreadLocal() {}

    static T& Instance() {
        static THREAD_LOCAL T *spInstance = nullptr;
        ONE_TIME_INITIALIZE_THREAD_LOCAL(AutoCreate_, autocreate, &spInstance);
        return *spInstance;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
