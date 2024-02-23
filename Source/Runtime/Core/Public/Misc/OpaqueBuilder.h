#pragma once

#include "Core_fwd.h"

#include "Misc/Opaque.h"

#include "Container/Stack.h"
#include "Container/Vector.h"
#include "IO/Text.h"
#include "IO/TextMemoization.h"

#include <variant>

namespace PPE {
namespace Opaq {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// trivial opaque types
//----------------------------------------------------------------------------
template <typename _Allocator = default_allocator>
using string = TBasicText<char, _Allocator>;
template <typename _Allocator = default_allocator>
using wstring = TBasicText<wchar_t, _Allocator>;
template <typename _Allocator = default_allocator>
using array = TVector<value<_Allocator>, _Allocator>;
template <typename _Allocator = default_allocator>
using object = TVector<key_value<_Allocator>, _Allocator>;
//----------------------------------------------------------------------------
// value_init: for inline declaration
//----------------------------------------------------------------------------
namespace details {
template <typename _Allocator>
using value_variant = std::variant<
    nil,
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

    PPE_FAKEBOOL_OPERATOR_DECL() { return not std::holds_alternative<nil>(*this); }

    bool Nil() const {
        return std::holds_alternative<nil>(*this);
    }

    void Reset() {
        details::value_variant<_Allocator>::operator =(nil{});
    }
};
template <typename _Allocator>
struct key_value {
    string<_Allocator> key;
    value<_Allocator> value;

    friend bool operator ==(const key_value& lhs, const key_value& rhs) NOEXCEPT {
        return (lhs.key == rhs.key && lhs.value == rhs.value);
    }
    friend bool operator !=(const key_value& lhs, const key_value& rhs) NOEXCEPT {
        return (not operator ==(lhs, rhs));
    }
};
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
// Lookup value_view for expected named properties
//----------------------------------------------------------------------------
template <typename _Allocator>
inline NODISCARD Meta::TOptional<TPtrRef<value<_Allocator>>> XPath(object<_Allocator>& o, const string<_Allocator>& key) NOEXCEPT;
template <typename _Allocator>
inline NODISCARD Meta::TOptional<TPtrRef<value<_Allocator>>> XPath(object<_Allocator>& o, FStringLiteral literal) NOEXCEPT { return XPath(o, string<_Allocator>(literal)); }
template <typename _Allocator>
inline NODISCARD Meta::TOptional<TPtrRef<value<_Allocator>>> XPath(value<_Allocator>& v, std::initializer_list<string<_Allocator>> path) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename _Allocator>
inline NODISCARD Meta::TOptional<TPtrRef<const value<_Allocator>>> XPath(const object<_Allocator>& o, const string<_Allocator>& key) NOEXCEPT;
template <typename _Allocator>
inline NODISCARD Meta::TOptional<TPtrRef<const value<_Allocator>>> XPath(const object<_Allocator>& o, FStringLiteral literal) NOEXCEPT { return XPath(o, string<_Allocator>(literal)); }
template <typename _Allocator>
inline NODISCARD Meta::TOptional<TPtrRef<const value<_Allocator>>> XPath(const value<_Allocator>& v, std::initializer_list<string<_Allocator>> path) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
inline NODISCARD Meta::TOptionalReference<T> XPathAs(object<_Allocator>& o, const string<_Allocator>& key) NOEXCEPT {
    if (Meta::TOptional<TPtrRef<value<_Allocator>>> result = XPath(o, key))
        return std::get_if<T>(result->get());
    return nullptr;
}
template <typename T, typename _Allocator>
inline NODISCARD Meta::TOptionalReference<T> XPathAs(object<_Allocator>& o, FStringLiteral literal) NOEXCEPT {
    if (Meta::TOptional<TPtrRef<value<_Allocator>>> result = XPath(o, literal))
        return std::get_if<T>(result->get());
    return nullptr;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
inline NODISCARD Meta::TOptionalReference<const T> XPathAs(const object<_Allocator>& o, const string<_Allocator>& key) NOEXCEPT {
    if (Meta::TOptional<TPtrRef<const value<_Allocator>>> result = XPath(o, key))
        return std::get_if<T>(result->get());
    return nullptr;
}
template <typename T, typename _Allocator>
inline NODISCARD Meta::TOptionalReference<const T> XPathAs(const object<_Allocator>& o, FStringLiteral literal) NOEXCEPT {
    if (Meta::TOptional<TPtrRef<const value<_Allocator>>> result = XPath(o, literal))
        return std::get_if<T>(result->get());
    return nullptr;
}
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
inline NODISCARD Meta::TOptionalReference<T> XPathAs(value<_Allocator>& v, std::initializer_list<string<_Allocator>> path) NOEXCEPT {
    if (Meta::TOptional<TPtrRef<value<_Allocator>>> result = XPath(v, path))
        return std::get_if<T>(result->get());
    return nullptr;
}
template <typename T, typename _Allocator>
inline NODISCARD Meta::TOptionalReference<const T> XPathAs(const value<_Allocator>& v, std::initializer_list<string<_Allocator>> path) NOEXCEPT {
    if (Meta::TOptional<TPtrRef<const value<_Allocator>>> result = XPath(v, path))
        return std::get_if<T>(result->get());
    return nullptr;
}
//----------------------------------------------------------------------------
// Create a dynamic value<> interactively
//----------------------------------------------------------------------------
class IBuilder {
public:
    virtual ~IBuilder() = default;

    virtual size_t BlockSize() const NOEXCEPT = 0;
    virtual value_block ToValueBlock(FAllocatorBlock block) = 0;

    virtual void Write(nil v) = 0;
    virtual void Write(boolean v) = 0;
    virtual void Write(integer v) = 0;
    virtual void Write(uinteger v) = 0;
    virtual void Write(floating_point v) = 0;

    virtual void Write(string_init v) = 0;
    virtual void Write(wstring_init v) = 0;
    virtual void Write(string_literal v) = 0;
    virtual void Write(wstring_literal v) = 0;
    virtual void Write(string_external v) = 0;
    virtual void Write(wstring_external v) = 0;
    virtual void Write(const string_format& v) = 0;
    virtual void Write(const wstring_format& v) = 0;

    void Write(const value_init& v) { std::visit(*this, v); }
    void Write(const value_view& v) { std::visit(*this, v); }
    template <typename _Al>
    void Write(const value<_Al>& v) { std::visit(*this, v); }

    // explicit trivial type promotion to storage format:
    // handling all variants would drastically rise code complexity,
    // and storage wise the stride of value_init is already majored by the largest type of the variant.
    void Write(i8  i) { Write(static_cast<integer>(i)); }
    void Write(i16 i) { Write(static_cast<integer>(i)); }
    void Write(i32 i) { Write(static_cast<integer>(i)); }

    void Write(u8  u) { Write(static_cast<uinteger>(u)); }
    void Write(u16 u) { Write(static_cast<uinteger>(u)); }
    void Write(u32 u) { Write(static_cast<uinteger>(u)); }

    void Write(float f) { Write(static_cast<floating_point>(f)); }

    virtual void BeginArray(size_t capacity = 0) = 0;
    virtual void EndArray() = 0;

    virtual void BeginObject(size_t capacity = 0) = 0;
    virtual void EndObject() = 0;

    virtual void BeginKeyValue(string_init key) = 0;
    virtual void BeginKeyValue(string_external key) = 0;
    virtual void BeginKeyValue(string_literal key) = 0;
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

    template <typename _KeyLike, typename _Functor, decltype(std::declval<_Functor&>()())* = nullptr>
    void KeyValue(_KeyLike&& keyLike, _Functor&& innerScope) {
        BeginKeyValue(std::forward<_KeyLike>(keyLike));
        innerScope();
        EndKeyValue();
    }

    // need to desambiguate between TBasicStringView/TBasicConstChar/TBasicText
    template <typename _Char, size_t _Len>
    void Write(const _Char (&stringArr)[_Len]) {
        Write(MakeStringLiteral(stringArr));
    }
    template <size_t _Len>
    void BeginKeyValue(const char (&staticKey)[_Len]) {
        BeginKeyValue(string_literal(staticKey));
    }

    // visitor

    void operator ()(nil) {}

    void operator ()(boolean v) { Write(v); }
    void operator ()(integer v) { Write(v); }
    void operator ()(uinteger v) { Write(v); }
    void operator ()(floating_point v) { Write(v); }

    void operator()(i8  i) { Write(i); }
    void operator()(i16 i) { Write(i); }
    void operator()(i32 i) { Write(i); }

    void operator()(u8  u) { Write(u); }
    void operator()(u16 u) { Write(u); }
    void operator()(u32 u) { Write(u); }

    void operator()(float f) { Write(f); }

    void operator ()(string_init v) { Write(v); }
    void operator ()(wstring_init v) { Write(v); }
    void operator ()(const string_view& v) { Write(string_init(v.MakeView())); }
    void operator ()(const wstring_view& v) { Write(wstring_init(v.MakeView())); }
    void operator ()(string_external v) { Write(v); }
    void operator ()(wstring_external v) { Write(v); }
    void operator ()(string_literal v) { Write(v); }
    void operator ()(wstring_literal v) { Write(v); }
    void operator ()(const string_format& v) { Write(v); }
    void operator ()(const wstring_format& v) { Write(v); }

    void operator ()(array_init v) { Array(v.begin(), v.end()); }
    void operator ()(const array_view& v) { Array(v.begin(), v.end()); }
    void operator ()(TPtrRef<const array_view> v) { operator ()(*v); }

    void operator ()(object_init v) { Object(v.begin(), v.end()); }
    void operator ()(const object_view& v) { Object(v.begin(), v.end()); }
    void operator ()(TPtrRef<const object_view> v) { operator ()(*v); }

    void operator ()(const value_init& v) { Write(v); }
    void operator ()(const value_view& v) { Write(v); }

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
    void operator ()(const value<_Al>& v) { Write(v); }
    template <typename _Al>
    void operator ()(const key_value<_Al>& v) {
        BeginKeyValue(string_init(v.key));
        operator ()(v.value);
        EndKeyValue();
    }
};
//----------------------------------------------------------------------------
template <typename _Allocator = default_allocator>
class TBuilder final : public _Allocator, public IBuilder {
public:
    using string_type = string<_Allocator>;
    using wstring_type = wstring<_Allocator>;
    using array_type = array<_Allocator>;
    using object_type = object<_Allocator>;
    using value_type = value<_Allocator>;
    using key_value_type = key_value<_Allocator>;

    using IBuilder::KeyValue;
    using IBuilder::BeginKeyValue;
    using IBuilder::Write;
    using IBuilder::operator();

    using text_memoization_ansi = TBasicTextMemoization<char, ECase::Sensitive, _Allocator>;
    using text_memoization_wide = TBasicTextMemoization<wchar_t, ECase::Sensitive, _Allocator>;

    explicit TBuilder(TPtrRef<value_type> dst) NOEXCEPT { Reset(dst); }
    virtual ~TBuilder() override;

    TBuilder(TPtrRef<value_type> dst, const _Allocator& allocator) : _Allocator(allocator) { Reset(dst); }
    TBuilder(TPtrRef<value_type> dst, _Allocator&& rallocator) NOEXCEPT : _Allocator(std::forward<_Allocator>(rallocator)) { Reset(dst); }

    TBuilder(const TBuilder&) = delete;
    TBuilder& operator =(const TBuilder&) = delete;

    TBuilder(TBuilder&& rvalue) NOEXCEPT
    :   _Allocator(std::move(rvalue.Allocator_()))
    ,   _edits(std::move(rvalue._edits))
    {}
    TBuilder& operator =(TBuilder&&) = delete;

    void Reset(TPtrRef<value_type> dst);

    void SetTextMemoization(TPtrRef<text_memoization_ansi> memoization);
    void SetTextMemoization(TPtrRef<text_memoization_wide> memoization);

    Meta::TOptionalReference<value_type> Peek() NOEXCEPT;
    Meta::TOptionalReference<const value_type> Peek() const NOEXCEPT { return const_cast<TBuilder*>(this)->Peek(); }

    virtual size_t BlockSize() const NOEXCEPT override final;
    virtual value_block ToValueBlock(FAllocatorBlock block) override final;

    virtual void Write(nil v) override final { SetValue_(v); }
    virtual void Write(boolean v) override final { SetValue_(v); }
    virtual void Write(integer v) override final { SetValue_(v); }
    virtual void Write(uinteger v) override final { SetValue_(v); }
    virtual void Write(floating_point v) override final { SetValue_(v); }

    virtual void Write(string_init v) override final { SetValue_(AllocateString_(v)); }
    virtual void Write(wstring_init v) override final { SetValue_(AllocateString_(v)); }
    virtual void Write(string_external v) override final { SetValue_(string_type::MakeForeignText(v)); }
    virtual void Write(wstring_external v) override final { SetValue_(wstring_type::MakeForeignText(v)); }
    virtual void Write(string_literal v) override final { SetValue_(string_type(v)); }
    virtual void Write(wstring_literal v) override final { SetValue_(wstring_type(v)); }
    virtual void Write(const string_format& v) override final;
    virtual void Write(const wstring_format& v) override final;

    void Write(string_type&& v) { SetValue_(std::move(v)); }
    void Write(wstring_type&& v) { SetValue_(std::move(v)); }

    void Write(const string_type& v) { SetValue_(string_type(v)); }
    void Write(const wstring_type& v) { SetValue_(wstring_type(v)); }

    virtual void BeginArray(size_t capacity = 0) override final;
    virtual void EndArray() override final;

    virtual void BeginObject(size_t capacity = 0) override final;
    virtual void EndObject() override final;

    virtual void BeginKeyValue(string_init key) override final { BeginKeyValue(AllocateString_(key)); }
    virtual void BeginKeyValue(string_external key) override final { BeginKeyValue(string_type::MakeForeignText(key)); }
    virtual void BeginKeyValue(string_literal key) override final { BeginKeyValue(string_type(key)); }
    virtual void EndKeyValue() override final;

    void BeginKeyValue(string_type&& rkey);
    void BeginKeyValue(const string_type& key) { BeginKeyValue(string_type(key)); }

    template <typename _KeyLike, typename _Char, size_t _Len,
        decltype(std::declval<IBuilder*>()->BeginKeyValue(std::forward<_KeyLike>(std::declval<_KeyLike&&>())))* = nullptr
    >
    void KeyValue(_KeyLike&& keyLike, const _Char (&staticString)[_Len]) {
        BeginKeyValue(std::forward<_KeyLike>(keyLike));
        Write(string_literal(staticString));
        EndKeyValue();
    }

    template <typename _KeyLike, typename... _Args,
        decltype(std::declval<TBuilder*>()->BeginKeyValue(std::forward<_KeyLike>(std::declval<_KeyLike&&>())))* = nullptr,
        decltype(std::declval<TBuilder*>()->Write(std::forward<_Args>(std::declval<_Args&&>())...))* = nullptr
    >
    void KeyValue(_KeyLike&& keyLike, _Args&&... args) {
        BeginKeyValue(std::forward<_KeyLike>(keyLike));
        Write(std::forward<_Args>(args)...);
        EndKeyValue();
    }

private:
    _Allocator& Allocator_() { return (*this); }

    value_type& Head_() NOEXCEPT;
    value_type& SetValue_(value_type&& rvalue);

    string_type AllocateString_(string_init v);
    wstring_type AllocateString_(wstring_init v);

    TFixedSizeStack<TPtrRef<value_type>, 16> _edits;

    TPtrRef<text_memoization_ansi> _memoization_ansi;
    TPtrRef<text_memoization_wide> _memoization_wide;
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
