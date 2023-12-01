#pragma once

#include "Core_fwd.h"

#if 0

#include "Misc/Opaque.h"

#include "Container/Stack.h"
#include "Container/Vector.h"
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

    virtual value_block ToValueBlock() = 0;

    virtual void Write(boolean v) = 0;
    virtual void Write(integer v) = 0;
    virtual void Write(uinteger v) = 0;
    virtual void Write(floating_point v) = 0;
    virtual void Write(string_init v) = 0;
    virtual void Write(wstring_init v) = 0;

    virtual void BeginArray(size_t capacity = 0) = 0;
    virtual void EndArray() = 0;

    virtual void BeginObject(size_t capacity = 0) = 0;
    virtual void EndObject() = 0;

    virtual void BeginKeyValue(string_init key) = 0;
    virtual void EndKeyValue() = 0;

    // template helpers for closures:

    template <typename _Functor, decltype(std::declval<_Functor&>()())* = nullptr>
    void Array(_Functor&& innerScope, size_t capacity = 0) {
        BeginArray(capacity);
        innerScope();
        EndArray();
    }

    template <typename _Functor, decltype(std::declval<_Functor&>()())* = nullptr>
    void Object(_Functor&& innerScope, size_t capacity = 0) {
        BeginObject(capacity);
        innerScope();
        EndObject();
    }

    template <typename _Functor, decltype(std::declval<_Functor&>()())* = nullptr>
    void KeyValue(string_init key, _Functor&& innerScope) {
        BeginKeyValue(key);
        innerScope();
        EndKeyValue();
    }
};
//----------------------------------------------------------------------------
template <typename _Allocator = default_allocator>
class TBuilder final : private _Allocator, public IBuilder {
public:
    TBuilder();
    ~TBuilder();

    virtual value_block ToValueBlock() override final;''

    virtual void Write(boolean v) override final;
    virtual void Write(integer v) override final;
    virtual void Write(uinteger v) override final;
    virtual void Write(floating_point v) override final;
    virtual void Write(string_init v) override final;
    virtual void Write(wstring_init v) override final;

    virtual void BeginArray(size_t capacity = 0) override final;
    virtual void EndArray() override final;

    virtual void BeginObject(size_t capacity = 0) override final;
    virtual void EndObject() override final;

    virtual void BeginKeyValue(string_init key) override final;
    virtual void EndKeyValue() override final;

private:

};
//----------------------------------------------------------------------------
#if 0
template <typename _Allocator = default_allocator>
class TBuilder final : public IBuilder, _Allocator {
public:
    using allocator_traits = TAllocatorTraits<_Allocator>;

    using string = string<_Allocator>;
    using wstring = wstring<_Allocator>;
    using array = array<_Allocator>;
    using object = object<_Allocator>;
    using value = value<_Allocator>;
    using key_value = key_value<_Allocator>;

    explicit TBuilder(TPtrRef<value> result);
    TBuilder(_Allocator&& rallocator, TPtrRef<value> result) NOEXCEPT;
    TBuilder(const _Allocator& allocator, TPtrRef<value> result);
    ~TBuilder() override;

    value& Head() const { return _edit.back(); }

public: // IBuilder interface
    virtual void Write(boolean v) override final;
    virtual void Write(integer v) override final;
    virtual void Write(uinteger v) override final;
    virtual void Write(floating_point v) override final;
    virtual void Write(string_view v) override final;
    virtual void Write(wstring_view v) override final;

    virtual void BeginArray(size_t capacity = 0) override;
    virtual void EndArray() override final;

    virtual void BeginObject(size_t capacity = 0) override final;
    virtual void EndObject() override final;

    virtual void BeginKeyValue(string_view key) override final;
    virtual void EndKeyValue() override final;

public: // specialized members
    TBuilder& operator <<(boolean v);
    TBuilder& operator <<(integer v);
    TBuilder& operator <<(uinteger v);
    TBuilder& operator <<(floating_point v);
    TBuilder& operator <<(string&& v);
    TBuilder& operator <<(wstring&& v);
    TBuilder& operator <<(array&& v);
    TBuilder& operator <<(object&& v);

    TBuilder operator [](string&& key);

    void BeginKeyValue(string&& key);

    template <typename _Functor, decltype(std::declval<_Functor&>()())* = nullptr>
    void KeyValue(string&& key, _Functor&& functor);

private:
    const _Allocator& Allocator_() const { return (*this); }

    value& Write_(value&& v);

    VECTORINSITU(Opaq, TPtrRef<value>, 16) _edit;
};
//----------------------------------------------------------------------------
template <typename _Allocator = default_allocator>
class TBlockBuilder final : public IBuilder, _Allocator {
public:
    using allocator_traits = TAllocatorTraits<_Allocator>;

    using string = string<_Allocator>;
    using wstring = wstring<_Allocator>;
    using array = array<_Allocator>;
    using object = object<_Allocator>;
    using value = value<_Allocator>;
    using key_value = key_value<_Allocator>;

    explicit TBlockBuilder(TPtrRef<value> result) {
        _edit.push_back(result);
    }
    TBlockBuilder(_Allocator&& rallocator, TPtrRef<value> result) NOEXCEPT : _Allocator(std::move(rallocator)) {
        _edit.push_back(result);
    }
    TBlockBuilder(const _Allocator& allocator, TPtrRef<value> result) : _Allocator(allocator) {
        _edit.push_back(result);
    }
    ~TBlockBuilder() override {
        Assert_NoAssume(_edit.size() == 1);
    }

    value& Head() const { return _edit.back(); }

public: // IBuilder interface
    virtual void Write(boolean v) override final { AppendScalar_(v); }
    virtual void Write(integer v) override final { AppendScalar_(v); }
    virtual void Write(uinteger v) override final { AppendScalar_(v); }
    virtual void Write(floating_point v) override final { AppendScalar_(v); }
    virtual void Write(string_view v) override final { AppendScalar_(string(v)); }
    virtual void Write(wstring_view v) override final { AppendScalar_(wstring(v)); }

    virtual void BeginArray(size_t capacity = 0) override {
        _edit.push_back(AppendScalar_(array{ capacity, Allocator_() }));
    }
    virtual void EndArray() override final {
        Assert_NoAssume(_edit.size() > 1);
        Assert_NoAssume(std::get_if<array>(Head()));
        _edit.pop_back();
    }

    virtual void BeginObject(size_t capacity = 0) override final {
        _edit.push_back(AppendScalar_(object{ capacity, Allocator_() }));
    }
    virtual void EndObject() override final {
        Assert_NoAssume(_edit.size() > 1);
        Assert_NoAssume(std::get_if<object>(Head()));
        _edit.pop_back();
    }

    virtual void BeginKeyValue(string_view key) override final {
        object& obj = std::get<object>(Head());
        obj.emplace_back(std::move(key), value{});
        _edit.push_back(obj.back().value);
    }
    virtual void EndKeyValue() override final {
        Assert_NoAssume(_edit.size() > 1);
        _edit.pop_back();
    }

private:
    const _Allocator& Allocator_() const { return (*this); }

    value& AppendScalar_(value&& v) {
        if (Head().valueless_by_exception())
            return (Head() = std::move(v));

        array& arr = std::get<array>(Head());
        arr.push_back(std::move(v));
        return arr.back();
    }

    VECTORINSITU(Opaq, void**, 8) _addresses;
    VECTORINSITU(Opaq, TPtrRef<value_view>, 8) _edit;
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
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Opaq
} //!namespace PPE

#include "Misc/OpaqueBuilder-inl.h"

#endif
