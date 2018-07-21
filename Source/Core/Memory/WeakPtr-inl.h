#pragma once

#include "Core/Memory/WeakPtr.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline FWeakAndRefCountable::FWeakAndRefCountable()
:   _weakPtrs(nullptr)
{}
//----------------------------------------------------------------------------
inline FWeakAndRefCountable::FWeakAndRefCountable(FWeakAndRefCountable&& )
:   _weakPtrs(nullptr)
{}
//----------------------------------------------------------------------------
inline FWeakAndRefCountable& FWeakAndRefCountable::operator =(FWeakAndRefCountable&& ) { return *this; }
//----------------------------------------------------------------------------
inline FWeakAndRefCountable::FWeakAndRefCountable(const FWeakAndRefCountable& )
:   _weakPtrs(nullptr)
{}
//----------------------------------------------------------------------------
inline FWeakAndRefCountable& FWeakAndRefCountable::operator =(const FWeakAndRefCountable& ) { return *this; }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline FWeakPtrBase::FWeakPtrBase()
:   _weakAndRefCountable(nullptr)
,   _next(nullptr)
,   _prev(nullptr)
{}
//----------------------------------------------------------------------------
inline FWeakPtrBase::~FWeakPtrBase() {
    set_(nullptr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
TWeakPtr<T>::TWeakPtr()
:   FWeakPtrBase((void **)&_ptr)
,   _ptr(nullptr) {}
//----------------------------------------------------------------------------
template <typename T>
TWeakPtr<T>::TWeakPtr(T* ptr)
:   FWeakPtrBase((void **)&_ptr)
,   _ptr(nullptr) {
    FWeakPtrBase::set_(ptr);
}
//----------------------------------------------------------------------------
template <typename T>
TWeakPtr<T>::~TWeakPtr() {
    FWeakPtrBase::set_<T>(nullptr);
}
//----------------------------------------------------------------------------
template <typename T>
TWeakPtr<T>::TWeakPtr(TWeakPtr&& rvalue)
:   TWeakPtr(rvalue._ptr) {
    rvalue.set_<T>(nullptr);
}
//----------------------------------------------------------------------------
template <typename T>
auto TWeakPtr<T>::operator =(TWeakPtr&& rvalue) -> TWeakPtr& {
    FWeakPtrBase::set_(rvalue._ptr);
    rvalue.set_<T>(nullptr);
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
TWeakPtr<T>::TWeakPtr(const TWeakPtr& other)
:   TWeakPtr(other._ptr) {}
//----------------------------------------------------------------------------
template <typename T>
auto TWeakPtr<T>::operator =(const TWeakPtr& other) -> TWeakPtr& {
    FWeakPtrBase::set_(other._ptr);
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
TWeakPtr<T>::TWeakPtr(const TWeakPtr<U>& other)
:   TWeakPtr(checked_cast<T *>(other.get())) {}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
auto TWeakPtr<T>::operator =(const TWeakPtr<U>& other) -> TWeakPtr& {
    FWeakPtrBase::set_(checked_cast<T *>(other.get()));
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
TWeakPtr<T>::TWeakPtr(TWeakPtr<U>&& rvalue)
:   TWeakPtr(checked_cast<T *>(rvalue.get())) {
    rvalue.set_<U>(nullptr);
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
auto TWeakPtr<T>::operator =(TWeakPtr<U>&& rvalue) -> TWeakPtr& {
    FWeakPtrBase::set_(checked_cast<T *>(rvalue.get()));
    rvalue.set_(nullptr);
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
void TWeakPtr<T>::reset(T* ptr/* = nullptr */) {
    FWeakPtrBase::set_(ptr);
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
bool TWeakPtr<T>::TryLock(TRefPtr<U> *pLocked) const {
    Assert(pLocked);
    Likely(pLocked);
    pLocked->reset(_ptr);
    return (pLocked->get() != nullptr);
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
void TWeakPtr<T>::Swap(TWeakPtr<U>& other) {
    T *const lhs = _ptr;
    U *const rhs = other._ptr;
    FWeakPtrBase::set_(checked_cast<T *>(rhs));
    other.set_(checked_cast<U *>(lhs));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
