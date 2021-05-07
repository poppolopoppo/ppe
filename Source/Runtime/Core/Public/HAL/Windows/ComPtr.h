#pragma once

#include "Core.h"

#ifndef PLATFORM_WINDOWS
#   error "invalid include for current platform"
#endif

#include "Memory/RefPtr.h"
#include "Meta/Assert.h"
#include "Meta/ThreadResource.h"

/*
// Intrusive ref counting for TComponent Object FModel (COM)
*/

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _ComInterface>
class TComPtr : private Meta::FThreadResource {
public:
    template <typename U>
    friend class TComPtr;

    TComPtr();
    ~TComPtr();

    TComPtr(_ComInterface* comObject);
    TComPtr& operator =(_ComInterface* comObject) = delete;

    TComPtr(TComPtr&& rvalue) NOEXCEPT;
    TComPtr& operator =(TComPtr&& rvalue) NOEXCEPT;

    TComPtr(const TComPtr& other);
    TComPtr& operator =(const TComPtr& other);

    _ComInterface *Get() const;

    _ComInterface& operator *() const { Assert(_comObject); return *Get(); }
    _ComInterface *operator ->() const { Assert(_comObject); return Get(); }

    bool IsValid() const { return (nullptr != _comObject); }

    PPE_FAKEBOOL_OPERATOR_DECL() { return (!!_comObject); }

    _ComInterface **GetAddressOf();

    void Reset();
    void Reset(_ComInterface* comObject);

    u32 RefCount() const;
    void CheckThreadAccess() const;

    void Swap(TComPtr& other);

    template <typename U>
    void Steal(TComPtr<U>& other) {
        DecRefCountIFP();
        _comObject = other._comObject;
        other._comObject = nullptr;
    }

private:
    void IncRefCountIFP() const;
    void DecRefCountIFP() const;

    _ComInterface *_comObject;
};
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
void swap(const TComPtr<_Lhs>& lhs, const TComPtr<_Rhs>& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
template <typename T>
size_t ComRefCount(const TComPtr<T>& p) {
    return p.RefCount();
}
//----------------------------------------------------------------------------
template <typename T>
void ReleaseComRef(TComPtr<T>& p) {
    p.Reset();
}
//----------------------------------------------------------------------------
template <typename T>
void ReleaseComRef_AssertReachZero(TComPtr<T>& p) {
    if (!p.Get())
        return;

    Assert(p.RefCount() == 1);
    p.Reset();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "HAL/Windows/ComPtr-inl.h"
