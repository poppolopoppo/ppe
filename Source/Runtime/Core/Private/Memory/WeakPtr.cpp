#include "stdafx.h"

#include "WeakPtr.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWeakAndRefCountable::~FWeakAndRefCountable() {
    THIS_THREADRESOURCE_CHECKACCESS();

    FWeakPtrBase *prev = nullptr;
    while (_weakPtrs) {
        Assert(_weakPtrs);
        Assert(prev == _weakPtrs->_prev);

        FWeakPtrBase *const next = _weakPtrs->_next;

        _weakPtrs->_weakAndRefCountable = nullptr;
        _weakPtrs->_prev = _weakPtrs->_next = nullptr;

        prev = _weakPtrs;
        _weakPtrs = next;
    }
}
//----------------------------------------------------------------------------
void FWeakPtrBase::set_(FWeakAndRefCountable* ptr) {
    if (_weakAndRefCountable == ptr)
        return;

    if (_weakAndRefCountable) {
        THREADRESOURCE_CHECKACCESS(_weakAndRefCountable);

        if (_prev) _prev->_next = _next;
        if (_next) _next->_prev = _prev;

        if (this == _weakAndRefCountable->_weakPtrs) {
            Assert(nullptr == _prev);
            _weakAndRefCountable->_weakPtrs = _next;
        }

        _prev = _next = nullptr;
        _weakAndRefCountable = nullptr;
    }

    Assert(nullptr == _weakAndRefCountable);
    Assert(nullptr == _next);
    Assert(nullptr == _prev);

    if (ptr) {
        THREADRESOURCE_CHECKACCESS(ptr);

        _next = ptr->_weakPtrs;
        if (ptr->_weakPtrs)
            ptr->_weakPtrs->_prev = this;

        ptr->_weakPtrs = this;
        _weakAndRefCountable = ptr;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
