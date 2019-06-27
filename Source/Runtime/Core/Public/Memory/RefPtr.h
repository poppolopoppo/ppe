#pragma once

#include "Core.h"

#include "Memory/MemoryDomain.h"

#include <type_traits>

/*
// Intrusive ref counting.
// Prefered to non intrusive ref counting (std::shared_ptr<>).
*/

#define USE_PPE_SAFEPTR (USE_PPE_DEBUG || USE_PPE_MEMORY_DEBUGGING)

#define _FWD_REFPTR_IMPL(T, _PREFIX)                                    \
    class CONCAT(_PREFIX, T);                                           \
    typedef ::PPE::TRefPtr<CONCAT(_PREFIX, T)>           CONCAT(P,  T); \
    typedef ::PPE::TRefPtr<const CONCAT(_PREFIX, T)>     CONCAT(PC, T); \
    typedef ::PPE::TSafePtr<CONCAT(_PREFIX, T)>          CONCAT(S,  T); \
    typedef ::PPE::TSafePtr<const CONCAT(_PREFIX, T)>    CONCAT(SC, T)

#define FWD_REFPTR(T_WITHOUT_F)             _FWD_REFPTR_IMPL(T_WITHOUT_F, F)
#define FWD_INTERFACE_REFPTR(T_WITHOUT_I)   _FWD_REFPTR_IMPL(T_WITHOUT_I, I)

// used for tracking memory allocated by instances derived from FRefCountable
#if USE_PPE_MEMORYDOMAINS
#   include "Allocator/TrackingMalloc.h"
#   define NEW_REF(_DOMAIN, T) new (MEMORYDOMAIN_TRACKING_DATA(_DOMAIN)) T
#else
#   include "Allocator/Malloc.h"
#   define NEW_REF(_DOMAIN, T) new (Meta::ForceInit) T
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
#if USE_PPE_MEMORYDOMAINS
    static void* operator new(size_t sz, FMemoryTracking& trackingData) { return tracking_malloc(trackingData, sz); }
    static void operator delete(void* p, FMemoryTracking&) { tracking_free(p); }
    static void operator delete(void* p) { tracking_free(p); }
#else
    static void* operator new(size_t sz, Meta::FForceInit/* force to use macro even wout domains */) { return PPE::malloc(sz); }
    static void operator delete(void* p, Meta::FForceInit/* force to use macro even wout domains */) { PPE::free(p); }
    static void operator delete(void* p) { PPE::free(p); }
#endif

    // general allocators are forbidden to force the client to provide metadata
    static void * operator new(std::size_t) = delete;
    static void * operator new[](std::size_t) = delete;

protected:
    friend void AddRef(const FRefCountable* ptr);
    template <typename T>
    friend void RemoveRef(T* ptr);
    template <typename T>
    friend T* RemoveRef_AssertAlive(TRefPtr<T>& ptr);
    template <typename T>
    friend T* RemoveRef_KeepAlive(TRefPtr<T>& ptr);
    template <typename T>
    friend void OnRefCountReachZero(T* ptr);
    template <typename T>
    friend void RemoveRef_AssertReachZero_NoDelete(T* ptr);
    template <typename T>
    friend void RemoveRef_AssertReachZero(TRefPtr<T>& ptr);
    template <typename T>
    friend T *RemoveRef_AssertReachZero_KeepAlive(TRefPtr<T>& ptr);

#if USE_PPE_SAFEPTR
    friend void AddSafeRef(const FRefCountable* ptr) NOEXCEPT;
    friend void RemoveSafeRef(const FRefCountable* ptr) NOEXCEPT;
#else
    FORCE_INLINE friend void AddSafeRef(const FRefCountable*) NOEXCEPT {}
    FORCE_INLINE friend void RemoveSafeRef(const FRefCountable*) NOEXCEPT {}
#endif

private:
    void IncRefCount() const NOEXCEPT;
    bool DecRefCount_ReturnIfReachZero() const NOEXCEPT;

    mutable std::atomic<int> _refCount;

#if USE_PPE_SAFEPTR
    // for debugging purpose : assert if TSafePtr<> are still tracking that object
    template <typename T>
    friend class TSafePtr;
    mutable std::atomic<int> _safeRefCount;

    void IncSafeRefCount() const NOEXCEPT;
    void DecSafeRefCount() const NOEXCEPT;
#endif
};
//----------------------------------------------------------------------------
template <typename T>
using IsRefCountable = std::is_base_of<FRefCountable, Meta::TDecay<T> >;
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

    template <typename U>
    TRefPtr(const TRefPtr<U>& other);
    template <typename U>
    TRefPtr& operator =(const TRefPtr<U>& other);

    template <typename U>
    TRefPtr(TRefPtr<U>&& rvalue) NOEXCEPT;
    template <typename U>
    TRefPtr& operator =(TRefPtr<U>&& rvalue) NOEXCEPT;

    FORCE_INLINE T* get() const { return _ptr; }
    void reset(T* ptr = nullptr);

    template <typename U>
    U *as() const { return checked_cast<U*>(_ptr); }

    T& operator *() const { Assert(_ptr); return *_ptr; }
    T* operator ->() const { Assert(_ptr); return _ptr; }

    PPE_FAKEBOOL_OPERATOR_DECL() { return _ptr; }
    bool valid() const { return (!!_ptr); }

    template <typename U>
    void Swap(TRefPtr<U>& other);

protected:
    void IncRefCountIFP() const NOEXCEPT;
    void DecRefCountIFP() const NOEXCEPT;

private:
    T* _ptr;
};
STATIC_ASSERT(sizeof(TRefPtr<FRefCountable>) == sizeof(FRefCountable*));
//----------------------------------------------------------------------------
template <typename T> struct IsRefPtr : public std::false_type {};
template <typename T> struct IsRefPtr<TRefPtr<T>> : public std::true_type {};
//----------------------------------------------------------------------------
template <typename T>
Meta::TEnableIf< IsRefCountable<T>::value, TRefPtr<T> > MakeRefPtr(T* ptr) {
    return TRefPtr<T>(ptr);
}
//----------------------------------------------------------------------------
template <typename T>
hash_t hash_value(const TRefPtr<T>& refPtr) {
    return hash_value(refPtr.get());
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
void swap(const TRefPtr<_Lhs>& lhs, const TRefPtr<_Rhs>& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
bool operator ==(const TRefPtr<_Lhs>& lhs, const TRefPtr<_Rhs>& rhs) {
    return (lhs.get() == static_cast<const _Lhs*>(rhs.get()) );
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
bool operator !=(const TRefPtr<_Lhs>& lhs, const TRefPtr<_Rhs>& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
bool operator <(const TRefPtr<_Lhs>& lhs, const TRefPtr<_Rhs>& rhs) {
    return (lhs.get() < static_cast<const _Lhs*>(rhs.get()) );
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
bool operator >=(const TRefPtr<_Lhs>& lhs, const TRefPtr<_Rhs>& rhs) {
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

    template <typename U>
    TSafePtr(const TSafePtr<U>& other) NOEXCEPT;
    template <typename U>
    TSafePtr& operator =(const TSafePtr<U>& other) NOEXCEPT;

    template <typename U>
    TSafePtr(TSafePtr<U>&& rvalue) NOEXCEPT;
    template <typename U>
    TSafePtr& operator =(TSafePtr<U>&& rvalue) NOEXCEPT;

    template <typename U>
    TSafePtr(const TRefPtr<U>& refptr) NOEXCEPT : TSafePtr(refptr.get()) {}
    template <typename U>
    TSafePtr& operator =(const TRefPtr<U>&& refptr) NOEXCEPT { return operator =(refptr.get()); }

    T* get() const NOEXCEPT { return _ptr; }
    void reset(T* ptr = nullptr) NOEXCEPT;

    template <typename U>
    U *as() const NOEXCEPT { return checked_cast<U*>(_ptr); }

    T& operator *() const NOEXCEPT { Assert(_ptr); return *_ptr; }
    T* operator ->() const NOEXCEPT { Assert(_ptr); return _ptr; }

    operator T* () const NOEXCEPT { return _ptr; }

    template <typename U>
    void Swap(TSafePtr<U>& other) NOEXCEPT;

#if USE_PPE_SAFEPTR
protected:
    void IncRefCountIFP() const NOEXCEPT;
    void DecRefCountIFP() const NOEXCEPT;
#endif //!USE_PPE_SAFEPTR
private:
    T* _ptr;
};
//----------------------------------------------------------------------------
template <typename T> struct IsSafePtr : public std::false_type {};
template <typename T> struct IsSafePtr<TSafePtr<T>> : public std::true_type {};
//----------------------------------------------------------------------------
template <typename T>
Meta::TEnableIf< IsRefCountable<T>::value, TSafePtr<T> > MakeSafePtr(T* ptr) NOEXCEPT {
    return TSafePtr<T>(ptr);
}
//----------------------------------------------------------------------------
template <typename T>
hash_t hash_value(const TSafePtr<T>& TSafePtr) NOEXCEPT {
    return hash_value(TSafePtr.get());
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
void swap(const TSafePtr<_Lhs>& lhs, const TSafePtr<_Rhs>& rhs) NOEXCEPT {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
bool operator ==(const TSafePtr<_Lhs>& lhs, const TSafePtr<_Rhs>& rhs) {
    return (lhs.get() == reinterpret_cast<_Lhs*>(rhs.get()) );
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
bool operator !=(const TSafePtr<_Lhs>& lhs, const TSafePtr<_Rhs>& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
bool operator <(const TSafePtr<_Lhs>& lhs, const TSafePtr<_Rhs>& rhs) {
    return (lhs.get() < reinterpret_cast<_Lhs*>(rhs.get()) );
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
bool operator >=(const TSafePtr<_Lhs>& lhs, const TSafePtr<_Rhs>& rhs) {
    return !operator <(lhs, rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Memory/RefPtr-inl.h"
