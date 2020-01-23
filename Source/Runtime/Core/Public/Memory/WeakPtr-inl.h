#pragma once

#include "Memory/WeakPtr.h"

namespace PPE {
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
TWeakPtr<T>::TWeakPtr(T* ptr)
:   FWeakPtrBase() {
    FWeakPtrBase::set_(ptr);
}
//----------------------------------------------------------------------------
template <typename T>
TWeakPtr<T>::~TWeakPtr() {
    STATIC_ASSERT(std::is_base_of_v<FWeakAndRefCountable, Meta::TDecay<T>>);
    FWeakPtrBase::set_(nullptr);
}
//----------------------------------------------------------------------------
template <typename T>
TWeakPtr<T>::TWeakPtr(TWeakPtr&& rvalue) {
    FWeakPtrBase::swap_(rvalue);
}
//----------------------------------------------------------------------------
template <typename T>
auto TWeakPtr<T>::operator =(TWeakPtr&& rvalue) -> TWeakPtr& {
    FWeakPtrBase::set_(nullptr);
    FWeakPtrBase::swap_(rvalue);
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
TWeakPtr<T>::TWeakPtr(const TWeakPtr& other)
:   TWeakPtr(other.template get_<T>()) {}
//----------------------------------------------------------------------------
template <typename T>
auto TWeakPtr<T>::operator =(const TWeakPtr& other) -> TWeakPtr& {
    FWeakPtrBase::set_(other.template get_<T>());
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
TWeakPtr<T>::TWeakPtr(const TWeakPtr<U>& other)
:   TWeakPtr(other.template get_<T>()) {}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
auto TWeakPtr<T>::operator =(const TWeakPtr<U>& other) -> TWeakPtr& {
    FWeakPtrBase::set_(other.template get_<T>());
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
TWeakPtr<T>::TWeakPtr(TWeakPtr<U>&& rvalue) {
    FWeakPtrBase::set_(rvalue.template get_<T>());
    rvalue.set_(nullptr);
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
auto TWeakPtr<T>::operator =(TWeakPtr<U>&& rvalue) -> TWeakPtr& {
    FWeakPtrBase::set_(rvalue.template get_<T>());
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
    U* const pRef = this->template get_<U>();
    pLocked->reset(pRef);
    return (!!pRef);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
