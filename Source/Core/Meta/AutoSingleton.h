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

#include "Core/Container/Vector.h"
#include "Core/Memory/MemoryDomain.h"
#include "Core/Meta/OneTimeInitialize.h"

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class AutoSingletonBase {
protected:
    AutoSingletonBase() {}

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
    VECTOR(Singleton, AutoSingletonBase *) _instances;

public:
    AutoSingletonManager(AutoSingletonManager&&) = delete;
    AutoSingletonManager& operator =(AutoSingletonManager&&) = delete;

    AutoSingletonManager(const AutoSingletonManager&) = delete;
    AutoSingletonManager& operator =(const AutoSingletonManager&) = delete;

    static void Start();
    static void Shutdown();

    static void Register(AutoSingletonBase *instance);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class AutoSingleton : public AutoSingletonBase {
private:
    AutoSingleton() {}

    struct AutoCreate_ {
        T *Instance;
        AutoCreate_() : Instance(new T()) {}

        AutoCreate_(AutoCreate_&&) = delete;
        AutoCreate_& operator =(AutoCreate_&&) = delete;

        AutoCreate_(const AutoCreate_&) = delete;
        AutoCreate_& operator =(const AutoCreate_&) = delete;
    };

protected:
    explicit AutoSingleton(T *instance) { AutoSingletonManager::Register(this); }

public:
    virtual ~AutoSingleton() {}

    static T& Instance() {
        ONE_TIME_DEFAULT_INITIALIZE(AutoCreate_, autocreate);
        return *autocreate.Instance;
    }
};
//----------------------------------------------------------------------------
template <typename T>
class AutoSingletonThreadLocal : public AutoSingletonBase {
private:
    AutoSingletonThreadLocal() {}

    struct AutoCreate_ {
        AutoCreate_(T **instance) { *instance = new T(); }
    };

protected:
    explicit AutoSingletonThreadLocal(T *instance) { AutoSingletonManager::Register(this); }

public:
    virtual ~AutoSingletonThreadLocal() {}

    static T& Instance() {
        static THREAD_LOCAL T *instance = nullptr;
        ONE_TIME_INITIALIZE_THREAD_LOCAL(AutoCreate_, autocreate, &instance);
        return *instance;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
