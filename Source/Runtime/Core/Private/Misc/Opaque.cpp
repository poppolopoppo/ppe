// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Misc/Opaque.h"

#include "Misc/OpaqueBuilder.h"

namespace PPE {
namespace Opaq {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t BlockSize(const value_init& init) NOEXCEPT {
    details::value_block_capacity capacity;
    capacity.Reserve<value_view>(1); // reserve space for root value_view, see NewBlock()
    std::visit(capacity, init);
    return capacity.Size;
}
//----------------------------------------------------------------------------
size_t BlockSize(const value_view& view) NOEXCEPT {
    details::value_block_capacity capacity;
    capacity.Reserve<value_view>(1); // reserve space for root value_view, see NewBlock()
    std::visit(capacity, view);
    return capacity.Size;
}
//----------------------------------------------------------------------------
value_block NewBlock(FAllocatorBlock block, const value_init& init) {
    details::value_block_inliner::slab slab{ block };
    // first value_view is embedded in block: TRelativeView<> can't be copied/moved!
    value_view& root = *slab.AllocateView<value_view>(1).data();
    std::visit(details::value_block_inliner{ root, slab }, init);
    return { block };
}
//----------------------------------------------------------------------------
value_block NewBlock(FAllocatorBlock block, const value_view& view) {
    details::value_block_inliner::slab slab{ block };
    // first value_view is embedded in block: TRelativeView<> can't be copied/moved!
    value_view& root = *slab.AllocateView<value_view>(1).data();
    std::visit(details::value_block_inliner{ root, slab }, view);
    return { block };
}
//----------------------------------------------------------------------------
Meta::TOptional<TPtrRef<const value_view>> XPath(const value_view& v, std::initializer_list<FStringView> path) NOEXCEPT {
    TPtrRef<const value_view> node{ &v };

    for (const FStringView& key : path) {
        const Meta::TOptionalReference<const object_view> obj{ std::get_if<object_view>(node.get()) };
        if (not obj)
            break;

        node.reset();
        for (const key_value_view& it : *obj) {
            if (it.key == key) {
                node = &it.value;
                break;
            }
        }

        if (not node.valid())
            break;
    }

    if (Likely(node.valid()))
        return *node;

    return Meta::TOptional<TPtrRef<const value_view>>{};
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const array_init& v) { return details::Print(oss, v); }
FWTextWriter& operator <<(FWTextWriter& oss, const array_init& v) { return details::Print(oss, v); }
FTextWriter& operator <<(FTextWriter& oss, const object_init& v) { return details::Print(oss, v); }
FWTextWriter& operator <<(FWTextWriter& oss, const object_init& v) { return details::Print(oss, v); }
FTextWriter& operator <<(FTextWriter& oss, const value_init& v) { return details::Print(oss, v); }
FWTextWriter& operator <<(FWTextWriter& oss, const value_init& v) { return details::Print(oss, v); }
FTextWriter& operator <<(FTextWriter& oss, const key_value_init& v) { return details::Print(oss, v); }
FWTextWriter& operator <<(FWTextWriter& oss, const key_value_init& v) { return details::Print(oss, v); }
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const array_view& v) { return details::Print(oss, v); }
FWTextWriter& operator <<(FWTextWriter& oss, const array_view& v) { return details::Print(oss, v); }
FTextWriter& operator <<(FTextWriter& oss, const object_view& v) { return details::Print(oss, v); }
FWTextWriter& operator <<(FWTextWriter& oss, const object_view& v) { return details::Print(oss, v); }
FTextWriter& operator <<(FTextWriter& oss, const value_view& v) { return details::Print(oss, v); }
FWTextWriter& operator <<(FWTextWriter& oss, const value_view& v) { return details::Print(oss, v); }
FTextWriter& operator <<(FTextWriter& oss, const key_value_view& v) { return details::Print(oss, v); }
FWTextWriter& operator <<(FWTextWriter& oss, const key_value_view& v) { return details::Print(oss, v); }
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const value_block& v) { return details::Print(oss, *v); }
FWTextWriter& operator <<(FWTextWriter& oss, const value_block& v) { return details::Print(oss, *v); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Opaq
} //!namespace PPE
