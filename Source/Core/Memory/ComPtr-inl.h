#pragma once

#include "Core/Memory/ComPtr.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _ComInterface>
TComPtr<_ComInterface>::TComPtr() : _comObject(nullptr) {}
//----------------------------------------------------------------------------
template <typename _ComInterface>
TComPtr<_ComInterface>::TComPtr(_ComInterface* comObject)
:   _comObject(comObject) {
    IncRefCountIFP();
}
//----------------------------------------------------------------------------
template <typename _ComInterface>
TComPtr<_ComInterface>::~TComPtr() {
    DecRefCountIFP();
}
//----------------------------------------------------------------------------
template <typename _ComInterface>
TComPtr<_ComInterface>::TComPtr(TComPtr&& rvalue)
:   Meta::FThreadResource(std::move(rvalue)) {
    CheckThreadAccess();
    _comObject = rvalue._comObject;
    _comObject = nullptr;
}
//----------------------------------------------------------------------------
template <typename _ComInterface>
auto TComPtr<_ComInterface>::operator =(TComPtr&& rvalue) -> TComPtr& {
    Meta::FThreadResource::operator =(std::move(rvalue));
    CheckThreadAccess();
    _comObject = rvalue._comObject;
    _comObject = nullptr;
    return *this;
}
//----------------------------------------------------------------------------
template <typename _ComInterface>
TComPtr<_ComInterface>::TComPtr(const TComPtr& other)
:   Meta::FThreadResource(other)
,   _comObject(other._comObject) {
    IncRefCountIFP();
}
//----------------------------------------------------------------------------
template <typename _ComInterface>
auto TComPtr<_ComInterface>::operator =(const TComPtr& other) -> TComPtr& {
    Meta::FThreadResource::operator =(other);
    DecRefCountIFP();
    _comObject = other._comObject;
    IncRefCountIFP();
    return *this;
}
//----------------------------------------------------------------------------
template <typename _ComInterface>
FORCE_INLINE _ComInterface *TComPtr<_ComInterface>::Get() const {
    CheckThreadAccess();
    return _comObject;
}
//----------------------------------------------------------------------------
template <typename _ComInterface>
_ComInterface **TComPtr<_ComInterface>::GetAddressOf() {
    Reset();
    return &_comObject;
}
//----------------------------------------------------------------------------
template <typename _ComInterface>
void TComPtr<_ComInterface>::Reset() {
    DecRefCountIFP();
    _comObject = nullptr;
}
//----------------------------------------------------------------------------
template <typename _ComInterface>
u32 TComPtr<_ComInterface>::RefCount() const {
    CheckThreadAccess();
    if (!_comObject)
        return ULONG(0);

    _comObject->AddRef();
    return checked_cast<u32>(_comObject->Release());
}
//----------------------------------------------------------------------------
template <typename _ComInterface>
void TComPtr<_ComInterface>::Swap(TComPtr& other) {
    CheckThreadAccess();
    std::swap(_comObject, other._comObject);
}
//----------------------------------------------------------------------------
template <typename _ComInterface>
FORCE_INLINE void TComPtr<_ComInterface>::CheckThreadAccess() const {
    THIS_THREADRESOURCE_CHECKACCESS();
}
//----------------------------------------------------------------------------
template <typename _ComInterface>
FORCE_INLINE void TComPtr<_ComInterface>::IncRefCountIFP() const {
    CheckThreadAccess();
    if (_comObject)
        _comObject->AddRef();
}
//----------------------------------------------------------------------------
template <typename _ComInterface>
FORCE_INLINE void TComPtr<_ComInterface>::DecRefCountIFP() const {
    CheckThreadAccess();
    if (_comObject)
        _comObject->Release();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
