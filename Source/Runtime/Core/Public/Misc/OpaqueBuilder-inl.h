// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Misc/OpaqueBuilder.h"

#if 0

#include "IO/TextWriter.h"
#include "Memory/PtrRef.h"

namespace PPE {
namespace Opaq {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
// Output opaque data in JSON format
//----------------------------------------------------------------------------
template <typename _Char>
struct value_printer {
    TPtrRef<TBasicTextWriter<_Char>> Output;
    TBasicStringView<_Char> IndentString{ STRING_LITERAL(_Char, "  ") };
    u32 IndentLevel{ 0 };
    bool Compact{ false };
    bool LineFeed{ false };

    explicit value_printer(TBasicTextWriter<_Char>& output) NOEXCEPT
        : Output(output)
        , Compact(output.Format().Misc()& FTextFormat::Compact)
    {}

    void PrintNewLine() {
        if (Compact) {
            *Output << STRING_LITERAL(_Char, ' ');
        }
        else {
            *Output << STRING_LITERAL(_Char, '\n');
            forrange(i, 0, IndentLevel)
                * Output << IndentString;
        }
    }
    template <typename... _Args>
    void Print(_Args&&... args) {
        if (LineFeed) {
            LineFeed = false;
            PrintNewLine();
        }
        (*Output << ... << std::forward<_Args>(args));
    }
    template <typename... _Args>
    void PrintLn(_Args&&... args) {
        Print(std::forward<_Args>(args)...);
        LineFeed = true;
    }
    template <typename... _Args>
    void PrintLn_IFN(_Args&&... args) {
        LineFeed = false;
        PrintLn(std::forward<_Args>(args)...);
    }

    template <typename... _Args>
    void BeginScope(_Args&&... args) {
        PrintLn_IFN(std::forward<_Args>(args)...);
        IndentLevel++;
    }
    template <typename... _Args>
    void EndScope(_Args&&... args) {
        Assert(IndentLevel > 0);
        --IndentLevel;
        if (not LineFeed)
            PrintNewLine();
        PrintLn_IFN(std::forward<_Args>(args)...);
    }

    template <typename T>
    void ScopedItems(const T& items, _Char beginScope, _Char endScope) {
        BeginScope(beginScope);
        bool separator = false;
        for (const auto& it : items) {
            if (separator)
                PrintLn_IFN(STRING_LITERAL(_Char, ','));
            operator ()(it);
            separator = true;
        }
        EndScope(endScope);
    }

    void operator ()(std::monostate) NOEXCEPT { Print("nil"); }

    void operator ()(boolean v) { Print(v); }
    void operator ()(integer v) { Print(v); }
    void operator ()(uinteger v) { Print(v); }
    void operator ()(floating_point v) { Print(v); }

    void operator ()(string_init v) { Print(STRING_LITERAL(_Char, '"'), v, STRING_LITERAL(_Char, '"')); }
    void operator ()(wstring_init v) { Print(STRING_LITERAL(_Char, '"'), v, STRING_LITERAL(_Char, '"')); }

    void operator ()(const string_view& v) { operator ()(string_init(v)); }
    void operator ()(const wstring_view& v) { operator ()(wstring_init(v)); }

    void operator ()(array_init v) { ScopedItems(v, STRING_LITERAL(_Char, '['), STRING_LITERAL(_Char, ']')); }
    void operator ()(object_init v) { ScopedItems(v, STRING_LITERAL(_Char, '{'), STRING_LITERAL(_Char, '}')); }

    inline void operator ()(const value_init& v);
    inline void operator ()(const key_value_init& v);

    //inline void operator ()(string_view v); // already implemented above
    //inline void operator ()(wstring_view v);

    void operator ()(const array_view& v) { ScopedItems(v, STRING_LITERAL(_Char, '['), STRING_LITERAL(_Char, ']')); }
    void operator ()(const object_view& v) { ScopedItems(v, STRING_LITERAL(_Char, '{'), STRING_LITERAL(_Char, '}')); }

    void operator ()(const TPtrRef<const array_view>& v) { operator ()(*v); }
    void operator ()(const TPtrRef<const object_view>& v) { operator ()(*v); }

    inline void operator ()(const value_view& v);
    inline void operator ()(const key_value_view& v);
};
template <typename _Char>
void value_printer<_Char>::operator ()(const value_init& v) {
    std::visit(*this, v);
}
template <typename _Char>
void value_printer<_Char>::operator ()(const value_view& v) {
    std::visit(*this, v);
}
template <typename _Char>
void value_printer<_Char>::operator ()(const key_value_init& v) {
    operator ()(v.key);
    Print(STRING_LITERAL(_Char, '='));
    operator ()(v.value);
}
template <typename _Char>
void value_printer<_Char>::operator ()(const key_value_view& v) {
    operator ()(string_init(v.key));
    Print(STRING_LITERAL(_Char, '='));
    operator ()(v.value);
}
//----------------------------------------------------------------------------
template <typename _Char, typename T>
static TBasicTextWriter<_Char>& WriteOpaq(TBasicTextWriter<_Char>& oss, const T& v) {
    value_printer<_Char>{ oss }(v);
    return oss;
}
//----------------------------------------------------------------------------
// Count memory need to store a deep value_view
//----------------------------------------------------------------------------
struct value_block_capacity {
    FSizeInBytes Size{ 0 };
    FSizeInBytes Wasted{ 0 };

    void Reserve(size_t sz, size_t alignment) NOEXCEPT {
        if (sz == 0) return;
        const size_t unalignedSize = *Size;
        *Size = Meta::RoundToNext(*Size, alignment);
        *Wasted += (*Size - unalignedSize);
        *Size += sz;
    }
    template <typename T>
    void Reserve(size_t count) {
        Reserve(sizeof(T) * count, alignof(T));
    }

    /// already fit in value_view ////////////////
    void operator ()(std::monostate) NOEXCEPT {}
    void operator ()(boolean) NOEXCEPT {}
    void operator ()(integer) NOEXCEPT {}
    void operator ()(uinteger) NOEXCEPT {}
    void operator ()(floating_point) NOEXCEPT {}
    //////////////////////////////////////////////

    void operator ()(string_init v) NOEXCEPT {
        Reserve<string_view::value_type>(v.size() + 1/* '\0' */);
    }
    void operator ()(const string_view& v) NOEXCEPT {
        Reserve<string_view::value_type>(v.size() + 1/* '\0' */);
    }

    void operator ()(wstring_init v) NOEXCEPT {
        Reserve<wstring_view::value_type>(v.size() + 1/* '\0' */);
    }
    void operator ()(const wstring_view& v) NOEXCEPT {
        Reserve<wstring_view::value_type>(v.size() + 1/* '\0' */);
    }

    void operator ()(array_init v) NOEXCEPT {
        Reserve<value_view>(v.size());
        std::for_each(std::begin(v), std::end(v), MakePtrRef(this));
    }
    void operator ()(const array_view& v) NOEXCEPT {
        Reserve<value_view>(v.size());
        std::for_each(std::begin(v), std::end(v), MakePtrRef(this));
    }
    void operator ()(const TPtrRef<const array_view>& v) NOEXCEPT {
        operator ()(*v);
    }

    void operator ()(object_init v) NOEXCEPT {
        Reserve<key_value_view>(v.size());
        std::for_each(std::begin(v), std::end(v), MakePtrRef(this));
    }
    void operator ()(const object_view& v) NOEXCEPT {
        Reserve<key_value_view>(v.size());
        std::for_each(std::begin(v), std::end(v), MakePtrRef(this));
    }
    void operator ()(const TPtrRef<const object_view>& v) NOEXCEPT {
        operator ()(*v);
    }

    void operator ()(const key_value_init& v) NOEXCEPT {
        operator ()(v.key);
        operator ()(v.value);
    }
    void operator ()(const key_value_view& v) NOEXCEPT {
        operator ()(v.key);
        operator ()(v.value);
    }

    void operator ()(const value_init& v) NOEXCEPT {
        std::visit(*this, v);
    }
    void operator ()(const value_view& v) NOEXCEPT {
        std::visit(*this, v);
    }
};
//----------------------------------------------------------------------------
// Convert a value_init/value<> to a value_view using a single allocation
//----------------------------------------------------------------------------
struct value_block_slab {
    FAllocatorBlock Block;
    size_t OffsetInBlock{ 0 };

    explicit value_block_slab(FAllocatorBlock block) NOEXCEPT : Block(block) {}

    FRawMemory Allocate(size_t sz, size_t alignment) {
        if (sz == 0)
            return Default;

        OffsetInBlock = Meta::RoundToNext(OffsetInBlock, alignment);
        Assert(OffsetInBlock + sz <= Block.SizeInBytes);

        FRawMemory result = Block.MakeView().SubRange(OffsetInBlock, sz);
        OffsetInBlock += sz;
        return result;
    }
    template <typename T = value_view>
    TMemoryView<T> AllocateView(size_t n) {
        return Allocate(n * sizeof(T), alignof(T)).template Cast<T>();
    }

    template <typename _Char>
    TBasicStringView<_Char> CopyString(TBasicStringView<_Char> v) {
        if (v.empty())
            return Default;

        // every allocated string will be null-terminated
        TMemoryView<_Char> result = AllocateView<_Char>(v.size() + 1/* '\0' */);
        Unused(Copy(result.ShiftBack(), v));

        result.back() = STRING_LITERAL(_Char, '\0');
        return result.ShiftBack(); // remove final '\0' from the view, but it's here
    }
};
//----------------------------------------------------------------------------
struct value_block_inliner {
    TPtrRef<value_view> Output;
    TPtrRef<value_block_slab> Slab;

    explicit value_block_inliner(value_view& output, value_block_slab& slab) NOEXCEPT
    :   Output(output)
    ,   Slab(slab)
    {}

    void operator ()(std::monostate) const { Assert_NoAssume(Output->valueless_by_exception()); }

    void operator ()(boolean v) const { Output->emplace<boolean>(v); }
    void operator ()(integer v) const { Output->emplace<integer>(v); }
    void operator ()(uinteger v) const { Output->emplace<uinteger>(v); }
    void operator ()(floating_point v) const { Output->emplace<floating_point>(v); }

    void operator ()(string_init v) const { Output->emplace<string_view>(Slab->CopyString(v)); }
    void operator ()(const string_view& v) const { operator ()(string_init(v.MakeView())); }

    void operator ()(wstring_init v) const { Output->emplace<wstring_view>(Slab->CopyString(v)); }
    void operator ()(const wstring_view& v) const { operator ()(wstring_init(v.MakeView())); }

    void operator ()(array_init v) const {
        auto dst = Output->emplace<array_view>(Slab->AllocateView<value_view>(v.size())).begin();
        for (auto src = v.begin(), last = v.end(); src != last; ++src, ++dst)
            std::visit(value_block_inliner{ const_cast<value_view&>(*dst), Slab }, *src);
    }
    void operator ()(const array_view& v) const {
        auto dst = Output->emplace<array_view>(Slab->AllocateView<value_view>(v.size())).begin();
        for (auto src = v.begin(), last = v.end(); src != last; ++src, ++dst)
            std::visit(value_block_inliner{ const_cast<value_view&>(*dst), Slab }, *src);
    }
    void operator ()(const TPtrRef<const array_view>& v) NOEXCEPT const { operator ()(*v); }

    void operator ()(object_init v) const {
        auto dst = Output->emplace<object_view>(Slab->AllocateView<key_value_view>(v.size())).begin();
        for (auto src = v.begin(), last = v.end(); src != last; ++src, ++dst) {
            const_cast<string_view&>(dst->key).reset(Slab->CopyString(src->key));
            std::visit(value_block_inliner{ const_cast<value_view&>(dst->value), Slab }, src->value);
        }
    }
    void operator ()(const object_view& v) const {
        auto dst = Output->emplace<object_view>(Slab->AllocateView<key_value_view>(v.size())).begin();
        for (auto src = v.begin(), last = v.end(); src != last; ++src, ++dst) {
            const_cast<string_view&>(dst->key).reset(Slab->CopyString(string_init(src->key)));
            std::visit(value_block_inliner{ const_cast<value_view&>(dst->value), Slab }, src->value);
        }
    }
    void operator ()(const TPtrRef<const object_view>& v) const NOEXCEPT { operator ()(*v); }
};
////----------------------------------------------------------------------------
//inline value_view value_block_slab::make_view(const value_view& v) {
//    value_block_inliner result{ this };
//    std::visit(result, v);
//    return result.Output;
//}
//----------------------------------------------------------------------------
//template <typename _Al>
//inline value_view value_block_slab::make_view(const value<_Al>& v) {
//    value_block_inliner result{ this };
//    std::visit(result, v);
//    return result.Output;
//}
//----------------------------------------------------------------------------
// Convert a value_init/value_view to a value<> using dynamic allocations
//----------------------------------------------------------------------------
//template <typename _Allocator>
//struct value_allocator {
//    TBuilder<_Allocator> Builder;
//
//    value_allocator(TBuilder<_Allocator>&& builder) NOEXCEPT
//    :   Builder(std::move(builder))
//    {}
//
//    template <typename _It>
//    void Array(_It first, _It last) {
//        Builder.Array([&]() {
//            std::for_each(first, last, *this);
//        }, std::distance(first, last));
//    }
//
//    template <typename _It>
//    void Object(_It first, _It last) {
//        Builder.Object([&]() {
//            forrange(it, first, last) {
//                Builder.KeyValue(string<_Allocator>(it->key), [&]() {
//                    operator ()(it->value);
//                });
//            }
//        }, std::distance(first, last));
//    }
//
//    void operator ()(std::monostate) {}
//
//    void operator ()(boolean v) { Builder << v; }
//    void operator ()(integer v) { Builder << v; }
//    void operator ()(uinteger v) { Builder << v; }
//    void operator ()(floating_point v) { Builder << v; }
//
//    void operator ()(string_init v) { Builder << string<_Allocator>(v); }
//    void operator ()(wstring_init v) { Builder << wstring<_Allocator>(v); }
//
//    void operator ()(string<_Allocator>&& v) { Builder << std::move(v); }
//    void operator ()(wstring<_Allocator>&& v) { Builder << std::move(v); }
//
//    void operator ()(array_init v) { Array(std::begin(v), std::end(v)); }
//    void operator ()(array_view v) { Array(std::begin(v), std::end(v)); }
//    template <typename _Al>
//    void operator ()(const array<_Al>& v) { Array(std::begin(v), std::end(v)); }
//
//    void operator ()(object_init v) { Object(std::begin(v), std::end(v)); }
//    void operator ()(object_view v) { Object(std::begin(v), std::end(v)); }
//    template <typename _Al>
//    void operator ()(const object<_Al>& v) { Object(std::begin(v), std::end(v)); }
//
//    void operator ()(const value_init& v) { std::visit(*this, v); }
//    void operator ()(const value_view& v) { std::visit(*this, v); }
//    template <typename _Al>
//    void operator ()(const value<_Al>& v) { std::visit(*this, v); }
//};
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
//template <typename _Allocator>
//TBuilder<_Allocator>::TBuilder(TPtrRef<value> result) {
//    _edit.push_back(result);
//}
////----------------------------------------------------------------------------
//template <typename _Allocator>
//TBuilder<_Allocator>::TBuilder(_Allocator&& rallocator, TPtrRef<value> result) NOEXCEPT
//:   _Allocator(std::move(rallocator)) {
//    _edit.push_back(result);
//}
////----------------------------------------------------------------------------
//template <typename _Allocator>
//TBuilder<_Allocator>::TBuilder(const _Allocator& allocator, TPtrRef<value> result)
//:   _Allocator(allocator) {
//    _edit.push_back(result);
//}
////----------------------------------------------------------------------------
//template <typename _Allocator>
//TBuilder<_Allocator>::~TBuilder() {
//    Assert_NoAssume(_edit.size() == 1);
//}
////----------------------------------------------------------------------------
//template <typename _Allocator>
//void TBuilder<_Allocator>::Write(boolean v) { Write_(v); }
////----------------------------------------------------------------------------
//template <typename _Allocator>
//void TBuilder<_Allocator>::Write(integer v) { Write_(v); }
////----------------------------------------------------------------------------
//template <typename _Allocator>
//void TBuilder<_Allocator>::Write(uinteger v) { Write_(v); }
////----------------------------------------------------------------------------
//template <typename _Allocator>
//void TBuilder<_Allocator>::Write(floating_point v) { Write_(v); }
////----------------------------------------------------------------------------
//template <typename _Allocator>
//void TBuilder<_Allocator>::Write(string_view v) { Write_(string(v)); }
////----------------------------------------------------------------------------
//template <typename _Allocator>
//void TBuilder<_Allocator>::Write(wstring_view v) { Write_(wstring(v)); }
////----------------------------------------------------------------------------
//template <typename _Allocator>
//void TBuilder<_Allocator>::BeginArray(size_t capacity) {
//    _edit.push_back(Write_(array{ capacity, Allocator_() }));
//}
////----------------------------------------------------------------------------
//template <typename _Allocator>
//void TBuilder<_Allocator>::EndArray() {
//    Assert_NoAssume(_edit.size() > 1);
//    Assert_NoAssume(std::get_if<array>(_edit.Peek()));
//    _edit.pop_back();
//}
////----------------------------------------------------------------------------
//template <typename _Allocator>
//void TBuilder<_Allocator>::BeginObject(size_t capacity) {
//    _edit.push_back(Write_(object{ capacity, Allocator_() }));
//}
////----------------------------------------------------------------------------
//template <typename _Allocator>
//void TBuilder<_Allocator>::EndObject() {
//    Assert_NoAssume(_edit.size() > 1);
//    Assert_NoAssume(std::get_if<object>(_edit.Peek()));
//    _edit.pop_back();
//}
////----------------------------------------------------------------------------
//template <typename _Allocator>
//void TBuilder<_Allocator>::BeginKeyValue(string_view key) {
//    BeginKeyValue(string(key));
//}
////----------------------------------------------------------------------------
//template <typename _Allocator>
//void TBuilder<_Allocator>::BeginKeyValue(string&& key) {
//    object& obj = std::get<object>(Head());
//    obj.emplace_back(std::move(key), value{});
//    _edit.push_back(obj.back().value);
//}
////----------------------------------------------------------------------------
//template <typename _Allocator>
//void TBuilder<_Allocator>::EndKeyValue() {
//    Assert_NoAssume(_edit.size() > 1);
//    _edit.pop_back();
//}
////----------------------------------------------------------------------------
//template <typename _Allocator>
//TBuilder<_Allocator>& TBuilder<_Allocator>::operator<<(boolean v) { Write_(v); return (*this); }
////----------------------------------------------------------------------------
//template <typename _Allocator>
//TBuilder<_Allocator>& TBuilder<_Allocator>::operator<<(integer v) { Write_(v); return (*this); }
////----------------------------------------------------------------------------
//template <typename _Allocator>
//TBuilder<_Allocator>& TBuilder<_Allocator>::operator<<(uinteger v) { Write_(v); return (*this); }
////----------------------------------------------------------------------------
//template <typename _Allocator>
//TBuilder<_Allocator>& TBuilder<_Allocator>::operator<<(floating_point v) { Write_(v); return (*this); }
////----------------------------------------------------------------------------
//template <typename _Allocator>
//TBuilder<_Allocator>& TBuilder<_Allocator>::operator<<(string&& v) { Write_(std::move(v)); return (*this); }
////----------------------------------------------------------------------------
//template <typename _Allocator>
//TBuilder<_Allocator>& TBuilder<_Allocator>::operator<<(wstring&& v) { Write_(std::move(v)); return (*this); }
////----------------------------------------------------------------------------
//template <typename _Allocator>
//TBuilder<_Allocator>& TBuilder<_Allocator>::operator<<(array&& v) { Write_(std::move(v)); return (*this); }
////----------------------------------------------------------------------------
//template <typename _Allocator>
//TBuilder<_Allocator>& TBuilder<_Allocator>::operator<<(object&& v) { Write_(std::move(v)); return (*this); }
////----------------------------------------------------------------------------
//template <typename _Allocator>
//TBuilder<_Allocator> TBuilder<_Allocator>::operator[](string&& key) {
//    object& obj = std::get<object>(Head());
//    obj.emplace_back(std::move(key), value{});
//    return { Allocator_(), obj.back().value };
//}
////----------------------------------------------------------------------------
//template <typename _Allocator>
//template <typename _Functor, decltype(std::declval<_Functor&>()())*>
//void TBuilder<_Allocator>::KeyValue(string&& key, _Functor&& functor) {
//    BeginKeyValue(std::move(key));
//    functor();
//    EndKeyValue();
//}
////----------------------------------------------------------------------------
//template <typename _Allocator>
//typename TBuilder<_Allocator>::value& TBuilder<_Allocator>::Write_(value&& v) {
//    if (Head().valueless_by_exception())
//        return (Head() = std::move(v));
//    else {
//        array& arr = std::get<array>(Head());
//        arr.push_back(std::move(v));
//        return arr.back();
//    }
//}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
//template <typename _Allocator>
//NODISCARD inline size_t BlockSize(const value<_Allocator>& value) NOEXCEPT {
//    details::value_block_capacity capacity;
//    std::visit(capacity, value);
//    return capacity.Size;
//}
////----------------------------------------------------------------------------
//template <typename _Allocator>
//NODISCARD inline value_block NewBlock(FAllocatorBlock block, const value<_Allocator>& value) {
//    details::value_block_slab slab{ block };
//    return { slab.make_view(value), slab.Block };
//}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
//template <typename _Allocator>
//NODISCARD inline value<_Allocator> NewValue(const value_init& init) {
//    value<_Allocator> result;
//    details::value_allocator<_Allocator>{{ result }}(init);
//    return result;
//}
////----------------------------------------------------------------------------
//template <typename _Allocator>
//NODISCARD inline value<_Allocator> NewValue(_Allocator&& rallocator, const value_init& init) {
//    value<_Allocator> result;
//    details::value_allocator<_Allocator>{{ std::move(rallocator), result }}(init);
//    return result;
//}
////----------------------------------------------------------------------------
//template <typename _Allocator>
//NODISCARD inline value<_Allocator> NewValue(const _Allocator& allocator, const value_init& init) {
//    value<_Allocator> result;
//    details::value_allocator<_Allocator>{{ allocator, result }}(init);
//    return result;
//}
////----------------------------------------------------------------------------
//template <typename _Allocator>
//NODISCARD inline value<_Allocator> NewValue(const value_view& view) {
//    value<_Allocator> result;
//    details::value_allocator<_Allocator>{{ result }}(view);
//    return result;
//}
////----------------------------------------------------------------------------
//template <typename _Allocator>
//NODISCARD inline value<_Allocator> NewValue(_Allocator&& rallocator, const value_view& view) {
//    value<_Allocator> result;
//    details::value_allocator<_Allocator>{{ std::move(rallocator), result }}(view);
//    return result;
//}
////----------------------------------------------------------------------------
//template <typename _Allocator>
//NODISCARD inline value<_Allocator> NewValue(const _Allocator& allocator, const value_view& view) {
//    value<_Allocator> result;
//    details::value_allocator<_Allocator>{{ allocator, result }}(view);
//    return result;
//}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
//template <typename _Char, typename _Allocator>
//TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const array<_Allocator>& v) { return details::WriteOpaq(v); }
//template <typename _Char, typename _Allocator>
//TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const object<_Allocator>& v) { return details::WriteOpaq(v); }
//template <typename _Char, typename _Allocator>
//TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const value<_Allocator>& v) { return details::WriteOpaq(v); }
//template <typename _Char, typename _Allocator>
//TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const key_value<_Allocator>& v) { return details::WriteOpaq(v); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Opaq
} //!namespace PPE

#endif
