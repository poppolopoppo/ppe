#pragma once

#include "Core/Core.h"

#include "Core/Memory/RefPtr.h"
#include "Core/Meta/Assert.h"
#include "Core/Meta/ThreadResource.h"

/*
// Intrusive ref counting for TComponent Object FModel (COM)
*/

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _ComInterface>
class TComPtr : private Meta::FThreadResource {
public:
    TComPtr();
    ~TComPtr();

    TComPtr(_ComInterface* comObject);

    TComPtr(TComPtr&& rvalue);
    TComPtr& operator =(TComPtr&& rvalue);

    TComPtr(const TComPtr& other);
    TComPtr& operator =(const TComPtr& other);

    _ComInterface *Get() const;

    _ComInterface& operator *() const { Assert(_comObject); return *Get(); }
    _ComInterface *operator ->() const { Assert(_comObject); return Get(); }

    operator _ComInterface *() const { return Get(); }

    _ComInterface **GetAddressOf();
    void Reset();

    u32 RefCount() const;
    void CheckThreadAccess() const;

    void Swap(TComPtr& other);

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
    if (!p.Get())
        return;

    Assert(p.RefCount() == 1);
    p.Reset();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Memory/ComPtr-inl.h"
