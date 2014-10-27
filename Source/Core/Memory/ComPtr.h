#pragma once

#include "Core/Core.h"

#include "Core/Memory/RefPtr.h"
#include "Core/Meta/Assert.h"
#include "Core/Meta/ThreadResource.h"

/*
// Intrusive ref counting for Component Object Model (COM)
*/

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _ComInterface>
class ComPtr : private Meta::ThreadResource {
public:
    ComPtr();
    ComPtr(_ComInterface* comObject);
    ~ComPtr();

    ComPtr(ComPtr&& rvalue);
    ComPtr& operator =(ComPtr&& rvalue);

    ComPtr(const ComPtr& other);
    ComPtr& operator =(const ComPtr& other);

    _ComInterface *Get() const;

    _ComInterface& operator *() const { Assert(_comObject); return *Get(); }
    _ComInterface *operator ->() const { Assert(_comObject); return Get(); }

    operator _ComInterface *() const { return Get(); }

    _ComInterface **GetAddressOf();
    void Reset();

    u32 RefCount() const;
    void CheckThreadAccess() const;

    void Swap(ComPtr& other);

private:
    void IncRefCountIFP() const;
    void DecRefCountIFP() const;

    _ComInterface *_comObject;
};
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs>
void swap(const ComPtr<_Lhs>& lhs, const ComPtr<_Rhs>& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
template <typename T>
size_t ComRefCount(const ComPtr<T>& p) {
    return p.RefCount();
}
//----------------------------------------------------------------------------
template <typename T>
void ReleaseComRef(ComPtr<T>& p) {
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
