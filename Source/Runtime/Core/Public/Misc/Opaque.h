#pragma once

#include "Core_fwd.h"

#include "Misc/Opaque_fwd.h"

#include "IO/ConstChar.h"
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
// can provide a format function to generate non-trivial strings
//----------------------------------------------------------------------------
using string_format = TTinyFunction<void(FTextWriter&)>;
using wstring_format = TTinyFunction<void(FWTextWriter&)>;
//----------------------------------------------------------------------------
template <typename T>
string_format Format(const T& value) {
    return [&value](FTextWriter& oss) { oss << value; };
}
template <typename T>
wstring_format FormatW(const T& value) {
    return [&value](FWTextWriter& oss) { oss << value; };
}
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
    nil,
    boolean, integer, uinteger, floating_point,
    string_init, wstring_init,
    array_init, object_init,
    // string literals are static strings: we do not copy them
    string_literal, wstring_literal,
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

    // need to disambiguate between TBasicStringView<> and TBasicStringLiteral<>
    template <typename _Char, size_t _Len>
    CONSTEXPR value_init(const _Char (&staticString)[_Len]) : value_init(TBasicStringLiteral<_Char>(staticString)) {}

    bool Nil() const {
        return std::holds_alternative<nil>(*this);
    }

    void Reset() {
        details::value_init_variant::operator =(nil{});
    }
};
struct key_value_init {
    string_literal key;
    value_init value;

    friend bool operator ==(const key_value_init& lhs, const key_value_init& rhs) NOEXCEPT {
        return (lhs.key == rhs.key && lhs.value == rhs.value);
    }
    friend bool operator !=(const key_value_init& lhs, const key_value_init& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }
};
//----------------------------------------------------------------------------
// value_view: one contiguous memory block with every value packed inside
//----------------------------------------------------------------------------
namespace details {
using value_view_variant = std::variant<
    nil,
    boolean, integer, uinteger, floating_point,
    string_view, wstring_view,
    array_view, object_view
>;
} //!details
struct value_view : details::value_view_variant {
    using details::value_view_variant::value_view_variant;

    PPE_FAKEBOOL_OPERATOR_DECL() { return not std::holds_alternative<nil>(*this); }

    bool Nil() const {
        return std::holds_alternative<nil>(*this);
    }

    void Reset() {
        details::value_view_variant::operator =(nil{});
    }
};
struct key_value_view {
    string_view key;
    value_view value;

    friend bool operator ==(const key_value_view& lhs, const key_value_view& rhs) NOEXCEPT {
        return (lhs.key.MakeView() == rhs.key.MakeView() && lhs.value == rhs.value);
    }
    friend bool operator !=(const key_value_view& lhs, const key_value_view& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }
};
//----------------------------------------------------------------------------
PPE_ASSUME_TYPE_AS_POD(string_init);
PPE_ASSUME_TYPE_AS_POD(wstring_init);
PPE_ASSUME_TYPE_AS_POD(array_init);
PPE_ASSUME_TYPE_AS_POD(object_init);
PPE_ASSUME_TYPE_AS_POD(value_init);
PPE_ASSUME_TYPE_AS_POD(key_value_init);
PPE_ASSUME_TYPE_AS_POD(string_view);
PPE_ASSUME_TYPE_AS_POD(wstring_view);
PPE_ASSUME_TYPE_AS_POD(string_literal);
PPE_ASSUME_TYPE_AS_POD(wstring_literal);
PPE_ASSUME_TYPE_AS_POD(string_external);
PPE_ASSUME_TYPE_AS_POD(wstring_external);
PPE_ASSUME_TYPE_AS_POD(array_view);
PPE_ASSUME_TYPE_AS_POD(object_view);
PPE_ASSUME_TYPE_AS_POD(value_view);
PPE_ASSUME_TYPE_AS_POD(key_value_view);
//----------------------------------------------------------------------------
// Create a value_view from a value_init
//----------------------------------------------------------------------------
struct value_block {
    FAllocatorBlock alloc;

    PPE_FAKEBOOL_OPERATOR_DECL() { return !!alloc; }

    TPtrRef<value_view> Value() { return static_cast<value_view*>(alloc.Data); }
    TPtrRef<const value_view> Value() const { return static_cast<const value_view*>(alloc.Data); }

    NODISCARD FAllocatorBlock Reset() {
        return std::move(alloc);
    }

    value_view& operator *() { return Value(); }
    const value_view& operator *() const { return Value(); }

    value_view* operator ->() { return Value(); }
    const value_view* operator ->() const { return Value(); }
};
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API size_t BlockSize(const value_init& init) NOEXCEPT;
NODISCARD PPE_CORE_API size_t BlockSize(const value_view& view) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API value_block NewBlock(FAllocatorBlock alloc, const value_init& init);
NODISCARD PPE_CORE_API value_block NewBlock(FAllocatorBlock alloc, const value_view& view);
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
NODISCARD value_block NewBlock(_Allocator& allocator, const value_view& v) {
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
    TStaticAllocator<_Allocator>::Deallocate(v.alloc);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void DeleteBlock(_Allocator& allocator, const value_block& v) {
    STATIC_ASSERT(Meta::is_pod_v<value_view>);
    TAllocatorTraits<_Allocator>::Deallocate(allocator, v.alloc);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void DeleteBlock(_Allocator& allocator, value_block& v) {
    STATIC_ASSERT(Meta::is_pod_v<value_view>);
    TAllocatorTraits<_Allocator>::Deallocate(allocator, v.Reset());
}
//----------------------------------------------------------------------------
// Templated helper for value_block to associate a static allocator
//----------------------------------------------------------------------------
template <typename _Allocator>
class TValueBlock : private _Allocator {
public:
    using allocator_type = _Allocator;
    using allocator_traits = TAllocatorTraits<_Allocator>;

    TValueBlock() = default;

    ~TValueBlock() {
        Reset();
    }

    explicit TValueBlock(allocator_type&& ralloc) : _Allocator(std::move(ralloc)) {}
    explicit TValueBlock(const allocator_type& alloc) : _Allocator(alloc) {}

    explicit TValueBlock(const value_init& v) : Block(NewBlock(Allocator(), v)) {}
    explicit TValueBlock(const value_view& v) : Block(NewBlock(Allocator(), v)) {}

    TValueBlock(const value_init& v, allocator_type&& ralloc) : _Allocator(std::move(ralloc)), Block(NewBlock(Allocator(), v)) {}
    TValueBlock(const value_init& v, const allocator_type& alloc) : _Allocator(alloc), Block(NewBlock(Allocator(), v)) {}

    TValueBlock(const value_view& v, allocator_type&& ralloc) : _Allocator(std::move(ralloc)), Block(NewBlock(Allocator(), v)) {}
    TValueBlock(const value_view& v, const allocator_type& alloc) : _Allocator(alloc), Block(NewBlock(Allocator(), v)) {}

    TValueBlock(TValueBlock&& rvalue) NOEXCEPT
    :   _Allocator(std::move(rvalue.Allocator()))
    ,   Block(std::move(rvalue.Block))
    {}
    TValueBlock& operator =(TValueBlock&& rvalue) NOEXCEPT {
        if (MoveAllocatorBlock(&allocator_traits::Get(Allocator()), allocator_traits::Get(rvalue.Allocator()), rvalue.Block.alloc)) {
            Reset();
            Block.alloc = rvalue.Block.alloc.Reset();
        }
        else {
            Assign(rvalue.Value());
        }
        return (*this);
    }

    TValueBlock(const TValueBlock& ) = delete;
    TValueBlock& operator =(const TValueBlock& ) = delete;

    value_block Block;

    PPE_FAKEBOOL_OPERATOR_DECL() { return !!Block; }

    _Allocator& Allocator() { return (*this); }
    const _Allocator& Allocator() const { return (*this); }

    TPtrRef<value_view> Value() { return Block.Value(); }
    TPtrRef<const value_view> Value() const { return Block.Value(); }

    void Assign(const value_init& v) {
        Reset();
        Block = NewBlock(Allocator(), v);
    }

    void Assign(const value_view& v) {
        Reset();
        Block = NewBlock(Allocator(), v);
    }

    void Reset() {
        if (Block.alloc)
            DeleteBlock(Allocator(), Block);
        Assert_NoAssume(not Block.alloc);
    }

    value_view& operator *() { return Value(); }
    const value_view& operator *() const { return Value(); }

    value_view* operator ->() { return Value(); }
    const value_view* operator ->() const { return Value(); }
};
//----------------------------------------------------------------------------
// Lookup value_view for expected named properties
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API Meta::TOptional<TPtrRef<const value_view>> XPath(const object_view& o, FStringView key) NOEXCEPT;
NODISCARD PPE_CORE_API Meta::TOptional<TPtrRef<const value_view>> XPath(const value_view& v, std::initializer_list<FStringView> path) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T>
NODISCARD Meta::TOptionalReference<const T> XPathAs(const object_view& o, FStringView key) NOEXCEPT {
    if (Meta::TOptional<TPtrRef<const value_view>> result = XPath(o, key); result.has_value())
        return std::get_if<T>(result->get());
    return nullptr;
}
//----------------------------------------------------------------------------
template <typename T>
NODISCARD Meta::TOptionalReference<const T> XPathAs(const value_view& v, std::initializer_list<FStringView> path) NOEXCEPT {
    if (Meta::TOptional<TPtrRef<const value_view>> result = XPath(v, path); result.has_value())
        return std::get_if<T>(result->get());
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
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, const value_block& v);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, const value_block& v);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Opaq
} //!namespace PPE
