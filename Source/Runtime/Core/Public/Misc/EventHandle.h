#pragma once

#include "Core_fwd.h"

#include "Container/SparseArray_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// #TODO: add debug checks ensuring that FEventHandle is used with the correct event
//----------------------------------------------------------------------------
class FEventHandle {
public:
    CONSTEXPR FEventHandle() NOEXCEPT : _id(0) {}

    explicit FEventHandle(FSparseDataId id) NOEXCEPT
    :   _id(id) {
        Assert(_id);
    }

    FEventHandle(const FEventHandle&) = delete;
    FEventHandle& operator =(const FEventHandle&) = delete;

    FEventHandle(FEventHandle&& rvalue) NOEXCEPT
    :   FEventHandle() {
        Swap(rvalue);
    }

    FEventHandle& operator =(FEventHandle&& rvalue) NOEXCEPT {
        Assert(0 == _id); // don't support assigning to initialized handle !
        Swap(rvalue);
        return (*this);
    }

#if USE_PPE_ASSERT
    ~FEventHandle() NOEXCEPT {
        Assert_NoAssume(0 == _id);
    }
#endif

    PPE_FAKEBOOL_OPERATOR_DECL() { return (_id ? this : nullptr); }

    FSparseDataId Forget() NOEXCEPT {
        const FSparseDataId result = _id;
        _id = 0; // won't assert on destruction, use wisely ;O
        return result;
    }

    void Swap(FEventHandle& other) {
        std::swap(_id, other._id);
    }

    inline friend void swap(FEventHandle& lhs, FEventHandle& rhs) {
        lhs.Swap(rhs);
    }

private:
    FSparseDataId _id;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
