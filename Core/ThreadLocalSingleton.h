#pragma once

#include "Core.h"
#include "ThreadLocalStorage.h"

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Tag>
class ThreadLocalSingleton
{
public:
    static T& Instance() { Assert(HasInstance()); return _gTLS_Slot->Instance(); }
    static bool HasInstance() { return _gTLS_Slot && _gTLS_Slot->HasInstance(); }

    template <typename... _Args>
    static void Create(const char* key, _Args&&... args);
    static void Destroy();

    ThreadLocalSingleton(const ThreadLocalSingleton&) = delete;
    ThreadLocalSingleton& operator =(const ThreadLocalSingleton&) = delete;

protected:
    ThreadLocalSingleton() {}
    ~ThreadLocalSingleton() {}

private:
    static THREAD_LOCAL ThreadLocalSlot<T>* _gTLS_Slot;
};
//----------------------------------------------------------------------------
template <typename T, typename _Tag>
THREAD_LOCAL ThreadLocalSlot<T>* ThreadLocalSingleton<T, _Tag>::_gTLS_Slot = nullptr;
//----------------------------------------------------------------------------
template <typename T, typename _Tag>
template <typename... _Args>
void ThreadLocalSingleton<T, _Tag>::Create(const char* key, _Args&&... args) {
    Assert(nullptr == _gTLS_Slot);
    _gTLS_Slot = ThreadLocalManager::Instance().GetThreadLocalStorage()->CreateSlot<T>(key);
    Assert(_gTLS_Slot);
    _gTLS_Slot->Create(std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename T, typename _Tag>
void ThreadLocalSingleton<T, _Tag>::Destroy() {
    Assert(nullptr != _gTLS_Slot);
    ThreadLocalManager::Instance().GetThreadLocalStorage()->DestroySlot(_gTLS_Slot);
    _gTLS_Slot = nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
