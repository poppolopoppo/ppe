#include "stdafx.h"

#include "WeakPtr.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
WeakAndRefCountable::~WeakAndRefCountable() {
    THIS_THREADRESOURCE_CHECKACCESS();
    
    WeakPtrBase *prev = nullptr;
    while (_weakPtrs) {
        Assert(_weakPtrs);
        Assert(this == _weakPtrs->_ptr);
        Assert(prev == _weakPtrs->_prev);

        WeakPtrBase *const next = _weakPtrs->_next;

        _weakPtrs->_ptr = nullptr;
        _weakPtrs->_prev = _weakPtrs->_next = nullptr;

        _weakPtrs = next;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void WeakPtrBase::set_(const WeakAndRefCountable *ptr) {
    if (_ptr == ptr)
        return;

    if (_ptr) {
        THREADRESOURCE_CHECKACCESS(_ptr);
        
        if (_prev) _prev->_next = _next;
        if (_next) _next->_prev = _prev;

        if (this == _ptr->_weakPtrs) {
            Assert(nullptr == _prev);
            _ptr->_weakPtrs = _next;
        }

        _prev = _next = nullptr;
        _ptr = nullptr;
    }
    
    Assert(nullptr == _ptr);
    Assert(nullptr == _next);
    Assert(nullptr == _prev);

    if (ptr) {
        THREADRESOURCE_CHECKACCESS(ptr);

        _next = ptr->_weakPtrs;
        if (ptr->_weakPtrs)
            ptr->_weakPtrs->_prev = this;

        ptr->_weakPtrs = this;
        _ptr = ptr;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
