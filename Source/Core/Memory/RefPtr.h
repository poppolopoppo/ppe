#pragma once

#include "Core/Core.h"

#include "Core/Memory/MemoryDomain.h"

#include <type_traits>

/*
// Intrusive ref counting.
// Prefered to non intrusive ref counting (std::shared_ptr<>).
*/

#if defined(_DEBUG) || !defined(NDEBUG) || USE_CORE_MEMORY_DEBUGGING
#   define USE_CORE_SAFEPTR 1
#else
#   define USE_CORE_SAFEPTR 0
#endif

#define _FWD_REFPTR_IMPL(T, _PREFIX)                                    \
    class CONCAT(_PREFIX, T);                                           \
    typedef Core::TRefPtr<CONCAT(_PREFIX, T)>           CONCAT(P,  T);  \
    typedef Core::TRefPtr<const CONCAT(_PREFIX, T)>     CONCAT(PC, T);  \
    typedef Core::TSafePtr<CONCAT(_PREFIX, T)>          CONCAT(S,  T);  \
    typedef Core::TSafePtr<const CONCAT(_PREFIX, T)>    CONCAT(SC, T)

#define FWD_REFPTR(T_WITHOUT_F)             _FWD_REFPTR_IMPL(T_WITHOUT_F, F)
#define FWD_INTERFACE_REFPTR(T_WITHOUT_I)   _FWD_REFPTR_IMPL(T_WITHOUT_I, I)

// used for tracking memory allocated by instances derived from FRefCountable
#if USE_CORE_MEMORYDOMAINS
#   include "Core/Allocator/TrackingMalloc.h"
#   define NEW_REF(_DOMAIN, T) new (MEMORYDOMAIN_TRACKING_DATA(_DOMAIN)) T
#else
#   include "Core/Allocator/Malloc.h"
#   define NEW_REF(_DOMAIN, T) new (Meta::ForceInit) T
#endif

#ifdef SAFEPTR
#   undef SAFEPTR
#endif

namespace Core {
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
class CORE_API FRefCountable {
public:
    FRefCountable();
    ~FRefCountable();

    FRefCountable(FRefCountable&& );
    FRefCountable& operator =(FRefCountable&& );

    FRefCountable(const FRefCountable& );
    FRefCountable& operator =(const FRefCountable& );

    int RefCount() const { return _refCount; }

#if USE_CORE_SAFEPTR
    int SafeRefCount() const { return _safeRefCount; }
#endif

public: // override new/delete operators for memory tracking
#if USE_CORE_MEMORYDOMAINS
    static void* operator new(size_t sz, FMemoryTracking& trackingData) { return tracking_malloc(trackingData, sz); }
    static void operator delete(void* p, FMemoryTracking&) { tracking_free(p); }
    static void operator delete(void* p) { tracking_free(p); }
#else
    static void* operator new(size_t sz, Meta::FForceInit/* force to use macro even wout domains */) { return Core::malloc(sz); }
    static void operator delete(void* p, Meta::FForceInit/* force to use macro even wout domains */) { Core::free(p); }
    static void operator delete(void* p) { Core::free(p); }
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
    friend void RemoveRef_AssertReachZero_NoDelete(T *& ptr);
    template <typename T>
    friend void RemoveRef_AssertReachZero(TRefPtr<T>& ptr);
    template <typename T>
    friend void RemoveRef_AssertGreaterThanZero(TRefPtr<T>& ptr);
    template <typename T>
    friend T *RemoveRef_AssertReachZero_KeepAlive(TRefPtr<T>& ptr);

#if USE_CORE_SAFEPTR
    friend void AddSafeRef(const FRefCountable* ptr);
    friend void RemoveSafeRef(const FRefCountable* ptr);
#else
    inline friend void AddSafeRef(const FRefCountable*) {}
    inline friend void RemoveSafeRef(const FRefCountable*) {}
#endif

private:
    void IncRefCount() const;
    bool DecRefCount_ReturnIfReachZero() const;

    mutable std::atomic<int> _refCount;

#if USE_CORE_SAFEPTR
    // for debugging purpose : assert if TSafePtr<> are still tracking that object
    template <typename T>
    friend class TSafePtr;
    mutable std::atomic<int> _safeRefCount;

    void IncSafeRefCount() const;
    void DecSafeRefCount() const;
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

    TRefPtr(TRefPtr&& rvalue);
    TRefPtr& operator =(TRefPtr&& rvalue);

    TRefPtr(const TRefPtr& other);
    TRefPtr& operator =(const TRefPtr& other);

    template <typename U>
    TRefPtr(const TRefPtr<U>& other);
    template <typename U>
    TRefPtr& operator =(const TRefPtr<U>& other);

    template <typename U>
    TRefPtr(TRefPtr<U>&& rvalue);
    template <typename U>
    TRefPtr& operator =(TRefPtr<U>&& rvalue);

    FORCE_INLINE T* get() const { return _ptr; }
    void reset(T* ptr = nullptr);

    template <typename U>
    U *as() const { return checked_cast<U*>(_ptr); }

    T& operator *() const { Assert(_ptr); return *_ptr; }
    T* operator ->() const { Assert(_ptr); return _ptr; }

    CORE_FAKEBOOL_OPERATOR_DECL() { return _ptr; }
    bool valid() const { return (!!_ptr); }

    template <typename U>
    void Swap(TRefPtr<U>& other);

protected:
    void IncRefCountIFP() const;
    void DecRefCountIFP() const;

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

    TSafePtr();
    TSafePtr(T* ptr);
    ~TSafePtr();

    TSafePtr(TSafePtr&& rvalue);
    TSafePtr& operator =(TSafePtr&& rvalue);

    TSafePtr(const TSafePtr& other);
    TSafePtr& operator =(const TSafePtr& other);

    template <typename U>
    TSafePtr(const TSafePtr<U>& other);
    template <typename U>
    TSafePtr& operator =(const TSafePtr<U>& other);

    template <typename U>
    TSafePtr(TSafePtr<U>&& rvalue);
    template <typename U>
    TSafePtr& operator =(TSafePtr<U>&& rvalue);

    template <typename U>
    TSafePtr(const TRefPtr<U>& refptr) : TSafePtr(refptr.get()) {}
    template <typename U>
    TSafePtr& operator =(const TRefPtr<U>&& refptr) { return operator =(refptr.get()); }

    T* get() const { return _ptr; }
    void reset(T* ptr = nullptr);

    template <typename U>
    U *as() const { return checked_cast<U*>(_ptr); }

    T& operator *() const { Assert(_ptr); return *_ptr; }
    T* operator ->() const { Assert(_ptr); return _ptr; }

    operator T* () const { return _ptr; }

    template <typename U>
    void Swap(TSafePtr<U>& other);

protected:
    void IncRefCountIFP() const;
    void DecRefCountIFP() const;

private:
    T* _ptr;
};
//----------------------------------------------------------------------------
template <typename T> struct IsSafePtr : public std::false_type {};
template <typename T> struct IsSafePtr<TSafePtr<T>> : public std::true_type {};
//----------------------------------------------------------------------------
template <typename T>
Meta::TEnableIf< IsRefCountable<T>::value, TSafePtr<T> > MakeSafePtr(T* ptr) {
    return TSafePtr<T>(ptr);
}
//----------------------------------------------------------------------------
template <typename T>
hash_t hash_value(const TSafePtr<T>& TSafePtr) {
    return hash_value(TSafePtr.get());
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
void swap(const TSafePtr<_Lhs>& lhs, const TSafePtr<_Rhs>& rhs) {
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
} //!namespace Core

#include "Core/Memory/RefPtr-inl.h"
