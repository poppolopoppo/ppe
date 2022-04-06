#pragma once

#include "Memory/WeakPtr.h"

#include "Allocator/StaticAllocator.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_ASSERT
inline FWeakRefCountable::~FWeakRefCountable() {
#    if USE_PPE_SAFEPTR
    Assert_Lightweight(0 == _safeRefCount);
#    endif
    Assert_Lightweight(not _cnt || _cnt->RefCount() == 0);
}
#endif
//----------------------------------------------------------------------------
template <typename T, typename... _Args>
TRefPtr<T> FWeakRefCountable::NewRefImpl(void* p, deleter_func deleter, _Args&&... args) NOEXCEPT {
    STATIC_ASSERT(IsWeakRefCountable<T>::value);

    Assert(p);
    Assert(deleter);

    T* const x = new (p) T{ std::forward<_Args>(args)... };

    static_cast<FWeakRefCountable*>(x)->_cnt = FWeakRefCounter::Allocate(deleter ARGS_IF_ASSERT(x));

    return { x };
}
//----------------------------------------------------------------------------
// provide a custom allocator, deleter will be exported in the counter
template <typename T, typename _Allocator, typename... _Args>
TRefPtr< TEnableIfWeakRefCountable<T> > NewRef(_Args&&... args) {
    using deleter_func = typename FWeakRefCountable::deleter_func;
    return FWeakRefCountable::NewRefImpl<T>(
        TStaticAllocator<_Allocator>::template AllocateOneT<T>(),
        reinterpret_cast<deleter_func>(&TStaticAllocator<_Allocator>::template DeallocateOneT<T>),
        std::forward<_Args>(args)... );
}
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
template <typename T, typename... _Args>
TRefPtr< TEnableIfWeakRefCountable<T> > NewRef(FMemoryTracking& trackingData, _Args&&... args) {
    return FWeakRefCountable::NewRefImpl<T>(
        tracking_malloc(trackingData, sizeof(T)),
        &tracking_free,
        std::forward<_Args>(args)... );
}
#else
template <typename T, typename... _Args>
TRefPtr< TEnableIfWeakRefCountable<T> > NewRef(_Args&&... args) {
    return FWeakRefCountable::NewRefImpl<T>(
        PPE::malloc(sizeof(T)),
        [](void* p) NOEXCEPT { PPE::free(p); },
        std::forward<_Args>(args)... );
}
#endif
//----------------------------------------------------------------------------
inline void AddRef(const FWeakRefCountable* ptr) NOEXCEPT {
    Assert(ptr);
    ptr->IncStrongRefCount();
}
//----------------------------------------------------------------------------
inline void AddRefIFP(const FWeakRefCountable* ptr) NOEXCEPT {
    Assert(ptr);
    if (ptr->_cnt) {
        Assert_NoAssume(ptr->RefCount() > 0); // object must be already *locked* !
        AddRef(ptr->_cnt.get()); // also needs to keep the counter alive
        AddRef(ptr);
    }
}
//----------------------------------------------------------------------------
template <typename T>
void AddRefIFP(TRefPtr<T>& pRefPtr, TEnableIfWeakRefCountable<T>* ptr) NOEXCEPT {
    Assert(ptr);
    if (ptr->_cnt) {
        Assert_NoAssume(ptr->RefCount() > 0); // object must be already *locked* !
        pRefPtr.reset(ptr);
    }
}
//----------------------------------------------------------------------------
template <typename T>
void RemoveRefIFP(TEnableIfWeakRefCountable<T>* ptr) NOEXCEPT {
    Assert(ptr);
    if (ptr->_cnt) {
        Assert_NoAssume(ptr->_cnt->WeakRefCount() > 0); // counter must be already *locked* !
        Assert_NoAssume(ptr->_cnt->RefCount() > 0); // object must be already *locked* !
        RemoveRef(ptr->_cnt.get());
        RemoveRef(ptr);
    }
}
//----------------------------------------------------------------------------
template <typename T>
bool RemoveRefIFP_KeepAlive_ReturnIfReachZero(TEnableIfWeakRefCountable<T>* ptr) NOEXCEPT {
    Assert(ptr);
    if (Likely(ptr->_cnt)) {
        Assert_NoAssume(ptr->_cnt->WeakRefCount() > 0); // counter must be already *locked* !
        Assert_NoAssume(ptr->_cnt->RefCount() > 0); // object must be already *locked* !
        RemoveRef(ptr->_cnt.get());
        return ptr->DecStrongRefCount_ReturnIfReachZero();
    }
    return false;
}
//----------------------------------------------------------------------------
#if USE_PPE_SAFEPTR
inline void AddSafeRef(const FWeakRefCountable* ptr) NOEXCEPT {
    ptr->IncSafeRefCount();
}
inline void RemoveSafeRef(const FWeakRefCountable* ptr) NOEXCEPT {
    ptr->DecSafeRefCount();
}
inline void FWeakRefCountable::IncSafeRefCount() const NOEXCEPT {
    _safeRefCount.fetch_add(1, std::memory_order_relaxed);
}
inline void FWeakRefCountable::DecSafeRefCount() const NOEXCEPT {
    Verify(_safeRefCount.fetch_sub(1, std::memory_order_relaxed) > 0);
}
#endif //!WITH_PPE_SAFEPTR
//----------------------------------------------------------------------------
template <typename T>
NO_INLINE TEnableIfWeakRefCountable<T, void> OnStrongRefCountReachZero(T* ptr) NOEXCEPT {
    Assert(ptr);
    Assert_NoAssume(0 == ptr->RefCount());
#if USE_PPE_SAFEPTR
    Assert_Lightweight(0 == ptr->SafeRefCount());
#endif

    // retrieve internal deleter *before* object destruction (don't AddRef() the counter)
    using deleter_func = typename FWeakRefCountable::deleter_func;
    const deleter_func deleter = static_cast<FWeakRefCountable*>(ptr)->Deleter_Unsafe();

    Meta::Destroy(ptr);

    deleter(ptr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
template <typename U, class>
TWeakPtr<T>::TWeakPtr(const TRefPtr<U>& refptr) NOEXCEPT
:   _ptr(refptr.get())
,   _cnt(_ptr ? _ptr->_cnt : nullptr)
{}
//----------------------------------------------------------------------------
template <typename T>
void TWeakPtr<T>::reset() NOEXCEPT {
    _ptr = nullptr;
    _cnt.reset();
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U, class>
void TWeakPtr<T>::reset(const TRefPtr<U>& refptr) NOEXCEPT {
    _ptr = refptr.get();
    _cnt.reset(_ptr ? _ptr->_cnt.get() : nullptr);
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U, class>
bool TWeakPtr<T>::TryLock(TRefPtr<U> *pLocked) const NOEXCEPT {
    Assert(pLocked);

    if (Likely(!!_ptr & !!_cnt)) {
        Assert_NoAssume(_cnt->_holderForDebug == _ptr);
        if (_cnt->TryLockForWeakPtr()) {
            Assert_NoAssume(_cnt == _ptr->_cnt);
            pLocked->reset(_ptr);
            // release the lock, guaranteed to not delete since we just added a ref
            Verify(not _cnt->Weak_DecStrongRefCount_ReturnIfReachZero());
            return true;
        }
    }

    pLocked->reset();
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
