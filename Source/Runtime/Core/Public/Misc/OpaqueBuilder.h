#pragma once

#include "Core_fwd.h"

#include "Misc/Opaque.h"

#include "Container/Vector.h"
#include "Container/Stack.h"
#include "IO/String.h"

#include <variant>

namespace PPE {
namespace Opaq {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// dynamic value allocator can be overriden
//----------------------------------------------------------------------------
template <typename _Allocator = default_allocator>
struct value;
template <typename _Allocator = default_allocator>
struct key_value;
//----------------------------------------------------------------------------
// trivial opaque types
//----------------------------------------------------------------------------
template <typename _Allocator = default_allocator>
using string = FString;
template <typename _Allocator = default_allocator>
using wstring = FWString;
template <typename _Allocator = default_allocator>
using array = VECTOR(Opaq, value<_Allocator>);
template <typename _Allocator = default_allocator>
using object = VECTOR(Opaq, key_value<_Allocator>);
//----------------------------------------------------------------------------
// value_init: for inline declaration
//----------------------------------------------------------------------------
namespace details {
template <typename _Allocator>
using value_variant = std::variant<
    std::monostate,
    boolean, integer, uinteger, floating_point,
    string<_Allocator>, wstring<_Allocator>,
    array<_Allocator>, object<_Allocator>>;
} //!details
template <typename _Allocator>
struct value : details::value_variant<_Allocator> {
    using details::value_variant<_Allocator>::value_variant;

    // explicit trivial type promotion to storage format:
    // handling all variants would drastically rise code complexity,
    // and storage wise the stride of value_init is already majored by the largest type of the variant.
    CONSTEXPR value(i8  i) : value(static_cast<integer>(i)) {}
    CONSTEXPR value(i16 i) : value(static_cast<integer>(i)) {}
    CONSTEXPR value(i32 i) : value(static_cast<integer>(i)) {}

    CONSTEXPR value(u8  u) : value(static_cast<uinteger>(u)) {}
    CONSTEXPR value(u16 u) : value(static_cast<uinteger>(u)) {}
    CONSTEXPR value(u32 u) : value(static_cast<uinteger>(u)) {}

    CONSTEXPR value(float f) : value(static_cast<floating_point>(f)) {}

    PPE_FAKEBOOL_OPERATOR_DECL() { return not std::holds_alternative<std::monostate>(*this); }
};
template <typename _Allocator>
struct key_value { string<_Allocator> key; value<_Allocator> value; };
//----------------------------------------------------------------------------
// Create a value_view from a dynamic value<>
//----------------------------------------------------------------------------
template <typename _Allocator>
NODISCARD inline size_t BlockSize(const value<_Allocator>& value) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename _Allocator>
NODISCARD inline value_block NewBlock(FAllocatorBlock block, const value<_Allocator>& value);
//----------------------------------------------------------------------------
template <typename _Allocator = default_allocator, typename _ValueAllocator = default_allocator>
NODISCARD inline value_block NewBlock(const value<_ValueAllocator>& value) {
    return NewBlock(TStaticAllocator<_Allocator>::Allocate(BlockSize(value)), value);
}
//----------------------------------------------------------------------------
// Create a dynamic value<> from a value_init/value_view
//----------------------------------------------------------------------------
template <typename _Allocator = default_allocator>
NODISCARD inline value<_Allocator> NewValue(const value_init& init);
template <typename _Allocator>
NODISCARD inline value<_Allocator> NewValue(_Allocator&& rallocator, const value_init& init);
template <typename _Allocator>
NODISCARD inline value<_Allocator> NewValue(const _Allocator& allocator, const value_init& init);
//----------------------------------------------------------------------------
template <typename _Allocator = default_allocator>
NODISCARD inline value<_Allocator> NewValue(const value_view& view);
template <typename _Allocator>
NODISCARD inline value<_Allocator> NewValue(_Allocator&& rallocator, const value_view& view);
template <typename _Allocator>
NODISCARD inline value<_Allocator> NewValue(const _Allocator& allocator, const value_view& view);
//----------------------------------------------------------------------------
// Create a dynamic value<> interactively
//----------------------------------------------------------------------------
class IBuilder {
public:
    virtual ~IBuilder() = default;

    virtual size_t BlockSize() const NOEXCEPT = 0;
    virtual value_block ToValueBlock(FAllocatorBlock block) = 0;

    virtual void Write(boolean v) = 0;
    virtual void Write(integer v) = 0;
    virtual void Write(uinteger v) = 0;
    virtual void Write(floating_point v) = 0;
    virtual void Write(string_init v) = 0;
    virtual void Write(wstring_init v) = 0;
    virtual void Write(const string_format& v) = 0;
    virtual void Write(const wstring_format& v) = 0;
    virtual void Write(FString&& v) = 0;
    virtual void Write(FWString&& v) = 0;

    virtual void BeginArray(size_t capacity = 0) = 0;
    virtual void EndArray() = 0;

    virtual void BeginObject(size_t capacity = 0) = 0;
    virtual void EndObject() = 0;

    virtual void BeginKeyValue(FString&& rkey) = 0;
    virtual void EndKeyValue() = 0;

    // template helpers for closures:

    template <typename _Functor, decltype(std::declval<_Functor&>()())* = nullptr>
    void Array(_Functor&& innerScope, size_t capacity = 0) {
        BeginArray(capacity);
        innerScope();
        EndArray();
    }

    template <typename _It>
    void Array(_It first, _It last) {
        BeginArray(std::distance(first, last));
        std::for_each(first, last, MakePtrRef(this));
        EndArray();
    }

    template <typename _Functor, decltype(std::declval<_Functor&>()())* = nullptr>
    void Object(_Functor&& innerScope, size_t capacity = 0) {
        BeginObject(capacity);
        innerScope();
        EndObject();
    }

    template <typename _It>
    void Object(_It first, _It last) {
        BeginObject(std::distance(first, last));
        std::for_each(first, last, MakePtrRef(this));
        EndObject();
    }

    template <typename _Functor, decltype(std::declval<_Functor&>()())* = nullptr>
    void KeyValue(FString&& key, _Functor&& innerScope) {
        BeginKeyValue(std::move(key));
        innerScope();
        EndKeyValue();
    }

    void BeginKeyValue(string_init key) {
        BeginKeyValue(FString(key));
    }

    // visitor

    void operator ()(std::monostate) {}

    void operator ()(boolean v) { Write(v); }
    void operator ()(integer v) { Write(v); }
    void operator ()(uinteger v) { Write(v); }
    void operator ()(floating_point v) { Write(v); }

    void operator ()(string_init v) { Write(v); }
    void operator ()(wstring_init v) { Write(v); }
    void operator ()(const string_view& v) { Write(string_init(v.MakeView())); }
    void operator ()(const wstring_view& v) { Write(wstring_init(v.MakeView())); }
    void operator ()(const string_format& v) { Write(v); }
    void operator ()(const wstring_format& v) { Write(v); }

    void operator ()(array_init v) { Array(v.begin(), v.end()); }
    void operator ()(const array_view& v) { Array(v.begin(), v.end()); }
    void operator ()(TPtrRef<const array_view> v) { operator ()(*v); }

    void operator ()(object_init v) { Object(v.begin(), v.end()); }
    void operator ()(const object_view& v) { Object(v.begin(), v.end()); }
    void operator ()(TPtrRef<const object_view> v) { operator ()(*v); }

    void operator ()(const value_init& v) { std::visit(*this, v); }
    void operator ()(const value_view& v) { std::visit(*this, v); }

    void operator ()(const key_value_init& v) {
        BeginKeyValue(v.key);
        operator ()(v.value);
        EndKeyValue();
    }
    void operator ()(const key_value_view& v) {
        BeginKeyValue(string_init(v.key.MakeView()));
        operator ()(v.value);
        EndKeyValue();
    }

    template <typename _Al>
    void operator ()(const array<_Al>& v) { Array(v.begin(), v.end()); }
    template <typename _Al>
    void operator ()(const object<_Al>& v) { Object(v.begin(), v.end()); }
    template <typename _Al>
    void operator ()(const value<_Al>& v) { std::visit(*this, v); }
    template <typename _Al>
    void operator ()(const key_value<_Al>& v) {
        BeginKeyValue(string_init(v.key));
        operator ()(v.value);
        EndKeyValue();
    }
};
//----------------------------------------------------------------------------
template <typename _Allocator = default_allocator>
class TBuilder final : private _Allocator, public IBuilder {
public:
    using string_type = string<_Allocator>;
    using wstring_type = wstring<_Allocator>;
    using array_type = array<_Allocator>;
    using object_type = object<_Allocator>;
    using value_type = value<_Allocator>;
    using key_value_type = key_value<_Allocator>;

    explicit TBuilder(TPtrRef<value_type> dst) NOEXCEPT { Reset(dst); }
    virtual ~TBuilder() override;

    TBuilder(TPtrRef<value_type> dst, const _Allocator& allocator) : _Allocator(allocator) { Reset(dst); }
    TBuilder(TPtrRef<value_type> dst, _Allocator&& rallocator) NOEXCEPT : _Allocator(std::move(rallocator)) { Reset(dst); }

    TBuilder(const TBuilder&) = delete;
    TBuilder& operator =(const TBuilder&) = delete;

    TBuilder(TBuilder&& rvalue) NOEXCEPT
    :   _Allocator(std::move(rvalue.Allocator_()))
    ,   _edits(std::move(rvalue._edits))
    {}
    TBuilder& operator =(TBuilder&&) = delete;

    void Reset(TPtrRef<value_type> dst);

    virtual size_t BlockSize() const NOEXCEPT override final;
    virtual value_block ToValueBlock(FAllocatorBlock block) override final;

    virtual void Write(boolean v) override final { SetValue_(v); }
    virtual void Write(integer v) override final { SetValue_(v); }
    virtual void Write(uinteger v) override final { SetValue_(v); }
    virtual void Write(floating_point v) override final { SetValue_(v); }
    virtual void Write(string_init v) override final { SetValue_(string_type(v)); }
    virtual void Write(wstring_init v) override final { SetValue_(wstring_type(v)); }
    virtual void Write(const string_format& v) override final;
    virtual void Write(const wstring_format& v) override final;
    virtual void Write(FString&& v) override final { SetValue_(std::move(v)); }
    virtual void Write(FWString&& v) override final { SetValue_(std::move(v)); }

    virtual void BeginArray(size_t capacity = 0) override final;
    virtual void EndArray() override final;

    virtual void BeginObject(size_t capacity = 0) override final;
    virtual void EndObject() override final;

    virtual void BeginKeyValue(FString&& rkey) override final;
    virtual void EndKeyValue() override final;

    using IBuilder::KeyValue;
    void KeyValue(string_type&& rkey, value_type&& rvalue) {
        BeginKeyValue(std::move(rkey));
        SetValue_(std::move(rvalue));
        EndKeyValue();
    }

private:
    _Allocator& Allocator_() { return (*this); }

    value_type& Head_() NOEXCEPT;
    value_type& SetValue_(value_type&& rvalue);

    TFixedSizeStack<TPtrRef<value_type>, 8> _edits;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const array<_Allocator>& v);
template <typename _Char, typename _Allocator>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const object<_Allocator>& v);
template <typename _Char, typename _Allocator>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const value<_Allocator>& v);
template <typename _Char, typename _Allocator>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const key_value<_Allocator>& v);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Opaq
} //!namespace PPE

#include "Misc/OpaqueBuilder-inl.h"
