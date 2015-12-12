#pragma once

#include "Core/Memory/WeakPtr.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline WeakAndRefCountable::WeakAndRefCountable()
:   _weakPtrs(nullptr)
{}
//----------------------------------------------------------------------------
inline WeakAndRefCountable::WeakAndRefCountable(WeakAndRefCountable&& )
:   _weakPtrs(nullptr)
{}
//----------------------------------------------------------------------------
inline WeakAndRefCountable& WeakAndRefCountable::operator =(WeakAndRefCountable&& ) { return *this; }
//----------------------------------------------------------------------------
inline WeakAndRefCountable::WeakAndRefCountable(const WeakAndRefCountable& )
:   _weakPtrs(nullptr)
{}
//----------------------------------------------------------------------------
inline WeakAndRefCountable& WeakAndRefCountable::operator =(const WeakAndRefCountable& ) { return *this; }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline WeakPtrBase::WeakPtrBase(void **pptr)
:   _pptr(pptr)
,   _next(nullptr)
,   _prev(nullptr) {
    Assert(_pptr);
}
//----------------------------------------------------------------------------
inline WeakPtrBase::~WeakPtrBase() {}
//----------------------------------------------------------------------------
template <typename T>
void WeakPtrBase::set_(T *ptr) {
    STATIC_ASSERT(std::is_base_of<WeakAndRefCountable COMMA T>::value);

    if (*_pptr == (void *)ptr)
        return;

    if (*_pptr) {
        T *const pptrAsT = (T *)*_pptr;
        THREADRESOURCE_CHECKACCESS(pptrAsT);

        if (_prev) _prev->_next = _next;
        if (_next) _next->_prev = _prev;

        if (this == pptrAsT->_weakPtrs) {
            Assert(nullptr == _prev);
            pptrAsT->_weakPtrs = _next;
        }

        _prev = _next = nullptr;
        *_pptr = nullptr;
    }

    Assert(nullptr == *_pptr);
    Assert(nullptr == _next);
    Assert(nullptr == _prev);

    if (ptr) {
        THREADRESOURCE_CHECKACCESS(ptr);

        _next = ptr->_weakPtrs;
        if (ptr->_weakPtrs)
            ptr->_weakPtrs->_prev = this;

        ptr->_weakPtrs = this;
        *_pptr = (void *)ptr;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
WeakPtr<T>::WeakPtr()
:   WeakPtrBase((void **)&_ptr)
,   _ptr(nullptr) {}
//----------------------------------------------------------------------------
template <typename T>
WeakPtr<T>::WeakPtr(T* ptr)
:   WeakPtrBase((void **)&_ptr)
,   _ptr(nullptr) {
    WeakPtrBase::set_(ptr);
}
//----------------------------------------------------------------------------
template <typename T>
WeakPtr<T>::~WeakPtr() {
    WeakPtrBase::set_<T>(nullptr);
}
//----------------------------------------------------------------------------
template <typename T>
WeakPtr<T>::WeakPtr(WeakPtr&& rvalue)
:   WeakPtr(rvalue._ptr) {
    rvalue.set_<T>(nullptr);
}
//----------------------------------------------------------------------------
template <typename T>
auto WeakPtr<T>::operator =(WeakPtr&& rvalue) -> WeakPtr& {
    WeakPtrBase::set_(rvalue._ptr);
    rvalue.set_<T>(nullptr);
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
WeakPtr<T>::WeakPtr(const WeakPtr& other)
:   WeakPtr(other._ptr) {}
//----------------------------------------------------------------------------
template <typename T>
auto WeakPtr<T>::operator =(const WeakPtr& other) -> WeakPtr& {
    WeakPtrBase::set_(other._ptr);
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
WeakPtr<T>::WeakPtr(const WeakPtr<U>& other)
:   WeakPtr(checked_cast<T *>(other.get())) {}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
auto WeakPtr<T>::operator =(const WeakPtr<U>& other) -> WeakPtr& {
    WeakPtrBase::set_(checked_cast<T *>(other.get()));
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
WeakPtr<T>::WeakPtr(WeakPtr<U>&& rvalue)
:   WeakPtr(checked_cast<T *>(rvalue.get())) {
    rvalue.set_<U>(nullptr);
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
auto WeakPtr<T>::operator =(WeakPtr<U>&& rvalue) -> WeakPtr& {
    WeakPtrBase::set_(checked_cast<T *>(rvalue.get()));
    rvalue.set_<U>(nullptr);
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
void WeakPtr<T>::reset(T* ptr/* = nullptr */) {
    WeakPtrBase::set_(ptr);
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
bool WeakPtr<T>::TryLock(RefPtr<U> *pLocked) const {
    Assert(pLocked);
    Likely(pLocked);
    pLocked->reset(_ptr);
    return nullptr != pLocked->get();
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
void WeakPtr<T>::Swap(WeakPtr<U>& other) {
    T *const lhs = get();
    U *const rhs = get();
    WeakPtrBase::set_(checked_cast<T *>(rhs));
    other.set_(checked_cast<U *>(lhs));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
