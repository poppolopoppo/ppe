#pragma once

#include "Memory/RefPtr.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline FRefCountable::FRefCountable() NOEXCEPT
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
    Assert_Lightweight(0 == _safeRefCount);
#endif
}
//----------------------------------------------------------------------------
inline FRefCountable::FRefCountable(FRefCountable&& ) NOEXCEPT
:   _refCount(0)
#if USE_PPE_SAFEPTR
,   _safeRefCount(0)
#endif
{}
//----------------------------------------------------------------------------
inline FRefCountable& FRefCountable::operator =(FRefCountable&& ) NOEXCEPT {
    return (*this);
}
//----------------------------------------------------------------------------
inline FRefCountable::FRefCountable(const FRefCountable& ) NOEXCEPT
:   _refCount(0)
#if USE_PPE_SAFEPTR
,   _safeRefCount(0)
#endif
{}
//----------------------------------------------------------------------------
inline FRefCountable& FRefCountable::operator =(const FRefCountable& ) NOEXCEPT {
    return (*this);
}
//----------------------------------------------------------------------------
inline void FRefCountable::IncStrongRefCount() const NOEXCEPT {
    Assert(_refCount >= 0);
    _refCount.fetch_add(1, std::memory_order_relaxed);
}
//----------------------------------------------------------------------------
inline bool FRefCountable::DecStrongRefCount_ReturnIfReachZero() const NOEXCEPT {
    Assert(_refCount > 0);
    const int n = atomic_fetch_sub_explicit(&_refCount, 1, std::memory_order_release);
    if (1 == n) {
        std::atomic_thread_fence(std::memory_order_acquire);
        return true;
    }
    else {
        Assert_NoAssume(n > 0);
        return false;
    }
}
//----------------------------------------------------------------------------
// /!\ Should **ALWAYS** allocate with NewRef<>() helper !
inline void FRefCountable::operator delete(void* p) {
#if USE_PPE_MEMORYDOMAINS
    tracking_free(p);
#else
    PPE::free(p);
#endif
}
//----------------------------------------------------------------------------
template <typename T, typename... _Args>
#if USE_PPE_MEMORYDOMAINS
TRefPtr< TEnableIfRefCountable<T> > NewRef(FMemoryTracking& trackingData, _Args&&... args) {
    return TRefPtr<T>{ new (tracking_malloc(trackingData, sizeof(T))) T{ std::forward<_Args>(args)... } };
}
#else
TRefPtr< TEnableIfRefCountable<T> > NewRef(_Args&&... args) {
    return TRefPtr<T>{ new (PPE::malloc(sizeof(T))) T{ std::forward<_Args>(args)... } };
}
#endif
//----------------------------------------------------------------------------
inline void AddRef(const FRefCountable* ptr) NOEXCEPT {
    Assert(ptr);
    ptr->IncStrongRefCount();
}
//----------------------------------------------------------------------------
template <typename T>
NO_INLINE TEnableIfRefCountable<T, void> OnStrongRefCountReachZero(T* ptr) NOEXCEPT {
    Assert(ptr);
    Assert_NoAssume(0 == ptr->RefCount());
#if USE_PPE_SAFEPTR
    Assert_Lightweight(0 == ptr->SafeRefCount());
#endif

    checked_delete(ptr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
void RemoveRef(T* ptr) {
    if (ptr->DecStrongRefCount_ReturnIfReachZero()) {
        OnStrongRefCountReachZero(ptr);
    }
}
//----------------------------------------------------------------------------
template <typename T>
T* RemoveRef_AssertAlive(TRefPtr<T>& refptr) {
    Assert(1 < refptr->RefCount());
    T* alive = refptr.get();
    refptr.reset();
    Assert_NoAssume(0 < alive->RefCount());
    return alive;
}
//----------------------------------------------------------------------------
template <typename T>
T* RemoveRef_KeepAlive(TRefPtr<T>& refptr) {
    Assert(refptr);
    T* const result = refptr.get();
    result->IncStrongRefCount();
    refptr.reset();
    result->DecStrongRefCount_ReturnIfReachZero();
    return result;
}
//----------------------------------------------------------------------------
template <typename T>
void RemoveRef_AssertReachZero_NoDelete(T* ptr) {
    typedef char type_must_be_complete[sizeof(T) ? 1 : -1];
    (void) sizeof(type_must_be_complete);
    Assert(ptr);
    VerifyRelease(ptr->DecStrongRefCount_ReturnIfReachZero());
}
//----------------------------------------------------------------------------
template <typename T>
void RemoveRef_AssertReachZero(TRefPtr<T>& refptr) {
    STATIC_ASSERT(IsRefCountable<T>::value);
    Assert(refptr);
    Assert_NoAssume(1 == refptr->RefCount());
    refptr.reset();
}
//----------------------------------------------------------------------------
template <typename T>
T* RemoveRef_AssertReachZero_KeepAlive(TRefPtr<T>& refptr) {
    Assert(refptr);
    Assert(1 == refptr->RefCount());
    T* const result = refptr.get();
    result->IncStrongRefCount();
    refptr.reset();
    VerifyRelease(result->DecStrongRefCount_ReturnIfReachZero());
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_SAFEPTR
inline void AddSafeRef(const FRefCountable* ptr) NOEXCEPT {
    ptr->IncSafeRefCount();
}
inline void RemoveSafeRef(const FRefCountable* ptr) NOEXCEPT {
    ptr->DecSafeRefCount();
}
inline void FRefCountable::IncSafeRefCount() const NOEXCEPT {
    _safeRefCount.fetch_add(1, std::memory_order_relaxed);
}
inline void FRefCountable::DecSafeRefCount() const NOEXCEPT {
    Verify(_safeRefCount.fetch_sub(1, std::memory_order_release) > 0);
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
    IncRefCountIFP(_ptr);
}
//----------------------------------------------------------------------------
template <typename T>
TRefPtr<T>::~TRefPtr() {
    DecRefCountIFP(_ptr);
}
//----------------------------------------------------------------------------
template <typename T>
TRefPtr<T>::TRefPtr(TRefPtr&& rvalue) NOEXCEPT : _ptr(rvalue._ptr) {
    rvalue._ptr = nullptr;
}
//----------------------------------------------------------------------------
template <typename T>
auto TRefPtr<T>::operator =(TRefPtr&& rvalue) NOEXCEPT -> TRefPtr& {
    DecRefCountIFP(_ptr);
    _ptr = rvalue._ptr;
    rvalue._ptr = nullptr;
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
TRefPtr<T>::TRefPtr(const TRefPtr& other) : _ptr(other._ptr) {
    IncRefCountIFP(_ptr);
}
//----------------------------------------------------------------------------
template <typename T>
auto TRefPtr<T>::operator =(const TRefPtr& other) -> TRefPtr& {
    if (other._ptr != _ptr) {
        DecRefCountIFP(_ptr);
        IncRefCountIFP(other._ptr);

        _ptr = other._ptr;
    }
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
TRefPtr<T>::TRefPtr(const TRefPtr<U>& other)
:   _ptr(checked_cast<T *>(other.get())) {
    IncRefCountIFP(_ptr);
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
auto TRefPtr<T>::operator =(const TRefPtr<U>& other) -> TRefPtr& {
    T* const otherT = checked_cast<T*>(other.get());
    if (otherT != _ptr) {
        DecRefCountIFP(_ptr);
        IncRefCountIFP(otherT);

        _ptr = otherT;
    }
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
TRefPtr<T>::TRefPtr(TRefPtr<U>&& rvalue) NOEXCEPT
:   _ptr(checked_cast<T*>(rvalue.get())) {
    rvalue._ptr = nullptr;
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
auto TRefPtr<T>::operator =(TRefPtr<U>&& rvalue) NOEXCEPT -> TRefPtr& {
    DecRefCountIFP(_ptr);
    _ptr = checked_cast<T *>(rvalue.get());
    rvalue._ptr = nullptr;
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
void TRefPtr<T>::reset(T* ptr/* = nullptr */) NOEXCEPT {
    if (ptr != _ptr) {
        DecRefCountIFP(_ptr);
        IncRefCountIFP(ptr);

        _ptr = ptr;
    }
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
void TRefPtr<T>::Swap(TRefPtr<U>& other) NOEXCEPT {
    std::swap(other._ptr, _ptr);
}
//----------------------------------------------------------------------------
template <typename T>
void TRefPtr<T>::IncRefCountIFP(T* ptr) NOEXCEPT {
    if (ptr)
        AddRef(ptr);
}
//----------------------------------------------------------------------------
template <typename T>
void TRefPtr<T>::DecRefCountIFP(T* ptr) NOEXCEPT {
    if (ptr)
        RemoveRef(ptr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
TSafePtr<T>::TSafePtr() NOEXCEPT
:   _ptr(nullptr) {}
//----------------------------------------------------------------------------
template <typename T>
TSafePtr<T>::TSafePtr(T* ptr) NOEXCEPT
:   _ptr(ptr) {
#if USE_PPE_SAFEPTR
    IncRefCountIFP(ptr);
#endif
}
//----------------------------------------------------------------------------
template <typename T>
TSafePtr<T>::~TSafePtr() {
#if USE_PPE_SAFEPTR
    DecRefCountIFP(_ptr);
#endif
}
//----------------------------------------------------------------------------
template <typename T>
TSafePtr<T>::TSafePtr(TSafePtr&& rvalue) NOEXCEPT : _ptr(rvalue._ptr) {
    rvalue._ptr = nullptr;
}
//----------------------------------------------------------------------------
template <typename T>
auto TSafePtr<T>::operator =(TSafePtr&& rvalue) NOEXCEPT -> TSafePtr& {
#if USE_PPE_SAFEPTR
    DecRefCountIFP(_ptr);
#endif
    _ptr = rvalue._ptr;
    rvalue._ptr = nullptr;
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
TSafePtr<T>::TSafePtr(const TSafePtr& other) NOEXCEPT : _ptr(other._ptr) {
#if USE_PPE_SAFEPTR
    IncRefCountIFP(_ptr);
#endif
}
//----------------------------------------------------------------------------
template <typename T>
auto TSafePtr<T>::operator =(const TSafePtr& other) NOEXCEPT -> TSafePtr& {
#if USE_PPE_SAFEPTR
    if (other._ptr != _ptr) {
        DecRefCountIFP(_ptr);
        IncRefCountIFP(other._ptr);

        _ptr = other._ptr;
    }
#else
    _ptr = other._ptr;
#endif
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
TSafePtr<T>::TSafePtr(const TSafePtr<U>& other) NOEXCEPT
:   _ptr(checked_cast<T *>(other.get())) {
#if USE_PPE_SAFEPTR
    IncRefCountIFP(_ptr);
#endif
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
auto TSafePtr<T>::operator =(const TSafePtr<U>& other) NOEXCEPT -> TSafePtr& {
#if USE_PPE_SAFEPTR
    T* const otherT = checked_cast<T*>(other.get());
    if (otherT != _ptr) {
        DecRefCountIFP(_ptr);
        IncRefCountIFP(otherT);

        _ptr = otherT;
    }
#else
    _ptr = checked_cast<T *>(other.get());
#endif
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
TSafePtr<T>::TSafePtr(TSafePtr<U>&& rvalue) NOEXCEPT
:   _ptr(checked_cast<T *>(rvalue.get())) {
    rvalue._ptr = nullptr;
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
auto TSafePtr<T>::operator =(TSafePtr<U>&& rvalue) NOEXCEPT -> TSafePtr& {
#if USE_PPE_SAFEPTR
    DecRefCountIFP(_ptr);
#endif
    _ptr = checked_cast<T *>(rvalue.get());
    rvalue._ptr = nullptr;
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
void TSafePtr<T>::reset(T* ptr/* = nullptr */) NOEXCEPT {
#if USE_PPE_SAFEPTR
    if (ptr != _ptr) {
        DecRefCountIFP(_ptr);
        IncRefCountIFP(ptr);

        _ptr = ptr;
    }
#else
    _ptr = ptr;
#endif
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
void TSafePtr<T>::Swap(TSafePtr<U>& other) NOEXCEPT {
    std::swap(other._ptr, _ptr);
}
//----------------------------------------------------------------------------
#if USE_PPE_SAFEPTR
template <typename T>
void TSafePtr<T>::IncRefCountIFP(T* ptr) NOEXCEPT {
    if (ptr)
        AddSafeRef(ptr);
}
#endif //!USE_PPE_SAFEPTR
//----------------------------------------------------------------------------
#if USE_PPE_SAFEPTR
template <typename T>
void TSafePtr<T>::DecRefCountIFP(T* ptr) NOEXCEPT {
    if (ptr)
        RemoveSafeRef(ptr);
}
#endif //!USE_PPE_SAFEPTR
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
