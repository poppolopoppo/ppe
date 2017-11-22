#pragma once

#include "Core/Core.h"

#include <iosfwd>
#include <type_traits>

/*
// Intrusive ref counting.
// Prefered to non intrusive ref counting (std::shared_ptr<>).
*/

#ifdef _DEBUG
#   define WITH_CORE_SAFEPTR
#endif

#define _FWD_REFPTR_IMPL(T, _PREFIX)                                    \
    class CONCAT(_PREFIX, T);                                           \
    typedef Core::TRefPtr<CONCAT(_PREFIX, T)>           CONCAT(P,  T);  \
    typedef Core::TRefPtr<const CONCAT(_PREFIX, T)>     CONCAT(PC, T);  \
    typedef Core::TSafePtr<CONCAT(_PREFIX, T)>          CONCAT(S,  T);  \
    typedef Core::TSafePtr<const CONCAT(_PREFIX, T)>    CONCAT(SC, T)

#define FWD_REFPTR(T_WITHOUT_F)             _FWD_REFPTR_IMPL(T_WITHOUT_F, F)
#define FWD_INTERFACE_REFPTR(T_WITHOUT_I)   _FWD_REFPTR_IMPL(T_WITHOUT_I, I)


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
class FRefCountable {
public:
    FRefCountable();
    ~FRefCountable();

    FRefCountable(FRefCountable&& );
    FRefCountable& operator =(FRefCountable&& );

    FRefCountable(const FRefCountable& );
    FRefCountable& operator =(const FRefCountable& );

    size_t RefCount() const { return _refCount; }

#ifdef WITH_CORE_SAFEPTR
    size_t SafeRefCount() const { return _safeRefCount; }
#endif

protected:
    friend void AddRef(const FRefCountable* ptr);
    template <typename T>
    friend void RemoveRef(T* ptr);
    template <typename T>
    friend T* RemoveRef_AssertAlive(TRefPtr<T>& ptr);
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

private:
    void IncRefCount() const;
    bool DecRefCount_ReturnIfReachZero() const;

    mutable std::atomic<size_t> _refCount;

#ifdef WITH_CORE_SAFEPTR
    // for debugging purpose : assert if TSafePtr<> are still tracking that object
    template <typename T>
    friend class TSafePtr;
    mutable std::atomic<size_t> _safeRefCount;
#endif
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
template < typename T, typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const TRefPtr<T>& refptr) {
    return oss << refptr.get();
}
//----------------------------------------------------------------------------
template < typename T, typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const TSafePtr<T>& safeptr) {
    return oss << safeptr.get();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Memory/RefPtr-inl.h"
