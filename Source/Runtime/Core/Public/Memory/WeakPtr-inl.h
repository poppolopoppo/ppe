#pragma once

#include "Memory/WeakPtr.h"

#include "Allocator/StaticAllocator.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline FWeakRefCountable::FWeakRefCountable()
:   _cnt(nullptr)
{}
//----------------------------------------------------------------------------
inline FWeakRefCountable::~FWeakRefCountable() {
    Assert_NoAssume(not _cnt or _cnt->RefCount() == 0);
    Assert_NoAssume(not _cnt or _cnt->SafeRefCount() == 0);
}
//----------------------------------------------------------------------------
inline void AddRef(const FWeakRefCountable* ptr) {
    ptr->IncStrongRefCount();
}
//----------------------------------------------------------------------------
#if USE_PPE_SAFEPTR
inline void AddSafeRef(const FWeakRefCountable* ptr) NOEXCEPT {
    ptr->IncSafeRefCount();
}
inline void RemoveSafeRef(const FWeakRefCountable* ptr) NOEXCEPT {
    ptr->DecSafeRefCount();
}
#endif //!WITH_PPE_SAFEPTR
//----------------------------------------------------------------------------
template <typename T, typename... _Args>
TRefPtr<T> FWeakRefCountable::NewRefImpl(void* p, deleter_func deleter, _Args&&... args) NOEXCEPT {
    STATIC_ASSERT(IsWeakRefCountable<T>::value);

    Assert(p);
    Assert(deleter);

    T* const x = new (p) T{ std::forward<_Args>(args)... };

    static_cast<FWeakRefCountable*>(x)->_cnt = FWeakRefCounter::Allocate(
#if USE_PPE_ASSERT
        x,
#endif
        deleter );

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
        std::forward<_Args>(args) );
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
        &PPE::free,
        std::forward<_Args>(args)... );
}
#endif
//----------------------------------------------------------------------------
template <typename T>
NO_INLINE void OnStrongRefCountReachZero(TEnableIfWeakRefCountable<T>* ptr) NOEXCEPT {
    Assert_NoAssume(0 == ptr->RefCount());
#if USE_PPE_SAFEPTR
    Assert_NoAssume(0 == ptr->SafeRefCount());
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
        Assert_NoAssume(_cnt->_holder == _ptr);
        // try to increment the counter if > 0, atomically
        if (_cnt->TryLockForWeakPtr()) {
            Assert_NoAssume(_cnt.get() == _ptr->_cnt.get());
            pLocked->reset(_ptr);
            // release the lock, guaranteed to not delete since we just added a ref
            Verify(not _cnt->Weak_DecStrongRefCount_ReturnIfReachZero());
            return true;
        }
        // cleanup the counter ref if failed to lock, to keeping it alive too long
        else {
            // /!\ don't cleanup _ptr, since we don't want to invalidate the
            // hash value or relation order (potentially used in containers)
            _cnt.reset();
        }
    }

    pLocked->reset();
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
