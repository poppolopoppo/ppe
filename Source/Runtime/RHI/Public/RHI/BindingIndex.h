#pragma once

#include "RHI_fwd.h"

#include "Memory/HashFunctions.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FBindingIndex {
public:
    using index_t = u16;

    FBindingIndex() = default;

    explicit FBindingIndex(size_t perResource, size_t unique) NOEXCEPT
    :   _perResource(checked_cast<index_t>(perResource))
    ,   _unique(checked_cast<index_t>(unique))
    {}

    index_t PerResource() const { return _unique; }
    index_t Unique() const { return _unique; }

    index_t GLBinding() const { return _perResource; }
    index_t DXBinding() const { return _perResource; }

    index_t VKBinding() const { return _unique; }
    index_t CLBinding() const { return _unique; }
    index_t SWBinding() const { return _unique; }

    bool operator ==(const FBindingIndex& other) const { return (_perResource == other._perResource && _unique == other._unique); }
    bool operator !=(const FBindingIndex& other) const { return (not operator ==(other)); }

    friend hash_t hash_value(const FBindingIndex& it) {
        STATIC_ASSERT(sizeof(it) == 2 * sizeof(index_t));
        return hash_as_pod(it);
    }

private:
    index_t _perResource = INDEX_NONE; // resource dependend index, may be optimized to minimize resource switches between pipelines
    index_t _unique = INDEX_NONE; // resource unique index in current pipeline
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
