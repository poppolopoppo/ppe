#include "stdafx.h"

#include "WeakPtr.h"

namespace Core {
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
