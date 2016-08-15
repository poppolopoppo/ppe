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

#define FWD_REFPTR(T) \
    class T; \
    typedef Core::RefPtr<T> CONCAT(P, T); \
    typedef Core::RefPtr<const T> CONCAT(PC, T); \
    typedef Core::SafePtr<T> CONCAT(S, T); \
    typedef Core::SafePtr<const T> CONCAT(SC, T)

#define FWD_INTERFACE_REFPTR(T_WITHOUT_I) \
    class CONCAT(I, T_WITHOUT_I); \
    typedef Core::RefPtr<CONCAT(I, T_WITHOUT_I)> CONCAT(P, T_WITHOUT_I); \
    typedef Core::RefPtr<const CONCAT(I, T_WITHOUT_I)> CONCAT(PC, T_WITHOUT_I); \
    typedef Core::SafePtr<CONCAT(I, T_WITHOUT_I)> CONCAT(S, T_WITHOUT_I); \
    typedef Core::SafePtr<const CONCAT(I, T_WITHOUT_I)> CONCAT(SC, T_WITHOUT_I)

#ifdef SAFEPTR
#   undef SAFEPTR
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class RefPtr;
//----------------------------------------------------------------------------
template <typename T>
class SafePtr;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class RefCountable {
public:
    RefCountable();
    ~RefCountable();

    RefCountable(RefCountable&& );
    RefCountable& operator =(RefCountable&& );

    RefCountable(const RefCountable& );
    RefCountable& operator =(const RefCountable& );

    size_t RefCount() const { return _refCount; }

protected:
    friend void AddRef(const RefCountable* ptr);
    template <typename T>
    friend void RemoveRef(T* ptr);
    template <typename T>
    friend void OnRefCountReachZero(T* ptr);
    template <typename T>
    friend void RemoveRef_AssertReachZero_NoDelete(T *& ptr);
    template <typename T>
    friend void RemoveRef_AssertReachZero(RefPtr<T>& ptr);
    template <typename T>
    friend void RemoveRef_AssertGreaterThanZero(RefPtr<T>& ptr);
    template <typename T>
    friend T *RemoveRef_AssertReachZero_KeepAlive(RefPtr<T>& ptr);

private:
    void IncRefCount() const;
    bool DecRefCount_ReturnIfReachZero() const;

    mutable std::atomic<size_t> _refCount;

#ifdef WITH_CORE_SAFEPTR
    // for debugging purpose : assert if SafePtr<> are still tracking that object
    template <typename T>
    friend class SafePtr;
    mutable std::atomic<size_t> _safeRefCount;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class RefPtr {
public:
    typedef T value_type;

    template <typename U>
    friend class RefPtr;

    RefPtr();
    RefPtr(T* ptr);
    ~RefPtr();

    RefPtr(RefPtr&& rvalue);
    RefPtr& operator =(RefPtr&& rvalue);

    RefPtr(const RefPtr& other);
    RefPtr& operator =(const RefPtr& other);

    template <typename U>
    RefPtr(const RefPtr<U>& other);
    template <typename U>
    RefPtr& operator =(const RefPtr<U>& other);

    template <typename U>
    RefPtr(RefPtr<U>&& rvalue);
    template <typename U>
    RefPtr& operator =(RefPtr<U>&& rvalue);

    FORCE_INLINE T* get() const { return _ptr; }
    void reset(T* ptr = nullptr);

    template <typename U>
    U *as() const { return checked_cast<U*>(_ptr); }

    T& operator *() const { Assert(_ptr); return *_ptr; }
    T* operator ->() const { Assert(_ptr); return _ptr; }

    //operator T* () const { return _ptr; } // better to let the user know what he/she is doing
    FORCE_INLINE operator const void *() const { return _ptr; }

    template <typename U>
    void Swap(RefPtr<U>& other);

protected:
    void IncRefCountIFP() const;
    void DecRefCountIFP() const;

private:
    T* _ptr;
};
STATIC_ASSERT(sizeof(RefPtr<RefCountable>) == sizeof(RefCountable*));
//----------------------------------------------------------------------------
template <typename T> struct IsRefPtr : public std::false_type {};
template <typename T> struct IsRefPtr<RefPtr<T>> : public std::true_type {};
//----------------------------------------------------------------------------
template <typename T>
hash_t hash_value(const RefPtr<T>& refPtr) {
    return hash_value(refPtr.get());
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
void swap(const RefPtr<_Lhs>& lhs, const RefPtr<_Rhs>& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
bool operator ==(const RefPtr<_Lhs>& lhs, const RefPtr<_Rhs>& rhs) {
    return (lhs.get() == static_cast<const _Lhs*>(rhs.get()) );
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
bool operator !=(const RefPtr<_Lhs>& lhs, const RefPtr<_Rhs>& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
bool operator <(const RefPtr<_Lhs>& lhs, const RefPtr<_Rhs>& rhs) {
    return (lhs.get() < static_cast<const _Lhs*>(rhs.get()) );
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
bool operator >=(const RefPtr<_Lhs>& lhs, const RefPtr<_Rhs>& rhs) {
    return !operator <(lhs, rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class SafePtr {
public:
    typedef T value_type;

    template <typename U>
    friend class SafePtr;

    SafePtr();
    SafePtr(T* ptr);
    ~SafePtr();

    SafePtr(SafePtr&& rvalue);
    SafePtr& operator =(SafePtr&& rvalue);

    SafePtr(const SafePtr& other);
    SafePtr& operator =(const SafePtr& other);

    template <typename U>
    SafePtr(const SafePtr<U>& other);
    template <typename U>
    SafePtr& operator =(const SafePtr<U>& other);

    template <typename U>
    SafePtr(SafePtr<U>&& rvalue);
    template <typename U>
    SafePtr& operator =(SafePtr<U>&& rvalue);

    T* get() const { return _ptr; }
    void reset(T* ptr = nullptr);

    template <typename U>
    U *as() const { return checked_cast<U*>(_ptr); }

    T& operator *() const { Assert(_ptr); return *_ptr; }
    T* operator ->() const { Assert(_ptr); return _ptr; }

    operator T* () const { return _ptr; }

    template <typename U>
    void Swap(SafePtr<U>& other);

protected:
    void IncRefCountIFP() const;
    void DecRefCountIFP() const;

private:
    T* _ptr;
};
//----------------------------------------------------------------------------
template <typename T> struct IsSafePtr : public std::false_type {};
template <typename T> struct IsSafePtr<SafePtr<T>> : public std::true_type {};
//----------------------------------------------------------------------------
template <typename T>
hash_t hash_value(const SafePtr<T>& SafePtr) {
    return hash_value(SafePtr.get());
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
void swap(const SafePtr<_Lhs>& lhs, const SafePtr<_Rhs>& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
bool operator ==(const SafePtr<_Lhs>& lhs, const SafePtr<_Rhs>& rhs) {
    return (lhs.get() == reinterpret_cast<_Lhs*>(rhs.get()) );
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
bool operator !=(const SafePtr<_Lhs>& lhs, const SafePtr<_Rhs>& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
bool operator <(const SafePtr<_Lhs>& lhs, const SafePtr<_Rhs>& rhs) {
    return (lhs.get() < reinterpret_cast<_Lhs*>(rhs.get()) );
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
bool operator >=(const SafePtr<_Lhs>& lhs, const SafePtr<_Rhs>& rhs) {
    return !operator <(lhs, rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template < typename T, typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const RefPtr<T>& refptr) {
    return oss << refptr.get();
}
//----------------------------------------------------------------------------
template < typename T, typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const SafePtr<T>& safeptr) {
    return oss << safeptr.get();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Memory/RefPtr-inl.h"
