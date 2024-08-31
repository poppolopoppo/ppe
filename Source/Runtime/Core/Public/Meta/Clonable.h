#pragma once

#include "Core_fwd.h"

#include "Allocator/AllocatorBlock.h"

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct IClonable {
    virtual ~IClonable() = default;
    virtual void ConstructCopy(FAllocatorBlock blk) const = 0;
    virtual void ConstructMove(FAllocatorBlock blk) NOEXCEPT = 0;
};
//----------------------------------------------------------------------------
template <typename T>
struct TClonable : IClonable {
    virtual void ConstructCopy(FAllocatorBlock blk) const override {
        INPLACE_NEW(blk.Data, T)(*this);
    }
    virtual void ConstructMove(FAllocatorBlock blk) NOEXCEPT override {
        INPLACE_NEW(blk.Data, T)(std::move(*this));
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
