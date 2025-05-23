#pragma once

#include "Core_fwd.h"

#include "Memory/MemoryDomain.h"

#include <type_traits>

/*
// Intrusive ref counting.
// Prefered to non intrusive ref counting (std::shared_ptr<>).
*/

#define USE_PPE_SAFEPTR (USE_PPE_DEBUG || USE_PPE_MEMORY_DEBUGGING)

#define PPE_REFPTR_ALIASES(T, _PREFIX)                                       \
    typedef ::PPE::TRefPtr<CONCAT(_PREFIX, T)>           CONCAT(P,  T); \
    typedef ::PPE::TRefPtr<const CONCAT(_PREFIX, T)>     CONCAT(PC, T); \
    typedef ::PPE::TSafePtr<CONCAT(_PREFIX, T)>          CONCAT(S,  T); \
    typedef ::PPE::TSafePtr<const CONCAT(_PREFIX, T)>    CONCAT(SC, T)
#define _FWD_REFPTR_IMPL(T, _PREFIX)                                    \
    class CONCAT(_PREFIX, T);                                           \
    PPE_REFPTR_ALIASES(T, _PREFIX)

#define FWD_REFPTR(T_WITHOUT_F)             _FWD_REFPTR_IMPL(T_WITHOUT_F, F)
#define FWD_INTERFACE_REFPTR(T_WITHOUT_I)   _FWD_REFPTR_IMPL(T_WITHOUT_I, I)

// used for tracking memory allocated by instances derived from FRefCountable
#if USE_PPE_MEMORYDOMAINS
#   include "Allocator/TrackingMalloc.h"
#   if PPE_VA_OPT_SUPPORTED
#       define NEW_REF(_DOMAIN, T, ...) PPE::NewRef< T >( MEMORYDOMAIN_TRACKING_DATA(_DOMAIN) __VA_OPT__(,) __VA_ARGS__ )
#   else
#       define NEW_REF(_DOMAIN, T, ...) PPE::NewRef< T >( MEMORYDOMAIN_TRACKING_DATA(_DOMAIN) ,## __VA_ARGS__ )
#   endif
#else
#   include "Allocator/Malloc.h"
#   define NEW_REF(_DOMAIN, T, ...) PPE::NewRef< T >( __VA_ARGS__ )
#endif

#ifdef SAFEPTR
#   undef SAFEPTR
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TRefPtr;
//----------------------------------------------------------------------------
template <typename T>
class TSafePtr;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FRefCountable;
//----------------------------------------------------------------------------
template <typename T>
using IsRefCountable = std::is_base_of<FRefCountable, Meta::TDecay<T> >;
template <typename T, typename _Result = T>
using TEnableIfRefCountable = Meta::TEnableIf< IsRefCountable<T>::value, _Result >;
//----------------------------------------------------------------------------
class PPE_CORE_API FRefCountable {
public:
    FRefCountable() NOEXCEPT;
    ~FRefCountable();

    FRefCountable(FRefCountable&& ) NOEXCEPT;
    FRefCountable& operator =(FRefCountable&& ) NOEXCEPT;

    FRefCountable(const FRefCountable& ) NOEXCEPT;
    FRefCountable& operator =(const FRefCountable& ) NOEXCEPT;

    int RefCount() const { return _refCount; }

#if USE_PPE_SAFEPTR
    int SafeRefCount() const { return _safeRefCount; }
#endif

public: // override new/delete operators for memory tracking
    // general allocators are forbidden to force the client to provide metadata
    static void* operator new(std::size_t) = delete;
    static void* operator new[](std::size_t) = delete;

    static void operator delete(void* p, std::size_t size);

    // provide memory tracking, don't support custom allocator (see FWeakRefCountable for that)
    template <typename T, typename... _Args>

#if USE_PPE_MEMORYDOMAINS
    friend TRefPtr< TEnableIfRefCountable<T> > NewRef(FMemoryTracking& trackingData, _Args&&... args);
#else
    friend TRefPtr< TEnableIfRefCountable<T> > NewRef(_Args&&... args);
#endif

    // still allows for placement new though
    static void* operator new(std::size_t, void* p) { return p; }
    static void operator delete(void*, void*) { }

    static void* operator new[](std::size_t, void* p) { return p; }
    static void operator delete[](void*, void*) {}

protected:
    friend void AddRef(const FRefCountable* ptr) NOEXCEPT;

    template <typename T>
    friend TEnableIfRefCountable<T, void> OnStrongRefCountReachZero(T* ptr) NOEXCEPT;

    template <typename T>
    using THasOnStrongRefCountReachZero = decltype(std::declval<T*>()->OnStrongRefCountReachZero());

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

    void IncStrongRefCount() const NOEXCEPT;
    NODISCARD bool DecStrongRefCount_ReturnIfReachZero() const NOEXCEPT;

#if USE_PPE_SAFEPTR
    void IncSafeRefCount() const NOEXCEPT;
    void DecSafeRefCount() const NOEXCEPT;

    friend void AddSafeRef(const FRefCountable* ptr) NOEXCEPT;
    friend void RemoveSafeRef(const FRefCountable* ptr) NOEXCEPT;
#else
    FORCE_INLINE friend void AddSafeRef(const FRefCountable*) NOEXCEPT {}
    FORCE_INLINE friend void RemoveSafeRef(const FRefCountable*) NOEXCEPT {}
#endif

private:
    mutable std::atomic<int> _refCount;

#if USE_PPE_SAFEPTR
    // for debugging purpose : assert if TSafePtr<> are still tracking that object
    template <typename T>
    friend class TSafePtr;
    mutable std::atomic<int> _safeRefCount;
#endif
};
//----------------------------------------------------------------------------
// Create an intrusive ref counted wrapper for any T
template <typename T>
class TRefCountable final : public FRefCountable, public T {
    STATIC_ASSERT(not std::is_base_of_v<FRefCountable, T>);
public:
    using T::T;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TRefPtr {
public:
    typedef T value_type;

    template <typename U>
    friend class TRefPtr;

    TRefPtr();
    TRefPtr(T* ptr);
    ~TRefPtr();

    TRefPtr(TRefPtr&& rvalue) NOEXCEPT;
    TRefPtr& operator =(TRefPtr&& rvalue) NOEXCEPT;

    TRefPtr(const TRefPtr& other);
    TRefPtr& operator =(const TRefPtr& other);

    template <typename U, class = Meta::TEnableIf<std::is_assignable_v<T*&, U*>> >
    TRefPtr(const TRefPtr<U>& other);

    template <typename U, class = Meta::TEnableIf<std::is_assignable_v<T*&, U*>> >
    TRefPtr& operator =(const TRefPtr<U>& other);

    template <typename U, class = Meta::TEnableIf<std::is_assignable_v<T*&, U*>> >
    TRefPtr(TRefPtr<U>&& rvalue) NOEXCEPT;
    template <typename U, class = Meta::TEnableIf<std::is_assignable_v<T*&, U*>> >
    TRefPtr& operator =(TRefPtr<U>&& rvalue) NOEXCEPT;

    NODISCARD FORCE_INLINE T* get() const { return _ptr; }
    void reset(T* ptr = nullptr) NOEXCEPT;

    template <typename U, class = Meta::TEnableIf<std::is_assignable_v<T*&, U*>> >
    NODISCARD U *as() const { return checked_cast<U*>(_ptr); }

    T& operator *() const { Assert(_ptr); return *_ptr; }
    T* operator ->() const { Assert(_ptr); return _ptr; }

    NODISCARD bool valid() const { return (!!_ptr); }
    PPE_FAKEBOOL_OPERATOR_DECL() { return valid(); }

    template <typename U, class = Meta::TEnableIf<std::is_assignable_v<T*&, U*>> >
    void Swap(TRefPtr<U>& other) NOEXCEPT;

    friend void swap(TRefPtr& lhs, TRefPtr& rhs) NOEXCEPT {
        lhs.Swap(rhs);
    }

protected:
    static void IncRefCountIFP(T* ptr) NOEXCEPT;
    static void DecRefCountIFP(T* ptr) NOEXCEPT;

private:
    T* _ptr;
};
STATIC_ASSERT(sizeof(TRefPtr<FRefCountable>) == sizeof(FRefCountable*));
//----------------------------------------------------------------------------
template <typename T> struct IsRefPtr : public std::false_type {};
template <typename T> struct IsRefPtr<TRefPtr<T>> : public std::true_type {};
//----------------------------------------------------------------------------
template <typename T>
NODISCARD hash_t hash_value(const TRefPtr<T>& refPtr) NOEXCEPT {
    return hash_value(refPtr.get());
}
//----------------------------------------------------------------------------
template <typename _Dst, typename _Src>
NODISCARD TRefPtr<_Dst> checked_cast(const TRefPtr<_Src>& refptr) NOEXCEPT {
    return { refptr.template as<_Dst>() };
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, class _CommonPtr = std::common_type_t<_Lhs*, _Rhs*> >
NODISCARD bool operator ==(const TRefPtr<_Lhs>& lhs, const TRefPtr<_Rhs>& rhs) NOEXCEPT {
    return (static_cast<_CommonPtr>(lhs.get()) == static_cast<_CommonPtr>(rhs.get()) );
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, class _CommonPtr = std::common_type_t<_Lhs*, _Rhs*> >
NODISCARD bool operator !=(const TRefPtr<_Lhs>& lhs, const TRefPtr<_Rhs>& rhs) NOEXCEPT {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, class _CommonPtr = std::common_type_t<_Lhs*, _Rhs*> >
NODISCARD bool operator ==(const TRefPtr<_Lhs>& lhs, _Rhs* rhs) NOEXCEPT {
    return (static_cast<_CommonPtr>(lhs.get()) == static_cast<_CommonPtr>(rhs) );
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, class _CommonPtr = std::common_type_t<_Lhs*, _Rhs*> >
NODISCARD bool operator !=(const TRefPtr<_Lhs>& lhs, _Rhs* rhs) NOEXCEPT {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, class _CommonPtr = std::common_type_t<_Lhs*, _Rhs*> >
NODISCARD bool operator ==(_Lhs* lhs, const TRefPtr<_Rhs>& rhs) NOEXCEPT {
    return (static_cast<_CommonPtr>(lhs) == static_cast<_CommonPtr>(rhs.get()) );
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, class _CommonPtr = std::common_type_t<_Lhs*, _Rhs*> >
NODISCARD bool operator !=(_Lhs* lhs, const TRefPtr<_Rhs>& rhs) NOEXCEPT {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, class _CommonPtr = std::common_type_t<_Lhs*, _Rhs*> >
NODISCARD bool operator <(const TRefPtr<_Lhs>& lhs, const TRefPtr<_Rhs>& rhs) {
    return (static_cast<_CommonPtr>(lhs.get()) < static_cast<_CommonPtr>(rhs.get()) );
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, class _CommonPtr = std::common_type_t<_Lhs*, _Rhs*> >
NODISCARD bool operator >=(const TRefPtr<_Lhs>& lhs, const TRefPtr<_Rhs>& rhs) {
    return !operator <(lhs, rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TSafePtr {
public:
    typedef T value_type;

    template <typename U>
    friend class TSafePtr;

    TSafePtr() NOEXCEPT;
    TSafePtr(T* ptr) NOEXCEPT;
    ~TSafePtr();

    TSafePtr(TSafePtr&& rvalue) NOEXCEPT;
    TSafePtr& operator =(TSafePtr&& rvalue) NOEXCEPT;

    TSafePtr(const TSafePtr& other) NOEXCEPT;
    TSafePtr& operator =(const TSafePtr& other) NOEXCEPT;

    template <typename U, class = Meta::TEnableIf<std::is_assignable_v<T*&, U*>> >
    TSafePtr(const TSafePtr<U>& other) NOEXCEPT;
    template <typename U, class = Meta::TEnableIf<std::is_assignable_v<T*&, U*>> >
    TSafePtr& operator =(const TSafePtr<U>& other) NOEXCEPT;

    template <typename U, class = Meta::TEnableIf<std::is_assignable_v<T*&, U*>> >
    TSafePtr(TSafePtr<U>&& rvalue) NOEXCEPT;
    template <typename U, class = Meta::TEnableIf<std::is_assignable_v<T*&, U*>> >
    TSafePtr& operator =(TSafePtr<U>&& rvalue) NOEXCEPT;

    template <typename U, class = Meta::TEnableIf<std::is_assignable_v<T*&, U*>> >
    TSafePtr(const TRefPtr<U>& refptr) NOEXCEPT : TSafePtr(refptr.get()) {}
    template <typename U, class = Meta::TEnableIf<std::is_assignable_v<T*&, U*>> >
    TSafePtr& operator =(const TRefPtr<U>&& refptr) NOEXCEPT { return operator =(refptr.get()); }

    NODISCARD T* get() const NOEXCEPT { return _ptr; }
    void reset(T* ptr = nullptr) NOEXCEPT;

    template <typename U>
    NODISCARD U *as() const NOEXCEPT { return checked_cast<U*>(_ptr); }

    NODISCARD T* Release() NOEXCEPT {
        T* const p = get();
        Assert(p);
        reset();
        return p;
    }

    T& operator *() const NOEXCEPT { Assert(_ptr); return *_ptr; }
    T* operator ->() const NOEXCEPT { Assert(_ptr); return _ptr; }

    operator T* () const NOEXCEPT { return _ptr; }

    template <typename U, class = Meta::TEnableIf<std::is_assignable_v<T*&, U*>> >
    void Swap(TSafePtr<U>& other) NOEXCEPT;

    friend void swap(TSafePtr& lhs, TSafePtr& rhs) NOEXCEPT {
        lhs.Swap(rhs);
    }

#if USE_PPE_SAFEPTR
protected:
    static void IncRefCountIFP(T* ptr) NOEXCEPT;
    static void DecRefCountIFP(T* ptr) NOEXCEPT;
#endif //!USE_PPE_SAFEPTR
private:
    T* _ptr;
};
//----------------------------------------------------------------------------
template <typename T> struct IsSafePtr : public std::false_type {};
template <typename T> struct IsSafePtr<TSafePtr<T>> : public std::true_type {};
//----------------------------------------------------------------------------
template <typename T>
NODISCARD Meta::TEnableIf< IsRefCountable<T>::value, TSafePtr<T> > MakeSafePtr(T* ptr) NOEXCEPT {
    return TSafePtr<T>(ptr);
}
//----------------------------------------------------------------------------
template <typename T>
NODISCARD Meta::TEnableIf< IsRefCountable<T>::value, TSafePtr<T> > MakeSafePtr(const TRefPtr<T>& refptr) NOEXCEPT {
    return TSafePtr<T>(refptr.get());
}
//----------------------------------------------------------------------------
template <typename T>
NODISCARD hash_t hash_value(const TSafePtr<T>& TSafePtr) NOEXCEPT {
    return hash_value(TSafePtr.get());
}
//----------------------------------------------------------------------------
template <typename _Dst, typename _Src>
NODISCARD TSafePtr<_Dst> checked_cast(const TSafePtr<_Src>& safeptr) NOEXCEPT {
    return { safeptr.template as<_Dst>() };
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, typename _CommonPtr = std::common_type_t<_Lhs*, _Rhs*> >
NODISCARD bool operator ==(const TSafePtr<_Lhs>& lhs, const TSafePtr<_Rhs>& rhs) NOEXCEPT {
    return (static_cast<_CommonPtr>(lhs.get()) == static_cast<_CommonPtr>(rhs.get()) );
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, typename _CommonPtr = std::common_type_t<_Lhs*, _Rhs*> >
NODISCARD bool operator !=(const TSafePtr<_Lhs>& lhs, const TSafePtr<_Rhs>& rhs) NOEXCEPT {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, typename _CommonPtr = std::common_type_t<_Lhs*, _Rhs*> >
NODISCARD bool operator <(const TSafePtr<_Lhs>& lhs, const TSafePtr<_Rhs>& rhs) {
    return (static_cast<_CommonPtr>(lhs.get()) < static_cast<_CommonPtr>(rhs.get()) );
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
NODISCARD bool operator >=(const TSafePtr<_Lhs>& lhs, const TSafePtr<_Rhs>& rhs) {
    return !operator <(lhs, rhs);
}
//----------------------------------------------------------------------------
PPE_ASSUME_TEMPLATE_AS_POINTER(TRefPtr<T>, typename T)
PPE_ASSUME_TEMPLATE_AS_POINTER(TSafePtr<T>, typename T)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Memory/RefPtr-inl.h"
