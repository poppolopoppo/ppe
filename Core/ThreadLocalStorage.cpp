#include "stdafx.h"

#include "Core.h"
#include "ThreadLocalStorage.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ThreadLocalStorage::ThreadLocalStorage(std::thread::id threadId)
:   _threadId(threadId) {
    Assert(std::this_thread::get_id() == _threadId);
}
//----------------------------------------------------------------------------
ThreadLocalStorage::~ThreadLocalStorage() {
    Assert(std::this_thread::get_id() == _threadId);
    Assert(_slots.empty());
    Assert(_storage.empty());
}
//----------------------------------------------------------------------------
auto ThreadLocalStorage::SlotByName(const char* key) const -> Slots::iterator {
    Assert(std::this_thread::get_id() == _threadId);
    Assert(key);
    return std::find_if(_slots.begin(), _slots.end(), [=](const KeyValue& kv) {
        return 0 == std::strcmp(kv.first, key);
    });
}
//----------------------------------------------------------------------------
void ThreadLocalStorage::DestroySlot(IThreadLocalSlot* slot) {
    Assert(std::this_thread::get_id() == _threadId);
    const Slots::iterator it = std::find_if(_slots.begin(), _slots.end(), [=](const KeyValue& kv) {
        return (kv.second == slot);
    });
    Assert(_slots.end() != it);
    it->second->Destroy(_storage);
    if (it + 1 != _slots.end())
        std::swap(*it, *(_slots.end() - 1));
    _slots.resize(_slots.size() - 1);
}
//----------------------------------------------------------------------------
void ThreadLocalStorage::DestroyAllSlots_MainThread_() {
    Assert(_slots.empty());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ThreadLocalManagerImpl::ThreadLocalManagerImpl() {
    _threads.reserve(std::thread::hardware_concurrency()*4);
}
//----------------------------------------------------------------------------
ThreadLocalManagerImpl::~ThreadLocalManagerImpl() {
    Assert(_threads.empty());
}
//----------------------------------------------------------------------------
void ThreadLocalManagerImpl::CreateThreadLocalStorage() {
    const std::thread::id threadId = std::this_thread::get_id();
    const std::lock_guard<std::mutex> scope_lock(_barrier);
    Assert(_threads.end() == _threads.find(threadId));
    _threads[threadId] = new ThreadLocalStorage(threadId);
}
//----------------------------------------------------------------------------
auto ThreadLocalManagerImpl::GetThreadLocalStorage() -> ThreadLocalStorage* {
    const std::thread::id threadId = std::this_thread::get_id();
    const std::lock_guard<std::mutex> scope_lock(_barrier);
    const auto it = _threads.find(threadId);
    Assert(_threads.end() != it);
    Assert(it->second);
    return it->second;
}
//----------------------------------------------------------------------------
void ThreadLocalManagerImpl::DestroyThreadLocalStorage() {
    const std::thread::id threadId = std::this_thread::get_id();
    const std::lock_guard<std::mutex> scope_lock(_barrier);
    const auto it = _threads.find(threadId);
    Assert(_threads.end() != it);
    Assert(it->second);
    it->second->DestroyAllSlots_MainThread_();
    checked_delete(it->second);
    _threads.erase(it);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
