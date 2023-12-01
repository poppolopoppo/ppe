#pragma once

#include "Core_fwd.h"

#include "Allocator/Allocation.h"
#include "IO/StringView.h"
#include "IO/TextWriter_fwd.h"
#include "Memory/MemoryView.h"
#include "Memory/RelativeView.h"
#include "Misc/Function_fwd.h"
#include "Meta/Optional.h"

#include <initializer_list>
#include <variant>

// opaque data DSL
// https://godbolt.org/z/e98vcnYrb

namespace PPE {
namespace Opaq {
//----------------------------------------------------------------------------
// main wrapper struct forward declaration
//----------------------------------------------------------------------------
using default_allocator = ALLOCATOR(Opaq);
struct value_init;
struct key_value_init;
struct value_view;
struct key_value_view;
//----------------------------------------------------------------------------
// trivial opaque types
//----------------------------------------------------------------------------
using boolean = bool;
using integer = i64;
using uinteger = u64;
using floating_point = double;
//----------------------------------------------------------------------------
// *_init variants are used for inline definition
//----------------------------------------------------------------------------
using string_init = FStringView;
using wstring_init = FWStringView;
using array_init = TMemoryView<const value_init>;
using object_init = TMemoryView<const key_value_init>;
//----------------------------------------------------------------------------
// can provide a format function to generate non-trivial strings
//----------------------------------------------------------------------------
using string_format = TFunction<void(FTextWriter&)>;
using wstring_format = TFunction<void(FWTextWriter&)>;
//----------------------------------------------------------------------------
// *_view variants are packed inside a single dynamically allocated block
//----------------------------------------------------------------------------
using string_view = TRelativeView<const char>;
using wstring_view = TRelativeView<const wchar_t>;
using array_view = TRelativeView<const value_view>;
using object_view = TRelativeView<const key_value_view>;
//----------------------------------------------------------------------------
// value_init: for inline declaration
//----------------------------------------------------------------------------
namespace details {
using value_init_variant = std::variant<
    boolean, integer, uinteger, floating_point,
    string_init, wstring_init,
    array_init, object_init,
    // format strings dynamically (/!\ prefer static strings when possible)
    string_format, wstring_format,
    // forward an inlined value_view to a value_init
    TPtrRef<const array_view>,
    TPtrRef<const object_view>>;
} //!details
struct value_init : details::value_init_variant {
    using details::value_init_variant::value_init_variant;

    // explicit trivial type promotion to storage format:
    // handling all variants would drastically rise code complexity,
    // and storage wise the stride of value_init is already majored by the largest type of the variant.
    CONSTEXPR value_init(i8  i) : value_init(static_cast<integer>(i)) {}
    CONSTEXPR value_init(i16 i) : value_init(static_cast<integer>(i)) {}
    CONSTEXPR value_init(i32 i) : value_init(static_cast<integer>(i)) {}

    CONSTEXPR value_init(u8  u) : value_init(static_cast<uinteger>(u)) {}
    CONSTEXPR value_init(u16 u) : value_init(static_cast<uinteger>(u)) {}
    CONSTEXPR value_init(u32 u) : value_init(static_cast<uinteger>(u)) {}

    CONSTEXPR value_init(float f) : value_init(static_cast<floating_point>(f)) {}
};
struct key_value_init { string_init key; value_init value; };
//----------------------------------------------------------------------------
// value_view: one contiguous memory block with every value packed inside
//----------------------------------------------------------------------------
namespace details {
using value_view_variant = std::variant<
    std::monostate,
    boolean, integer, uinteger, floating_point,
    string_view, wstring_view,
    array_view, object_view>;
} //!details
struct value_view : details::value_view_variant {
    using details::value_view_variant::value_view_variant;
    PPE_FAKEBOOL_OPERATOR_DECL() { return not details::value_view_variant::valueless_by_exception(); }
};
struct key_value_view { string_view key; value_view value; };
//----------------------------------------------------------------------------
PPE_ASSUME_TYPE_AS_POD(string_init);
PPE_ASSUME_TYPE_AS_POD(wstring_init);
PPE_ASSUME_TYPE_AS_POD(array_init);
PPE_ASSUME_TYPE_AS_POD(object_init);
PPE_ASSUME_TYPE_AS_POD(value_init);
PPE_ASSUME_TYPE_AS_POD(key_value_init);
PPE_ASSUME_TYPE_AS_POD(string_view);
PPE_ASSUME_TYPE_AS_POD(wstring_view);
PPE_ASSUME_TYPE_AS_POD(array_view);
PPE_ASSUME_TYPE_AS_POD(object_view);
PPE_ASSUME_TYPE_AS_POD(value_view);
PPE_ASSUME_TYPE_AS_POD(key_value_view);
//----------------------------------------------------------------------------
// Create a value_view from a value_init
//----------------------------------------------------------------------------
struct value_block {
    FAllocatorBlock block;

    TPtrRef<value_view> Value() { return static_cast<value_view*>(block.Data); }
    TPtrRef<const value_view> Value() const { return static_cast<const value_view*>(block.Data); }

    value_view& operator *() { return Value(); }
    const value_view& operator *() const { return Value(); }

    value_view* operator ->() { return Value(); }
    const value_view* operator ->() const { return Value(); }
};
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API size_t BlockSize(const value_init& init) NOEXCEPT;
NODISCARD PPE_CORE_API size_t BlockSize(const value_view& view) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API value_block NewBlock(FAllocatorBlock block, const value_init& init);
NODISCARD PPE_CORE_API value_block NewBlock(FAllocatorBlock block, const value_view& view);
//----------------------------------------------------------------------------
template <typename _Allocator = default_allocator>
NODISCARD value_block NewBlock(const value_init& v) {
    return NewBlock(TStaticAllocator<_Allocator>::Allocate(BlockSize(v)), v);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
NODISCARD value_block NewBlock(_Allocator& allocator, const value_init& v) {
    return NewBlock(TAllocatorTraits<_Allocator>::Allocate(allocator, BlockSize(v)), v);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
NODISCARD value_block NewBlock(const _Allocator& allocator, const value_init& v) {
    return NewBlock(TAllocatorTraits<_Allocator>::Allocate(allocator, BlockSize(v)), v);
}
//----------------------------------------------------------------------------
template <typename _Allocator = default_allocator>
void DeleteBlock(const value_block& v) {
    STATIC_ASSERT(Meta::is_pod_v<value_view>);
    TStaticAllocator<_Allocator>::Deallocate(v.block);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void DeleteBlock(_Allocator& allocator, const value_block& v) {
    STATIC_ASSERT(Meta::is_pod_v<value_view>);
    TAllocatorTraits<_Allocator>::Deallocate(allocator, v.block);
}
//----------------------------------------------------------------------------
// Lookup value_view for expected named properties
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API Meta::TOptional<TPtrRef<const value_view>> XPath(const value_view& v, std::initializer_list<FStringView> path) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T>
NODISCARD Meta::TOptionalReference<T> XPathAs(const value_view& v, std::initializer_list<FStringView> path) NOEXCEPT {
    if (Meta::TOptional<TPtrRef<const value_view>> result = XPath(v, path); result.has_value())
        return std::get_if<T>(result.value());
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, const array_init& v);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, const array_init& v);
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, const object_init& v);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, const object_init& v);
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, const value_init& v);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, const value_init& v);
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, const key_value_init& v);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, const key_value_init& v);
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, const array_view& v);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, const array_view& v);
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, const object_view& v);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, const object_view& v);
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, const value_view& v);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, const value_view& v);
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, const key_value_view& v);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, const key_value_view& v);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Opaq
} //!namespace PPE
