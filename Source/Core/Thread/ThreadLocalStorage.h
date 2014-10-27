#pragma once

#include "Core/Core.h"

#include "Core/Container/HashMap.h"
#include "Core/Container/Pair.h"
#include "Core/Memory/AlignedStorage.h"
#include "Core/Memory/MemoryStack.h"
#include "Core/Meta/Activator.h"
#include "Core/Meta/Singleton.h"

#include <mutex>
#include <thread>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IThreadLocalSlot {
public:
    virtual ~IThreadLocalSlot() {}
    virtual void Destroy(StackAllocatorMemory& stack) = 0;
};
//----------------------------------------------------------------------------
template<typename T>
class ThreadLocalSlot : public IThreadLocalSlot {
public:
    ThreadLocalSlot() : _instance(nullptr) {}
    virtual ~ThreadLocalSlot() { Assert(!_instance); }

    ThreadLocalSlot(const ThreadLocalSlot&) = delete;
    ThreadLocalSlot& operator =(const ThreadLocalSlot&) = delete;

    T& Instance() const { Assert(_instance); return *_instance; }
    bool HasInstance() const { return (nullptr != _instance); }

    template <typename... _Args>
    void Create(_Args&&... args);
    virtual void Destroy(StackAllocatorMemory& stack) override;

private:
    T *_instance;
    typename POD_STORAGE(T) _storage;
};
//----------------------------------------------------------------------------
template<typename T>
template <typename... _Args>
void ThreadLocalSlot<T>::Create(_Args&&... args) {
    AssertRelease(nullptr == _instance);
    _instance = reinterpret_cast<T *>(&_storage);
    Meta::Activator<T>::Construct(_instance, std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template<typename T>
void ThreadLocalSlot<T>::Destroy(StackAllocatorMemory& stack) {
    AssertRelease(nullptr != _instance);
    Meta::Activator<T>::Destroy(_instance);
    _instance = nullptr;

    stack.Deallocate(reinterpret_cast<uint8_t*>(this), sizeof(*this));
    this->~ThreadLocalSlot();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ThreadLocalStorage {
    friend class ThreadLocalManagerImpl;
public:
    typedef Pair<const char*, IThreadLocalSlot*> KeyValue;

    enum {
        StorageCapacity = 62 << 10 /* 62 ko */
    ,   SlotCapacity = (1 << 10) / sizeof(KeyValue) /* 1 ko */
    ,   Alignment = 16
    };

    typedef StaticStack<uint8_t, StorageCapacity, Alignment> Storage;
    typedef StaticStack<KeyValue, SlotCapacity, Alignment> Slots;

public:
    // this object must be only accessed by one thread,
    // except on destruction where ThreadLocalManagerImpl will call DestroyAllSlots_MainThread_()
    explicit ThreadLocalStorage(std::thread::id threadId);
    ~ThreadLocalStorage();

    ThreadLocalStorage(const ThreadLocalStorage&) = delete;
    ThreadLocalStorage& operator =(const ThreadLocalStorage&) = delete;

    std::thread::id ThreadId() const { return _threadId; }

    template <typename T>
    ThreadLocalSlot<T>* CreateSlot(const char* key);

    Slots::iterator SlotByName(const char* key) const;
    void DestroySlot(IThreadLocalSlot* slot);

private:
    void DestroyAllSlots_MainThread_();

    const std::thread::id _threadId;
    Slots _slots;
    Storage _storage;
};
//----------------------------------------------------------------------------
static_assert(sizeof(ThreadLocalStorage) < (64 << 10), "ThreadStorage is bigger than a large page");
//----------------------------------------------------------------------------
template <typename T>
ThreadLocalSlot<T>* ThreadLocalStorage::CreateSlot(const char* key) {
    Assert(std::this_thread::get_id() == _threadId);
    typedef ThreadLocalSlot<T> holder_type;
    holder_type* const instance = new ((void*)_storage.Allocate(sizeof(holder_type))) holder_type();
    new ((void*)_slots.Allocate(1)) KeyValue(key, instance);
    return instance;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ThreadLocalManagerImpl {
protected:
    friend struct Meta::Activator<ThreadLocalManagerImpl>;
    ThreadLocalManagerImpl();
    ~ThreadLocalManagerImpl();

public:
    void CreateThreadLocalStorage();
    ThreadLocalStorage* GetThreadLocalStorage();
    void DestroyThreadLocalStorage();

private:
    std::mutex _barrier;
    HASHMAP(ThreadLocal, std::thread::id, ThreadLocalStorage*) _threads;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ThreadLocalManager : Meta::Singleton<ThreadLocalManagerImpl, ThreadLocalManager> {
    typedef Meta::Singleton<ThreadLocalManagerImpl, ThreadLocalManager> parent_type;
public:
    using parent_type::HasInstance;
    using parent_type::Instance;
    using parent_type::Destroy;

    static void Create() {
        parent_type::Create();
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
