#pragma once

#include "Core/Memory/RefPtr.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline RefCountable::RefCountable()
:   _refCount(0)
#ifdef WITH_CORE_SAFEPTR
,   _safeRefCount(0)
#endif
{}
//----------------------------------------------------------------------------
inline RefCountable::~RefCountable() {
    Assert(0 == _refCount);
#ifdef WITH_CORE_SAFEPTR
    // check if a SafePtr<> is still holding a reference to this object
    AssertRelease(0 == _safeRefCount);
#endif
}
//----------------------------------------------------------------------------
inline RefCountable::RefCountable(RefCountable&& )
:   _refCount(0)
#ifdef WITH_CORE_SAFEPTR
,   _safeRefCount(0)
#endif
{}
//----------------------------------------------------------------------------
inline RefCountable& RefCountable::operator =(RefCountable&& ) { return *this; }
//----------------------------------------------------------------------------
inline RefCountable::RefCountable(const RefCountable& )
:   _refCount(0)
#ifdef WITH_CORE_SAFEPTR
,   _safeRefCount(0)
#endif
{}
//----------------------------------------------------------------------------
inline RefCountable& RefCountable::operator =(const RefCountable& ) { return *this; }
//----------------------------------------------------------------------------
inline void RefCountable::IncRefCount() const {
    ++_refCount;
}
//----------------------------------------------------------------------------
inline bool RefCountable::DecRefCount_ReturnIfReachZero() const {
    Assert(_refCount);
    return (0 == --_refCount);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline void AddRef(const RefCountable* ptr) {
    ptr->IncRefCount();
}
//----------------------------------------------------------------------------
template <typename T>
void RemoveRef(T* ptr) {
    static_assert(std::is_base_of<RefCountable, T>::value, "T must be derived from RefCountable");
    if (ptr->DecRefCount_ReturnIfReachZero())
        OnRefCountReachZero(ptr);
}
//----------------------------------------------------------------------------
template <typename T>
void OnRefCountReachZero(T* ptr) {
    static_assert(std::is_base_of<RefCountable, T>::value, "T must be derived from RefCountable");
    checked_delete(ptr);
}
//----------------------------------------------------------------------------
template <typename T>
void RemoveRef_AssertReachZero_NoDelete(T *& ptr) {
    typedef char type_must_be_complete[sizeof(T) ? 1 : -1];
    (void) sizeof(type_must_be_complete);
    Assert(ptr);
    const bool result = ptr->DecRefCount_ReturnIfReachZero();
    Assert(result);
    ptr->~T();
    ptr = nullptr;
}
//----------------------------------------------------------------------------
template <typename T>
void RemoveRef_AssertReachZero(RefPtr<T>& ptr) {
    static_assert(std::is_base_of<RefCountable, T>::value, "T must be derived from RefCountable");
    Assert(ptr);
    Assert(1 == ptr->RefCount());
    ptr.reset();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
RefPtr<T>::RefPtr()
:   _ptr(nullptr) {}
//----------------------------------------------------------------------------
template <typename T>
RefPtr<T>::RefPtr(T* ptr)
:   _ptr(ptr) {
    IncRefCountIFP();
}
//----------------------------------------------------------------------------
template <typename T>
RefPtr<T>::~RefPtr() {
    DecRefCountIFP();
}
//----------------------------------------------------------------------------
template <typename T>
RefPtr<T>::RefPtr(RefPtr&& rvalue) : _ptr(rvalue._ptr) {
    rvalue._ptr = nullptr;
}
//----------------------------------------------------------------------------
template <typename T>
auto RefPtr<T>::operator =(RefPtr&& rvalue) -> RefPtr& {
    DecRefCountIFP();
    _ptr = rvalue._ptr;
    rvalue._ptr = nullptr;
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
RefPtr<T>::RefPtr(const RefPtr& other) : _ptr(other._ptr) {
    IncRefCountIFP();
}
//----------------------------------------------------------------------------
template <typename T>
auto RefPtr<T>::operator =(const RefPtr& other) -> RefPtr& {
    if (other._ptr != _ptr) {
        DecRefCountIFP();
        _ptr = other._ptr;
        IncRefCountIFP();
    }
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
RefPtr<T>::RefPtr(const RefPtr<U>& other)
: _ptr(checked_cast<T *>(other.get())) {
    IncRefCountIFP();
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
auto RefPtr<T>::operator =(const RefPtr<U>& other) -> RefPtr& {
    if (other.get() != _ptr) {
        DecRefCountIFP();
        _ptr = checked_cast<T *>(other.get());
        IncRefCountIFP();
    }
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
RefPtr<T>::RefPtr(RefPtr<U>&& rvalue)
:   _ptr(checked_cast<T *>(rvalue.get())) {
    rvalue._ptr = nullptr;
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
auto RefPtr<T>::operator =(RefPtr<U>&& rvalue) -> RefPtr& {
    DecRefCountIFP();
    _ptr = checked_cast<T *>(rvalue.get());
    rvalue._ptr = nullptr;
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
void RefPtr<T>::reset(T* ptr/* = nullptr */) {
    if (ptr != _ptr) {
        DecRefCountIFP();
        _ptr = ptr;
        IncRefCountIFP();
    }
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
void RefPtr<T>::Swap(RefPtr<U>& other) {
    std::swap(other._ptr, _ptr);
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE void RefPtr<T>::IncRefCountIFP() const {
    static_assert(std::is_base_of<RefCountable, T>::value, "T must be derived from RefCountable");
    if (_ptr)
        AddRef(_ptr);
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE void RefPtr<T>::DecRefCountIFP() const {
    static_assert(std::is_base_of<RefCountable, T>::value, "T must be derived from RefCountable");
    if (_ptr)
        RemoveRef(_ptr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
SafePtr<T>::SafePtr()
:   _ptr(nullptr) {}
//----------------------------------------------------------------------------
template <typename T>
SafePtr<T>::SafePtr(T* ptr)
:   _ptr(ptr) {
    IncRefCountIFP();
}
//----------------------------------------------------------------------------
template <typename T>
SafePtr<T>::~SafePtr() {
    DecRefCountIFP();
}
//----------------------------------------------------------------------------
template <typename T>
SafePtr<T>::SafePtr(SafePtr&& rvalue) : _ptr(rvalue._ptr) {
    rvalue._ptr = nullptr;
}
//----------------------------------------------------------------------------
template <typename T>
auto SafePtr<T>::operator =(SafePtr&& rvalue) -> SafePtr& {
    DecRefCountIFP();
    _ptr = rvalue._ptr;
    rvalue._ptr = nullptr;
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
SafePtr<T>::SafePtr(const SafePtr& other) : _ptr(other._ptr) {
    IncRefCountIFP();
}
//----------------------------------------------------------------------------
template <typename T>
auto SafePtr<T>::operator =(const SafePtr& other) -> SafePtr& {
    if (other._ptr != _ptr) {
        DecRefCountIFP();
        _ptr = other._ptr;
        IncRefCountIFP();
    }
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
SafePtr<T>::SafePtr(const SafePtr<U>& other)
: _ptr(checked_cast<T *>(other.get())) {
    IncRefCountIFP();
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
auto SafePtr<T>::operator =(const SafePtr<U>& other) -> SafePtr& {
    if (other.get() != _ptr) {
        DecRefCountIFP();
        _ptr = checked_cast<T *>(other.get());
        IncRefCountIFP();
    }
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
SafePtr<T>::SafePtr(SafePtr<U>&& rvalue)
:   _ptr(checked_cast<T *>(rvalue.get())) {
    rvalue._ptr = nullptr;
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
auto SafePtr<T>::operator =(SafePtr<U>&& rvalue) -> SafePtr& {
    DecRefCountIFP();
    _ptr = checked_cast<T *>(rvalue.get());
    rvalue._ptr = nullptr;
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
void SafePtr<T>::reset(T* ptr/* = nullptr */) {
    if (ptr != _ptr) {
        DecRefCountIFP();
        _ptr = ptr;
        IncRefCountIFP();
    }
}
//----------------------------------------------------------------------------
template <typename T>
template <typename U>
void SafePtr<T>::Swap(SafePtr<U>& other) {
    std::swap(other._ptr, _ptr);
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE void SafePtr<T>::IncRefCountIFP() const {
    static_assert(std::is_base_of<RefCountable, T>::value, "T must be derived from RefCountable");
#ifdef WITH_CORE_SAFEPTR
    if (_ptr) {
        ++_ptr->_safeRefCount;
    }
#endif
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE void SafePtr<T>::DecRefCountIFP() const {
    static_assert(std::is_base_of<RefCountable, T>::value, "T must be derived from RefCountable");
#ifdef WITH_CORE_SAFEPTR
    if (_ptr) {
        Assert(_ptr->_safeRefCount);
        --_ptr->_safeRefCount;
    }
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
