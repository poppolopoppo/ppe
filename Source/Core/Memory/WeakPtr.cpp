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
        Assert(prev == _weakPtrs->_prev);

        WeakPtrBase *const next = _weakPtrs->_next;

        *_weakPtrs->_pptr = nullptr;
        _weakPtrs->_prev = _weakPtrs->_next = nullptr;

        prev = _weakPtrs;
        _weakPtrs = next;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
