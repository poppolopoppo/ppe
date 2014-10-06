#pragma once

#include "ComPtr.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _ComInterface>
ComPtr<_ComInterface>::ComPtr() {}
//----------------------------------------------------------------------------
template <typename _ComInterface>
ComPtr<_ComInterface>::ComPtr(_ComInterface* comObject)
:   _comObject(comObject) {
    IncRefCountIFP();
}
//----------------------------------------------------------------------------
template <typename _ComInterface>
ComPtr<_ComInterface>::~ComPtr() {
    DecRefCountIFP();
}
//----------------------------------------------------------------------------
template <typename _ComInterface>
ComPtr<_ComInterface>::ComPtr(ComPtr&& rvalue)
:   Meta::ThreadResource(std::move(rvalue)) {
    CheckThreadAccess();
    _comObject = rvalue._comObject;
    _comObject = nullptr;
}
//----------------------------------------------------------------------------
template <typename _ComInterface>
auto ComPtr<_ComInterface>::operator =(ComPtr&& rvalue) -> ComPtr& {
    Meta::ThreadResource::operator =(std::move(rvalue));
    CheckThreadAccess();
    _comObject = rvalue._comObject;
    _comObject = nullptr;
    return *this;
}
//----------------------------------------------------------------------------
template <typename _ComInterface>
ComPtr<_ComInterface>::ComPtr(const ComPtr& other)
:   Meta::ThreadResource(other)
,   _comObject(other._comObject) {
    IncRefCountIFP();
}
//----------------------------------------------------------------------------
template <typename _ComInterface>
auto ComPtr<_ComInterface>::operator =(const ComPtr& other) -> ComPtr& {
    Meta::ThreadResource::operator =(other);
    DecRefCountIFP();
    _comObject = other._comObject;
    IncRefCountIFP();
    return *this;
}
//----------------------------------------------------------------------------
template <typename _ComInterface>
FORCE_INLINE _ComInterface *ComPtr<_ComInterface>::Get() const {
    CheckThreadAccess();
    return _comObject;
}
//----------------------------------------------------------------------------
template <typename _ComInterface>
_ComInterface **ComPtr<_ComInterface>::GetAddressOf() {
    Reset();
    return &_comObject;
}
//----------------------------------------------------------------------------
template <typename _ComInterface>
void ComPtr<_ComInterface>::Reset() {
    DecRefCountIFP();
    _comObject = nullptr;
}
//----------------------------------------------------------------------------
template <typename _ComInterface>
u32 ComPtr<_ComInterface>::RefCount() const {
    CheckThreadAccess();
    if (!_comObject)
        return ULONG(0);

    _comObject->AddRef();
    return checked_cast<u32>(_comObject->Release());
}
//----------------------------------------------------------------------------
template <typename _ComInterface>
void ComPtr<_ComInterface>::Swap(ComPtr& other) {
    CheckThreadAccess();
    std::swap(_comObject, other._comObject);
}
//----------------------------------------------------------------------------
template <typename _ComInterface>
FORCE_INLINE void ComPtr<_ComInterface>::CheckThreadAccess() const {
    THIS_THREADRESOURCE_CHECKACCESS();
}
//----------------------------------------------------------------------------
template <typename _ComInterface>
FORCE_INLINE void ComPtr<_ComInterface>::IncRefCountIFP() const {
    CheckThreadAccess();
    if (_comObject)
        _comObject->AddRef();
}
//----------------------------------------------------------------------------
template <typename _ComInterface>
FORCE_INLINE void ComPtr<_ComInterface>::DecRefCountIFP() const {
    CheckThreadAccess();
    if (_comObject)
        _comObject->Release();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
