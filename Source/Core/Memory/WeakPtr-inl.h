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
inline WeakPtrBase::WeakPtrBase(const WeakAndRefCountable *ptr/* = nullptr */) 
:   _ptr(nullptr)
,   _next(nullptr)
,   _prev(nullptr) {
    set_(ptr);
}
//----------------------------------------------------------------------------
inline WeakPtrBase::~WeakPtrBase() {
    set_(nullptr);
}
//----------------------------------------------------------------------------
inline const WeakAndRefCountable *WeakPtrBase::get_() const {
    if (_ptr) THREADRESOURCE_CHECKACCESS(_ptr);
    return _ptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
WeakPtr<T>::WeakPtr() 
:   WeakPtrBase() {}
//----------------------------------------------------------------------------
template <typename T>
WeakPtr<T>::WeakPtr(T* ptr)
:   WeakPtrBase(ptr) {}
//----------------------------------------------------------------------------
template <typename T>
WeakPtr<T>::~WeakPtr() {}
//----------------------------------------------------------------------------
template <typename T>
WeakPtr<T>::WeakPtr(WeakPtr&& rvalue)
:   WeakPtrBase(rvalue.get_()) {
    rvalue.set_(nullptr);
}
//----------------------------------------------------------------------------
template <typename T>
auto WeakPtr<T>::operator =(WeakPtr&& rvalue) -> WeakPtr& {
    WeakPtrBase::set_(rvalue.get_());
    rvalue.set_(nullptr);
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
WeakPtr<T>::WeakPtr(const WeakPtr& other) 
:   WeakPtrBase(other.get_()) {}
//----------------------------------------------------------------------------
template <typename T>
auto WeakPtr<T>::operator =(const WeakPtr& other) -> WeakPtr& {
    WeakPtrBase::set_(other.get_());
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
WeakPtr<T>::WeakPtr(const WeakPtr<U>& other)
:   WeakPtrBase(checked_cast<T *>(other.get())) {}
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
:   WeakPtrBase(checked_cast<T *>(rvalue.get())) {
    rvalue.set_(nullptr);
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
auto WeakPtr<T>::operator =(WeakPtr<U>&& rvalue) -> WeakPtr& {
    WeakPtrBase::set_(checked_cast<T *>(rvalue.get()));
    rvalue.set_(nullptr);
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
void WeakPtr<T>::Swap(WeakPtr<U>& other) {
    T *const lhs = get();
    U *const rhs = get();
    WeakPtrBase::set_(rhs);
    other.set_(lhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
