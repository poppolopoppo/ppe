#pragma once

#include "Memory/RefPtr.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline FRefCountable::FRefCountable()
:   _refCount(0)
#if USE_PPE_SAFEPTR
,   _safeRefCount(0)
#endif
{}
//----------------------------------------------------------------------------
inline FRefCountable::~FRefCountable() {
    Assert(0 == _refCount);
#if USE_PPE_SAFEPTR
    // check if a TSafePtr<> is still holding a reference to this object
    AssertRelease(0 == _safeRefCount);
#endif
}
//----------------------------------------------------------------------------
inline FRefCountable::FRefCountable(FRefCountable&& )
:   _refCount(0)
#if USE_PPE_SAFEPTR
,   _safeRefCount(0)
#endif
{}
//----------------------------------------------------------------------------
inline FRefCountable& FRefCountable::operator =(FRefCountable&& ) { return *this; }
//----------------------------------------------------------------------------
inline FRefCountable::FRefCountable(const FRefCountable& )
:   _refCount(0)
#if USE_PPE_SAFEPTR
,   _safeRefCount(0)
#endif
{}
//----------------------------------------------------------------------------
inline FRefCountable& FRefCountable::operator =(const FRefCountable& ) { return *this; }
//----------------------------------------------------------------------------
inline void FRefCountable::IncRefCount() const {
    _refCount.fetch_add(1);
}
//----------------------------------------------------------------------------
inline bool FRefCountable::DecRefCount_ReturnIfReachZero() const {
    const int n = _refCount.fetch_sub(1);
    Assert(n > 0);
    return (1 == n);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline void AddRef(const FRefCountable* ptr) {
    ptr->IncRefCount();
}
//----------------------------------------------------------------------------
template <typename T>
void RemoveRef(T* ptr) {
    STATIC_ASSERT(std::is_base_of<FRefCountable, T>::value);
    if (ptr->DecRefCount_ReturnIfReachZero())
        OnRefCountReachZero(ptr);
}
//----------------------------------------------------------------------------
template <typename T>
T* RemoveRef_AssertAlive(TRefPtr<T>& ptr) {
    STATIC_ASSERT(std::is_base_of<FRefCountable, T>::value);
    Assert(1 < ptr->RefCount());
    T* alive = ptr.get();
    ptr.reset();
    Assert(0 < alive->RefCount());
    return alive;
}
//----------------------------------------------------------------------------
template <typename T>
T* RemoveRef_KeepAlive(TRefPtr<T>& ptr) {
    Assert(ptr);
    T *const result = ptr.get();
    result->IncRefCount();
    ptr.reset();
    result->DecRefCount_ReturnIfReachZero();
    return result;
}
//----------------------------------------------------------------------------
template <typename T>
void OnRefCountReachZero(T* ptr) {
    STATIC_ASSERT(std::is_base_of<FRefCountable, T>::value);
    checked_delete(ptr);
}
//----------------------------------------------------------------------------
template <typename T>
void RemoveRef_AssertReachZero_NoDelete(T *& ptr) {
    typedef char type_must_be_complete[sizeof(T) ? 1 : -1];
    (void) sizeof(type_must_be_complete);
    Assert(ptr);
    const bool result = ptr->DecRefCount_ReturnIfReachZero();
    Assert(result);
    ptr->~T();
    ptr = nullptr;
}
//----------------------------------------------------------------------------
template <typename T>
void RemoveRef_AssertReachZero(TRefPtr<T>& ptr) {
    STATIC_ASSERT(std::is_base_of<FRefCountable, T>::value);
    Assert(ptr);
    Assert(1 == ptr->RefCount());
    ptr.reset();
}
//----------------------------------------------------------------------------
template <typename T>
void RemoveRef_AssertGreaterThanZero(TRefPtr<T>& ptr) {
    STATIC_ASSERT(std::is_base_of<FRefCountable, T>::value);
    Assert(ptr);
    Assert(1 < ptr->RefCount());
    ptr.reset();
}
//----------------------------------------------------------------------------
template <typename T>
T *RemoveRef_AssertReachZero_KeepAlive(TRefPtr<T>& ptr) {
    Assert(ptr);
    Assert(1 == ptr->RefCount());
    T *const result = ptr.get();
    result->IncRefCount();
    ptr.reset();
    if (not result->DecRefCount_ReturnIfReachZero())
        AssertNotReached();
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_SAFEPTR
inline void AddSafeRef(const FRefCountable* ptr) {
    ptr->IncSafeRefCount();
}
inline void RemoveSafeRef(const FRefCountable* ptr) {
    ptr->DecSafeRefCount();
}
inline void FRefCountable::IncSafeRefCount() const {
    _safeRefCount.fetch_add(1);
}
inline void FRefCountable::DecSafeRefCount() const {
    Verify(_safeRefCount.fetch_sub(1) > 0);
}
#endif //!WITH_PPE_SAFEPTR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
TRefPtr<T>::TRefPtr()
:   _ptr(nullptr) {}
//----------------------------------------------------------------------------
template <typename T>
TRefPtr<T>::TRefPtr(T* ptr)
:   _ptr(ptr) {
    IncRefCountIFP();
}
//----------------------------------------------------------------------------
template <typename T>
TRefPtr<T>::~TRefPtr() {
    DecRefCountIFP();
}
//----------------------------------------------------------------------------
template <typename T>
TRefPtr<T>::TRefPtr(TRefPtr&& rvalue) : _ptr(rvalue._ptr) {
    rvalue._ptr = nullptr;
}
//----------------------------------------------------------------------------
template <typename T>
auto TRefPtr<T>::operator =(TRefPtr&& rvalue) -> TRefPtr& {
    DecRefCountIFP();
    _ptr = rvalue._ptr;
    rvalue._ptr = nullptr;
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
TRefPtr<T>::TRefPtr(const TRefPtr& other) : _ptr(other._ptr) {
    IncRefCountIFP();
}
//----------------------------------------------------------------------------
template <typename T>
auto TRefPtr<T>::operator =(const TRefPtr& other) -> TRefPtr& {
    if (other._ptr != _ptr) {
        DecRefCountIFP();
        _ptr = other._ptr;
        IncRefCountIFP();
    }
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
TRefPtr<T>::TRefPtr(const TRefPtr<U>& other)
:   _ptr(checked_cast<T *>(other.get())) {
    IncRefCountIFP();
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
auto TRefPtr<T>::operator =(const TRefPtr<U>& other) -> TRefPtr& {
    if (other.get() != _ptr) {
        DecRefCountIFP();
        _ptr = checked_cast<T *>(other.get());
        IncRefCountIFP();
    }
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
TRefPtr<T>::TRefPtr(TRefPtr<U>&& rvalue)
:   _ptr(checked_cast<T *>(rvalue.get())) {
    rvalue._ptr = nullptr;
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
auto TRefPtr<T>::operator =(TRefPtr<U>&& rvalue) -> TRefPtr& {
    DecRefCountIFP();
    _ptr = checked_cast<T *>(rvalue.get());
    rvalue._ptr = nullptr;
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
void TRefPtr<T>::reset(T* ptr/* = nullptr */) {
    if (ptr != _ptr) {
        DecRefCountIFP();
        _ptr = ptr;
        IncRefCountIFP();
    }
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
void TRefPtr<T>::Swap(TRefPtr<U>& other) {
    std::swap(other._ptr, _ptr);
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE void TRefPtr<T>::IncRefCountIFP() const {
    STATIC_ASSERT(std::is_base_of<FRefCountable, T>::value);
    if (_ptr)
        AddRef(_ptr);
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE void TRefPtr<T>::DecRefCountIFP() const {
    STATIC_ASSERT(std::is_base_of<FRefCountable, T>::value);
    if (_ptr)
        RemoveRef(_ptr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
TSafePtr<T>::TSafePtr()
:   _ptr(nullptr) {}
//----------------------------------------------------------------------------
template <typename T>
TSafePtr<T>::TSafePtr(T* ptr)
:   _ptr(ptr) {
#if USE_PPE_SAFEPTR
    IncRefCountIFP();
#endif
}
//----------------------------------------------------------------------------
template <typename T>
TSafePtr<T>::~TSafePtr() {
#if USE_PPE_SAFEPTR
    DecRefCountIFP();
#endif
}
//----------------------------------------------------------------------------
template <typename T>
TSafePtr<T>::TSafePtr(TSafePtr&& rvalue) : _ptr(rvalue._ptr) {
    rvalue._ptr = nullptr;
}
//----------------------------------------------------------------------------
template <typename T>
auto TSafePtr<T>::operator =(TSafePtr&& rvalue) -> TSafePtr& {
#if USE_PPE_SAFEPTR
    DecRefCountIFP();
#endif
    _ptr = rvalue._ptr;
    rvalue._ptr = nullptr;
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
TSafePtr<T>::TSafePtr(const TSafePtr& other) : _ptr(other._ptr) {
#if USE_PPE_SAFEPTR
    IncRefCountIFP();
#endif
}
//----------------------------------------------------------------------------
template <typename T>
auto TSafePtr<T>::operator =(const TSafePtr& other) -> TSafePtr& {
#if USE_PPE_SAFEPTR
    if (other._ptr != _ptr) {
        DecRefCountIFP();
        _ptr = other._ptr;
        IncRefCountIFP();
    }
#else
    _ptr = other._ptr;
#endif
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
TSafePtr<T>::TSafePtr(const TSafePtr<U>& other)
:   _ptr(checked_cast<T *>(other.get())) {
#if USE_PPE_SAFEPTR
    IncRefCountIFP();
#endif
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
auto TSafePtr<T>::operator =(const TSafePtr<U>& other) -> TSafePtr& {
#if USE_PPE_SAFEPTR
    if (other.get() != _ptr) {
        DecRefCountIFP();
        _ptr = checked_cast<T *>(other.get());
        IncRefCountIFP();
    }
#else
    _ptr = checked_cast<T *>(other.get());
#endif
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
TSafePtr<T>::TSafePtr(TSafePtr<U>&& rvalue)
:   _ptr(checked_cast<T *>(rvalue.get())) {
    rvalue._ptr = nullptr;
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
auto TSafePtr<T>::operator =(TSafePtr<U>&& rvalue) -> TSafePtr& {
#if USE_PPE_SAFEPTR
    DecRefCountIFP();
#endif
    _ptr = checked_cast<T *>(rvalue.get());
    rvalue._ptr = nullptr;
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
void TSafePtr<T>::reset(T* ptr/* = nullptr */) {
#if USE_PPE_SAFEPTR
    if (ptr != _ptr) {
        DecRefCountIFP();
        _ptr = ptr;
        IncRefCountIFP();
    }
#else
    _ptr = ptr;
#endif
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
void TSafePtr<T>::Swap(TSafePtr<U>& other) {
    std::swap(other._ptr, _ptr);
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE void TSafePtr<T>::IncRefCountIFP() const {
    STATIC_ASSERT(std::is_base_of<FRefCountable, T>::value);
#if USE_PPE_SAFEPTR
    if (_ptr)
        AddSafeRef(_ptr);
#endif
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE void TSafePtr<T>::DecRefCountIFP() const {
    STATIC_ASSERT(std::is_base_of<FRefCountable, T>::value);
#if USE_PPE_SAFEPTR
    if (_ptr)
        RemoveSafeRef(_ptr);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE