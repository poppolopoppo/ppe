#pragma once

#include "Core.h"

#include "Activator.h"
#include "AlignedStorage.h"

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Tag>
class Singleton
{
public:
    static T& Instance();
    static bool HasInstance();

    template <typename... _Args>
    static void Create(_Args&&... args);
    static void Destroy();

protected:
    Singleton() {}
    ~Singleton() {}

private:
    static typename POD_STORAGE(T) _gStorage;
    static T *_gInstancePtr;
};
//----------------------------------------------------------------------------
template <typename T, typename _Tag>
typename POD_STORAGE(T) Singleton<T, _Tag>::_gStorage;
//----------------------------------------------------------------------------
template <typename T, typename _Tag>
T *Singleton<T, _Tag>::_gInstancePtr = nullptr;
//----------------------------------------------------------------------------
template <typename T, typename _Tag>
T& Singleton<T, _Tag>::Instance() {
    Assert(_gInstancePtr);
    return *_gInstancePtr;
}
//----------------------------------------------------------------------------
template <typename T, typename _Tag>
bool Singleton<T, _Tag>::HasInstance() {
    return (nullptr != _gInstancePtr);
}
//----------------------------------------------------------------------------
template <typename T, typename _Tag>
template <typename... _Args>
void Singleton<T, _Tag>::Create(_Args&&... args) {
    AssertRelease(!_gInstancePtr);
    _gInstancePtr = reinterpret_cast<T *>(&_gStorage);
    Activator<T>::Construct(_gInstancePtr, std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename T, typename _Tag>
void Singleton<T, _Tag>::Destroy() {
    AssertRelease(_gInstancePtr);
    Activator<T>::Destroy(_gInstancePtr);
    _gInstancePtr = nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
