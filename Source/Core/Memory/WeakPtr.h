#pragma once

#include "Core/Core.h"

#include "Core/Memory/RefPtr.h"
#include "Core/Meta/ThreadResource.h"

#define FWD_WEAKPTR(T) \
    class T; \
    typedef Core::WeakPtr<T> CONCAT(W, T); \
    typedef Core::WeakPtr<const T> CONCAT(WC, T)

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class WeakPtr;
//----------------------------------------------------------------------------
class WeakPtrBase;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class WeakAndRefCountable : public RefCountable, public Meta::ThreadResource {
public:
    WeakAndRefCountable();
    ~WeakAndRefCountable();

    WeakAndRefCountable(WeakAndRefCountable&& );
    WeakAndRefCountable& operator =(WeakAndRefCountable&& );

    WeakAndRefCountable(const WeakAndRefCountable& );
    WeakAndRefCountable& operator =(const WeakAndRefCountable& );

private:
    friend class WeakPtrBase;

    mutable WeakPtrBase *_weakPtrs = nullptr;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class WeakPtrBase {
public:
    ~WeakPtrBase();

protected:
    WeakPtrBase(const WeakAndRefCountable *ptr = nullptr);

    const WeakAndRefCountable *get_() const;
    void set_(const WeakAndRefCountable *ptr);

private:
    friend class WeakAndRefCountable;

    const WeakAndRefCountable *_ptr;
    WeakPtrBase *_next, *_prev;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class WeakPtr : public WeakPtrBase {
public:
    template <typename U>
    friend class WeakPtr;

    WeakPtr();
    WeakPtr(T* ptr);
    ~WeakPtr();

    WeakPtr(WeakPtr&& rvalue);
    WeakPtr& operator =(WeakPtr&& rvalue);

    WeakPtr(const WeakPtr& other);
    WeakPtr& operator =(const WeakPtr& other);

    template <typename U>
    WeakPtr(const WeakPtr<U>& other);
    template <typename U>
    WeakPtr& operator =(const WeakPtr<U>& other);

    template <typename U>
    WeakPtr(WeakPtr<U>&& rvalue);
    template <typename U>
    WeakPtr& operator =(WeakPtr<U>&& rvalue);

    T* get() const { return reinterpret_cast<T *>(WeakPtrBase::get_()); }
    void reset(T* ptr = nullptr);

    template <typename U>
    U *as() const { return checked_cast<U*>(WeakPtrBase::_ptr); }

    T& operator *() const { T *const ptr = reinterpret_cast<T *>(WeakPtrBase::get_()); Assert(ptr); return *ptr; }
    T* operator ->() const { T *const ptr = reinterpret_cast<T *>(WeakPtrBase::get_()); Assert(ptr); return ptr; }

    operator T* () const { return reinterpret_cast<T *>(WeakPtrBase::get_()); }

    template <typename U>
    void Swap(WeakPtr<U>& other);
};
//----------------------------------------------------------------------------
template <typename T>
size_t hash_value(const WeakPtr<T>& weakPtr) {
    return hash_value(weakPtr.get());
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
void swap(const WeakPtr<_Lhs>& lhs, const WeakPtr<_Rhs>& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
bool operator ==(const WeakPtr<_Lhs>& lhs, const WeakPtr<_Rhs>& rhs) {
    return (lhs.get() == reinterpret_cast<_Lhs*>(rhs.get()) );
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
bool operator !=(const WeakPtr<_Lhs>& lhs, const WeakPtr<_Rhs>& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
bool operator <(const WeakPtr<_Lhs>& lhs, const WeakPtr<_Rhs>& rhs) {
    return (lhs.get() < reinterpret_cast<_Lhs*>(rhs.get()) );
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
bool operator >=(const WeakPtr<_Lhs>& lhs, const WeakPtr<_Rhs>& rhs) {
    return !operator <(lhs, rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template < typename T, typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const WeakPtr<T>& weakPtr) {
    return oss << weakPtr.get();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Memory/WeakPtr-inl.h"
