#pragma once

#include "Core.h"

#include <thread>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
class TMRUCache {
public:
    static_assert(0 == ((_Dim - 1) & _Dim), "_Dim must be a power of 2");
    enum : size_t { Dim = _Dim, Mask = _Dim - 1 };

    TMRUCache();
    ~TMRUCache();

    bool empty() const { return _headPos == _tailPos; }

    bool Get_ReturnIfEmpty(T **pacquire);
    bool Release_ReturnIfFull(T **prelease);
    void Clear_AssumeCacheDestroyed();

private:
    size_t _headPos;
    size_t _tailPos;

    T *_cache[_Dim];

#ifdef WITH_PPE_ASSERT
    std::thread::id _threadID;
    void CheckThreadOwnerShip_() const {
        Assert(std::this_thread::get_id() == _threadID);
    }
#else
    void CheckThreadOwnerShip_() const {}
#endif
};
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TMRUCache<T, _Dim>::TMRUCache()
:   _headPos(0)
,   _tailPos(0)
#ifdef WITH_PPE_ASSERT
,   _threadID(std::this_thread::get_id())
#endif
{
#ifdef WITH_PPE_ASSERT
    for (size_t i = 0; i < _Dim; ++i)
        _cache[i] = nullptr;
#endif
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TMRUCache<T, _Dim>::~TMRUCache() {
    Assert(empty());
    CheckThreadOwnerShip_();
#ifdef WITH_PPE_ASSERT
    for (size_t i = 0; i < _Dim; ++i)
        Assert(nullptr == _cache[i]);
#endif
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool TMRUCache<T, _Dim>::Get_ReturnIfEmpty(T **pacquire) {
    Assert(pacquire);
    Assert(!*pacquire);
    CheckThreadOwnerShip_();

    if (_headPos != _tailPos) {
        --_headPos;
        *pacquire = _cache[_headPos & Mask];
        Assert(*pacquire);
#ifdef WITH_PPE_ASSERT
        _cache[_headPos & Mask] = nullptr;
#endif
        return false;
    }

    return true;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool TMRUCache<T, _Dim>::Release_ReturnIfFull(T **prelease) {
    Assert(prelease);
    Assert(*prelease);
    CheckThreadOwnerShip_();
    Assert(_tailPos <= _headPos);

    if (_headPos - _tailPos == _Dim) {
        T *const toRelease = _cache[_tailPos++ & Mask];
        _cache[_headPos++ & Mask] = *prelease;
        *prelease = toRelease;
        Assert(*prelease);
        return true;
    }
    else {
        Assert(nullptr == _cache[_headPos & Mask]);
        _cache[_headPos++ & Mask] = *prelease;
        *prelease = nullptr;
        return false;
    }
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
void TMRUCache<T, _Dim>::Clear_AssumeCacheDestroyed() {
    CheckThreadOwnerShip_();

#ifdef WITH_PPE_ASSERT
    for (size_t i = 0; i < _Dim; ++i)
        Assert(nullptr == _cache[i]);
#endif

    _tailPos = _headPos = 0;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
