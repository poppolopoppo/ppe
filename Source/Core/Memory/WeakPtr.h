#pragma once

#include "Core/Core.h"

#include "Core/Memory/RefPtr.h"
#include "Core/Meta/ThreadResource.h"

#define _FWD_WEAKPTR_IMPL(T, _PREFIX)                                   \
    class CONCAT(_PREFIX, T);                                           \
    typedef Core::TWeakPtr<CONCAT(_PREFIX, T)>           CONCAT(W,  T); \
    typedef Core::TWeakPtr<const CONCAT(_PREFIX, T)>     CONCAT(WC, T)

#define FWD_WEAKPTR(T_WITHOUT_F)            _FWD_WEAKPTR_IMPL(T_WITHOUT_F, F)
#define FWD_INTERFACE_WEAKPTR(T_WITHOUT_I)  _FWD_WEAKPTR_IMPL(T_WITHOUT_I, I)

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TWeakPtr;
//----------------------------------------------------------------------------
class FWeakPtrBase;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FWeakAndRefCountable : public FRefCountable, public Meta::FThreadResource {
public:
    FWeakAndRefCountable();
    ~FWeakAndRefCountable();

    FWeakAndRefCountable(FWeakAndRefCountable&& );
    FWeakAndRefCountable& operator =(FWeakAndRefCountable&& );

    FWeakAndRefCountable(const FWeakAndRefCountable& );
    FWeakAndRefCountable& operator =(const FWeakAndRefCountable& );

private:
    friend class FWeakPtrBase;

    mutable FWeakPtrBase *_weakPtrs = nullptr;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FWeakPtrBase {
public:
    ~FWeakPtrBase();

protected:
    FWeakPtrBase(void **pptr);

    template <typename T>
    void set_(T *ptr);

private:
    friend class FWeakAndRefCountable;

    void **_pptr;
    FWeakPtrBase *_next, *_prev;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TWeakPtr : public FWeakPtrBase {
public:
    template <typename U>
    friend class TWeakPtr;

    TWeakPtr();
    TWeakPtr(T* ptr);
    ~TWeakPtr();

    TWeakPtr(TWeakPtr&& rvalue);
    TWeakPtr& operator =(TWeakPtr&& rvalue);

    TWeakPtr(const TWeakPtr& other);
    TWeakPtr& operator =(const TWeakPtr& other);

    template <typename U>
    TWeakPtr(const TWeakPtr<U>& other);
    template <typename U>
    TWeakPtr& operator =(const TWeakPtr<U>& other);

    template <typename U>
    TWeakPtr(TWeakPtr<U>&& rvalue);
    template <typename U>
    TWeakPtr& operator =(TWeakPtr<U>&& rvalue);

    void reset(T* ptr = nullptr);

    template <typename U>
    bool TryLock(TRefPtr<U> *pLocked) const;

    template <typename U>
    void Swap(TWeakPtr<U>& other);

private:
    T *_ptr;
};
//----------------------------------------------------------------------------
template <typename T>
hash_t hash_value(const TWeakPtr<T>& weakPtr) {
    return hash_value(weakPtr.get());
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
void swap(const TWeakPtr<_Lhs>& lhs, const TWeakPtr<_Rhs>& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
bool operator ==(const TWeakPtr<_Lhs>& lhs, const TWeakPtr<_Rhs>& rhs) {
    return (lhs.get() == reinterpret_cast<_Lhs*>(rhs.get()) );
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
bool operator !=(const TWeakPtr<_Lhs>& lhs, const TWeakPtr<_Rhs>& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
bool operator <(const TWeakPtr<_Lhs>& lhs, const TWeakPtr<_Rhs>& rhs) {
    return (lhs.get() < reinterpret_cast<_Lhs*>(rhs.get()) );
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
bool operator >=(const TWeakPtr<_Lhs>& lhs, const TWeakPtr<_Rhs>& rhs) {
    return !operator <(lhs, rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Memory/WeakPtr-inl.h"
