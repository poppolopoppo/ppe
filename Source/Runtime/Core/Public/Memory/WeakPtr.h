#pragma once

#include "Core_fwd.h"

#include "Allocator/AllocatorBase.h"
#include "Memory/RefPtr.h"
#include "Meta/ThreadResource.h"

#define _FWD_WEAKPTR_IMPL(T, _PREFIX)                                   \
    _FWD_REFPTR_IMPL(T, _PREFIX);                                       \
    typedef PPE::TWeakPtr<CONCAT(_PREFIX, T)>           CONCAT(W,  T);  \
    typedef PPE::TWeakPtr<const CONCAT(_PREFIX, T)>     CONCAT(WC, T)

#define FWD_WEAKPTR(T_WITHOUT_F)            _FWD_WEAKPTR_IMPL(T_WITHOUT_F, F)
#define FWD_INTERFACE_WEAKPTR(T_WITHOUT_I)  _FWD_WEAKPTR_IMPL(T_WITHOUT_I, I)

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TWeakPtr;
//----------------------------------------------------------------------------
FWD_REFPTR(WeakRefCounter);
//----------------------------------------------------------------------------
FWD_WEAKPTR(WeakRefCountable);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FWeakRefCounter : public FRefCountable, Meta::FNonCopyableNorMovable {
public:
#if USE_PPE_ASSERT
    ~FWeakRefCounter() {
        Assert_NoAssume(0 == RefCount());
        Assert_NoAssume(0 == WeakRefCount());
    }
#endif

    NODISCARD int RefCount() const { return _strongRefCount; }
    NODISCARD int WeakRefCount() const { return FRefCountable::RefCount(); }

    // custom deleter can be used with FWeakRefCountable (!= FRefCountable)
    typedef void (*deleter_func)(void*) NOEXCEPT;

    NODISCARD static PPE_CORE_API PWeakRefCounter Allocate(deleter_func deleter ARGS_IF_ASSERT(FWeakRefCountable* holderForDebug));

private:
    template <typename T>
    friend class TWeakPtr;
    friend class FWeakRefCountable;

    // NewRef() should be the only way to allocate this object !
    template <typename T, typename... _Args>
#if USE_PPE_MEMORYDOMAINS
    friend TRefPtr< TEnableIfRefCountable<T> > NewRef(FMemoryTracking& trackingData, _Args&&... args);
#else
    friend TRefPtr< TEnableIfRefCountable<T> > NewRef(_Args&&... args);
#endif

    explicit FWeakRefCounter(deleter_func deleter ARGS_IF_ASSERT(FWeakRefCountable* holderForDebug)) NOEXCEPT
    :   _deleter(deleter)
#if USE_PPE_ASSERT
    ,   _holderForDebug(holderForDebug)
#endif
    ,   _strongRefCount(0) {
        Assert(_deleter);
        Assert_NoAssume(_holderForDebug);
    }

    NODISCARD PPE_CORE_API bool TryLockForWeakPtr() NOEXCEPT;

    void Weak_IncWeakRefCount() const NOEXCEPT {
        FRefCountable::IncStrongRefCount();
    }
    NODISCARD bool Weak_DecWeakRefCount_ReturnIfReachZero() const NOEXCEPT {
        return FRefCountable::DecStrongRefCount_ReturnIfReachZero();
    }

    void Weak_IncStrongRefCount() const NOEXCEPT {
        Assert(_strongRefCount >= 0);
        _strongRefCount.fetch_add(1, std::memory_order_relaxed);
    }
    NODISCARD bool Weak_DecStrongRefCount_ReturnIfReachZero() const NOEXCEPT {
        Assert(_strongRefCount > 0);
        const int n = atomic_fetch_sub_explicit(&_strongRefCount, 1, std::memory_order_release);
        if (1 == n) {
            std::atomic_thread_fence(std::memory_order_acquire);
            return true;
        }
        else {
            Assert(n > 0);
            return false;
        }
    }

    deleter_func _deleter;
#if USE_PPE_ASSERT
    FWeakRefCountable* const _holderForDebug;
#endif
    mutable std::atomic<int> _strongRefCount;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
using IsWeakRefCountable = std::is_base_of<FWeakRefCountable, Meta::TDecay<T> >;
template <typename T, typename _Result = T>
using TEnableIfWeakRefCountable = Meta::TEnableIf< IsWeakRefCountable<T>::value, _Result >;
//----------------------------------------------------------------------------
class PPE_CORE_API FWeakRefCountable {
public:
    FWeakRefCountable() = default;

#if USE_PPE_ASSERT
    ~FWeakRefCountable();
#endif

    FWeakRefCountable(FWeakRefCountable&& ) NOEXCEPT : FWeakRefCountable() {}
    FWeakRefCountable& operator =(FWeakRefCountable&& ) NOEXCEPT { return (*this); }

    FWeakRefCountable(const FWeakRefCountable& ) : FWeakRefCountable() {}
    FWeakRefCountable& operator =(const FWeakRefCountable&) { return (*this); }

    NODISCARD int RefCount() const { return _cnt->RefCount(); }
    NODISCARD int WeakRefCount() const { return _cnt->WeakRefCount(); }

#if USE_PPE_SAFEPTR
    NODISCARD int SafeRefCount() const { return _safeRefCount; }
#endif

private: // override new/delete operators for custom allocation schemes
    using deleter_func = FWeakRefCounter::deleter_func;

    template <typename T, typename... _Args>
    NODISCARD static TRefPtr<T> NewRefImpl(void* p, deleter_func deleter, _Args&&... args) NOEXCEPT;

public:
    #if 0 // messing with virtual destructor in child classes...
    // general allocators are forbidden to force the client to provide metadata
    static void* operator new(std::size_t) = delete;
    static void* operator new[](std::size_t) = delete;

    static void operator delete(void* p) = delete; // always use the deleter stored inside the counter
    static void operator delete(void* p, std::size_t size) = delete; // always use the deleter stored inside the counter

    // still allows inplace new
    static void* operator new(std::size_t, void* p) { return p; }
    static void operator delete(void*, void*) { }

    static void* operator new[](std::size_t, void* p) { return p; }
    static void operator delete[](void*, void*) {}
    #endif

    // provide a custom allocator, deleter will be exported in the counter
    template <typename T, typename _Allocator, typename... _Args>
    friend TRefPtr< TEnableIfWeakRefCountable<T> > NewRef(_Args&&... args);

    // provide memory tracking, correct deleter is deduced
    template <typename T, typename... _Args>
#if USE_PPE_MEMORYDOMAINS
    friend TRefPtr< TEnableIfWeakRefCountable<T> > NewRef(FMemoryTracking& trackingData, _Args&&... args);
#else
    friend TRefPtr< TEnableIfWeakRefCountable<T> > NewRef(_Args&&... args);
#endif

protected:
    friend void AddRef(const FWeakRefCountable* ptr) NOEXCEPT;
    template <typename T>
    friend TEnableIfWeakRefCountable<T, void> OnStrongRefCountReachZero(T* ptr) NOEXCEPT;

    // manipulate if ref count, only if *already* ref-counted
    friend void AddRefIFP(const FWeakRefCountable* ptr) NOEXCEPT;
    template <typename T>
    friend void AddRefIFP(TRefPtr<T>& pRefPtr, TEnableIfWeakRefCountable<T>* ptr) NOEXCEPT;
    template <typename T>
    friend void RemoveRefIFP(TEnableIfWeakRefCountable<T>* ptr) NOEXCEPT;
    template <typename T>
    friend bool RemoveRefIFP_KeepAlive_ReturnIfReachZero(TEnableIfWeakRefCountable<T>* ptr) NOEXCEPT;

    template <typename T>
    friend void RemoveRef(T* ptr);
    template <typename T>
    friend T* RemoveRef_AssertAlive(TRefPtr<T>& refptr);
    template <typename T>
    friend T* RemoveRef_KeepAlive(TRefPtr<T>& refptr);
    template <typename T>
    friend void RemoveRef_AssertReachZero_NoDelete(T* ptr);
    template <typename T>
    friend void RemoveRef_AssertReachZero(TRefPtr<T>& refptr);
    template <typename T>
    friend T* RemoveRef_AssertReachZero_KeepAlive(TRefPtr<T>& refptr);

#if USE_PPE_SAFEPTR
    void IncSafeRefCount() const NOEXCEPT;
    void DecSafeRefCount() const NOEXCEPT;

    friend void AddSafeRef(const FWeakRefCountable* ptr) NOEXCEPT;
    friend void RemoveSafeRef(const FWeakRefCountable* ptr) NOEXCEPT;
#else
    FORCE_INLINE friend void AddSafeRef(const FWeakRefCountable*) NOEXCEPT {}
    FORCE_INLINE friend void RemoveSafeRef(const FWeakRefCountable*) NOEXCEPT {}
#endif

private:
    template <typename T>
    friend class TWeakPtr;

    NODISCARD deleter_func Deleter_Unsafe() const {
        Assert_NoAssume(_cnt->_holderForDebug == this);
        return _cnt->_deleter;
    }

    // forward ref counting to internal counter
    void IncWeakRefCount() const NOEXCEPT { _cnt->Weak_IncWeakRefCount(); }
    NODISCARD bool DecWeakRefCount_ReturnIfReachZero() const NOEXCEPT { return _cnt->Weak_DecWeakRefCount_ReturnIfReachZero(); }

    void IncStrongRefCount() const NOEXCEPT { _cnt->Weak_IncStrongRefCount(); }
    NODISCARD bool DecStrongRefCount_ReturnIfReachZero() const NOEXCEPT { return _cnt->Weak_DecStrongRefCount_ReturnIfReachZero(); }

    mutable PWeakRefCounter _cnt;

#if USE_PPE_SAFEPTR
    // for debugging purpose : assert if TSafePtr<> are still tracking that object
    template <typename T>
    friend class TSafePtr;
    mutable std::atomic<int> _safeRefCount{ 0 };
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TWeakPtr {
public:
    template <typename U>
    friend class TWeakPtr;

    TWeakPtr() NOEXCEPT : _ptr(nullptr) {}

    template <typename U, class = Meta::TEnableIf<std::is_assignable_v<T*&, U*>> >
    explicit TWeakPtr(const TRefPtr<U>& refptr) NOEXCEPT;

    void reset() NOEXCEPT;
    template <typename U, class = Meta::TEnableIf<std::is_assignable_v<T*&, U*>> >
    void reset(const TRefPtr<U>& refptr) NOEXCEPT;

    template <typename U, class = Meta::TEnableIf<std::is_assignable_v<T*&, U*>> >
    NODISCARD bool TryLock(TRefPtr<U>* pLocked) const NOEXCEPT;

    NODISCARD TRefPtr<T> Pin() const NOEXCEPT {
        TRefPtr<T> pinned;
        Unused(TryLock(&pinned));
        return pinned;
    }

public:
    NODISCARD friend hash_t hash_value(const TWeakPtr<T>& weakPtr) NOEXCEPT {
        return hash_ptr(weakPtr._ptr);
    }

    template <typename U, class _CommonPtr = std::common_type_t<T*, U*> >
    NODISCARD friend bool operator ==(const TWeakPtr& lhs, const TWeakPtr<U>& rhs) NOEXCEPT {
        return (static_cast<_CommonPtr>(lhs._ptr) == static_cast<_CommonPtr>(rhs._ptr));
    }
    template <typename U, class _CommonPtr = std::common_type_t<T*, U*> >
    NODISCARD friend bool operator !=(const TWeakPtr& lhs, const TWeakPtr<U>& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }

    template <typename U, class _CommonPtr = std::common_type_t<T*, U*> >
    NODISCARD friend bool operator <(const TWeakPtr& lhs, const TWeakPtr<U>& rhs) NOEXCEPT {
        return (static_cast<_CommonPtr>(lhs._ptr) < static_cast<_CommonPtr>(rhs._ptr));
    }
    template <typename U, class _CommonPtr = std::common_type_t<T*, U*> >
    NODISCARD friend bool operator >=(const TWeakPtr& lhs, const TWeakPtr<U>& rhs) NOEXCEPT {
        return (not operator <(lhs, rhs));
    }

    template <typename U, class _CommonPtr = std::common_type_t<T*, U*> >
    friend void swap(const TWeakPtr& lhs, const TWeakPtr<U>& rhs) {
        using std::swap;
        swap(lhs._ptr, rhs._ptr);
        swap(lhs._cnt, rhs._cnt);
    }

private:
    T* _ptr;
    PWeakRefCounter _cnt;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Memory/WeakPtr-inl.h"
