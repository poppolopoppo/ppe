// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Misc/OpaqueBuilder.h"

#include "Allocator/StackLocalAllocator.h"
#include "IO/DummyStreamWriter.h"
#include "IO/StringBuilder.h"
#include "IO/TextWriter.h"
#include "Memory/MemoryStream.h"
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
    TBasicStringLiteral<_Char> IndentString{ STRING_LITERAL(_Char, "  ") };
    u32 IndentLevel{ 0 };
    bool Compact            : 1;
    bool ControlCharacters  : 1;
    bool LineFeed           : 1;

    explicit value_printer(TBasicTextWriter<_Char>& output) NOEXCEPT
        : Output(output)
        , Compact(output.Format().Misc() & FTextFormat::Compact)
        , ControlCharacters(output.Format().Misc() & FTextFormat::Escape)
        , LineFeed(false) {
        if (ControlCharacters)
            Output->Format().SetMisc(FTextFormat::Escape, false);
    }

    ~value_printer() NOEXCEPT {
        if (ControlCharacters)
            Output->Format().SetMisc(FTextFormat::Escape, true);
    }

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
    void ScopedItems(const T& iterable, _Char beginScope, _Char endScope) {
        BeginScope(beginScope);
        bool separator = false;
        for (const auto& it : iterable) {
            if (separator)
                PrintLn_IFN(STRING_LITERAL(_Char, ','));
            operator ()(it);
            separator = true;
        }
        EndScope(endScope);
    }

    template <size_t _InSitu>
    void Yield(TFunction<void(TBasicTextWriter<_Char>&), _InSitu> fmt) {
        fmt(*Output);
    }
    template <typename _Char2, size_t _InSitu>
    void Yield(TFunction<void(TBasicTextWriter<_Char2>&), _InSitu> fmt) {
        MEMORYSTREAM_STACKLOCAL() tmp;
        TBasicTextWriter<_Char2> oss(&tmp);
        fmt(oss);
        operator ()(TBasicStringView<_Char2>(tmp.MakeView().Cast<const _Char2>()));
    }

    void operator ()(nil) NOEXCEPT { Print("null"); }

    void operator ()(boolean v) { Print(v ? STRING_LITERAL(_Char, "true") : STRING_LITERAL(_Char, "false")); }
    void operator ()(integer v) { Print(v); }
    void operator ()(uinteger v) { Print(v); }
    void operator ()(floating_point v) { Print(v); }

    template <typename _Char2>
    void operator ()(TBasicStringView<_Char2> v) {
        if (LineFeed) {
            LineFeed = false;
            PrintNewLine();
        }
        *Output << STRING_LITERAL(_Char, '"');
        if (ControlCharacters)
            Escape(*Output, v, EEscape::Unicode);
        else
            Print(v);
        *Output << STRING_LITERAL(_Char, '"');
    }

    void operator ()(const string_view& v) { operator ()(v.MakeView()); }
    void operator ()(const wstring_view& v) { operator ()(v.MakeView()); }

    void operator ()(string_external v) { operator ()(v.MakeView()); }
    void operator ()(wstring_external v) { operator ()(v.MakeView()); }

    void operator ()(string_literal v) { operator ()(v.MakeView()); }
    void operator ()(wstring_literal v) { operator ()(v.MakeView()); }

    void operator ()(const string_format& v) { Yield(v); }
    void operator ()(const wstring_format& v) { Yield(v); }

    void operator ()(array_init v) { ScopedItems(v, STRING_LITERAL(_Char, '['), STRING_LITERAL(_Char, ']')); }
    void operator ()(object_init v) { ScopedItems(v, STRING_LITERAL(_Char, '{'), STRING_LITERAL(_Char, '}')); }

    inline void operator ()(const value_init& v);
    inline void operator ()(const key_value_init& v);

    void operator ()(const array_view& v) { ScopedItems(v, STRING_LITERAL(_Char, '['), STRING_LITERAL(_Char, ']')); }
    void operator ()(const object_view& v) { ScopedItems(v, STRING_LITERAL(_Char, '{'), STRING_LITERAL(_Char, '}')); }

    void operator ()(const TPtrRef<const array_view>& v) { operator ()(*v); }
    void operator ()(const TPtrRef<const object_view>& v) { operator ()(*v); }

    inline void operator ()(const value_view& v);
    inline void operator ()(const key_value_view& v);

    template <typename _Al>
    void operator ()(const string<_Al>& v) { operator()(v.MakeView()); }
    template <typename _Al>
    void operator ()(const wstring<_Al>& v) { operator()(v.MakeView()); }
    template <typename _Al>
    void operator ()(const array<_Al>& v) { ScopedItems(v, STRING_LITERAL(_Char, '['), STRING_LITERAL(_Char, ']')); }
    template <typename _Al>
    void operator ()(const object<_Al>& v) { ScopedItems(v, STRING_LITERAL(_Char, '{'), STRING_LITERAL(_Char, '}')); }

    template <typename _Al>
    void operator ()(const value<_Al>& v);
    template <typename _Al>
    void operator ()(const key_value<_Al>& v);
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
template <typename _Al>
void value_printer<_Char>::operator ()(const value<_Al>& v) {
    std::visit(*this, v);
}
template <typename _Char>
void value_printer<_Char>::operator ()(const key_value_init& v) {
    operator ()(v.key);
    Print(Compact ? STRING_LITERAL(_Char, ":") : STRING_LITERAL(_Char, ": "));
    operator ()(v.value);
}
template <typename _Char>
void value_printer<_Char>::operator ()(const key_value_view& v) {
    operator ()(v.key);
    Print(Compact ? STRING_LITERAL(_Char, ":") : STRING_LITERAL(_Char, ": "));
    operator ()(v.value);
}
template <typename _Char>
template <typename _Al>
void value_printer<_Char>::operator ()(const key_value<_Al>& v) {
    operator ()(v.key);
    Print(Compact ? STRING_LITERAL(_Char, ":") : STRING_LITERAL(_Char, ": "));
    operator ()(v.value);
}
//----------------------------------------------------------------------------
template <typename _Char, typename T>
static TBasicTextWriter<_Char>& Print(TBasicTextWriter<_Char>& oss, const T& v) {
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
    void operator ()(nil) NOEXCEPT {}
    void operator ()(boolean) NOEXCEPT {}
    void operator ()(integer) NOEXCEPT {}
    void operator ()(uinteger) NOEXCEPT {}
    void operator ()(floating_point) NOEXCEPT {}
    //////////////////////////////////////////////

    void operator ()(string_init v) NOEXCEPT {
        Reserve<string_view::value_type>(v.size() + 1/* '\0' */);
    }
    void operator ()(const string_view& v) NOEXCEPT {
        Reserve<string_view::value_type>(static_cast<size_t>(v.size()) + 1/* '\0' */);
    }

    void operator ()(wstring_init v) NOEXCEPT {
        Reserve<wstring_view::value_type>(v.size() + 1/* '\0' */);
    }
    void operator ()(const wstring_view& v) NOEXCEPT {
        Reserve<wstring_view::value_type>(static_cast<size_t>(v.size()) + 1/* '\0' */);
    }

#if 0
    void operator ()(string_external ) { /* do not copy externals: their lifetime is already handled */ }
    void operator ()(wstring_external ) { /* do not copy externals: their lifetime is already handled */ }
    void operator ()(string_literal ) { /* do not copy literals: they are static strings */ }
    void operator ()(wstring_literal ) { /* do not copy literals: they are static strings */ }
#else
    void operator ()(string_external v) { operator ()(v.MakeView()); }
    void operator ()(wstring_external v) { operator ()(v.MakeView()); }
    void operator ()(string_literal v) { operator ()(v.MakeView()); }
    void operator ()(wstring_literal v) { operator ()(v.MakeView()); }
#endif

    void operator ()(const string_format& v) NOEXCEPT {
        FDummyStreamWriter dummy; // count memory without actually writing
        FTextWriter oss(&dummy);
        v(oss);
        oss << Eos; // '\0'
        Reserve(dummy.SizeInBytes(), alignof(string_init::value_type));
    }
    void operator ()(const wstring_format& v) NOEXCEPT {
        FDummyStreamWriter dummy; // count memory without actually writing
        FWTextWriter oss(&dummy);
        v(oss);
        oss << Eos; // '\0'
        Reserve(dummy.SizeInBytes(), alignof(wstring_init::value_type));
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

    template <typename _Al>
    void operator ()(const string<_Al>& v) { operator()(v.MakeView()); }
    template <typename _Al>
    void operator ()(const wstring<_Al>& v) { operator()(v.MakeView()); }

    template <typename _Al>
    void operator ()(const array<_Al>& v) {
        Reserve<value_view>(v.size());
        std::for_each(std::begin(v), std::end(v), MakePtrRef(this));
    }

    template <typename _Al>
    void operator ()(const object<_Al>& v) {
        Reserve<key_value_view>(v.size());
        std::for_each(std::begin(v), std::end(v), MakePtrRef(this));
    }

    template <typename _Al>
    void operator ()(const value<_Al>& v) {
        std::visit(*this, v);
    }
    template <typename _Al>
    void operator ()(const key_value<_Al>& v) {
        operator ()(v.key);
        operator ()(v.value);
    }
};
//----------------------------------------------------------------------------
// Convert a value_init/value<> to a value_view using a single allocation
//----------------------------------------------------------------------------
struct value_block_inliner {

    struct slab {
        FAllocatorBlock Block;
        size_t OffsetInBlock{ 0 };

        explicit slab(FAllocatorBlock alloc) NOEXCEPT : Block(alloc) {}

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
        TBasicStringView<_Char> AllocateString(TBasicStringView<_Char> v) {
            if (v.empty())
                return Default;

            // every allocated string will be null-terminated
            TMemoryView<_Char> result = AllocateView<_Char>(v.size() + 1/* '\0' */);
            Unused(Copy(result.ShiftBack(), v));

            result.back() = STRING_LITERAL(_Char, '\0');
            return result.ShiftBack(); // remove final '\0' from the view, but it's here
        }
    };

    struct slab_stream final : public IStreamWriter {
        TPtrRef<slab> Slab;
        const size_t StartOffsetInBlock{ 0 };

        explicit slab_stream(slab& slab) NOEXCEPT
        :   Slab(slab)
        ,   StartOffsetInBlock(slab.OffsetInBlock)
        {}

        FRawMemoryConst Written() const {
            return Slab->Block.MakeView().SubRangeConst(StartOffsetInBlock, Slab->OffsetInBlock - StartOffsetInBlock);
        }

        virtual bool IsSeekableO(ESeekOrigin origin = ESeekOrigin::All) const NOEXCEPT override final {
            Unused(origin);
            return false;
        }
        virtual std::streamoff TellO() const NOEXCEPT override final {
            return (Slab->OffsetInBlock - StartOffsetInBlock);
        }
        virtual std::streamoff SeekO(std::streamoff offset, ESeekOrigin origin = ESeekOrigin::Begin) override final {
            Unused(offset, origin);
            AssertNotImplemented();
        }
        virtual bool Write(const void* storage, std::streamsize sizeInBytes) override final {
            const FRawMemory dst = Slab->Allocate(sizeInBytes, 1);
            Assert(dst.SizeInBytes() >= checked_cast<size_t>(sizeInBytes));
            FPlatformMemory::Memcpy(dst.data(), storage, sizeInBytes);
            return true;
        }
        virtual size_t WriteSome(const void* storage, size_t eltsize, size_t count) override final {
            const size_t sizeInBytes = (eltsize * count);
            const FRawMemory dst = Slab->Allocate(sizeInBytes, eltsize);
            Assert(dst.SizeInBytes() >= sizeInBytes);
            FPlatformMemory::Memcpy(dst.data(), storage, sizeInBytes);
            return sizeInBytes;
        }

        virtual class IBufferedStreamWriter* ToBufferedO() NOEXCEPT override final { return nullptr; }
    };

    TPtrRef<value_view> Output;
    TPtrRef<slab> Slab;

    explicit value_block_inliner(value_view& output, slab& slab) NOEXCEPT
    :   Output(output)
    ,   Slab(slab)
    {}

    void operator ()(nil) const { Assert_NoAssume(Output->valueless_by_exception()); }

    void operator ()(boolean v) const { Output->emplace<boolean>(v); }
    void operator ()(integer v) const { Output->emplace<integer>(v); }
    void operator ()(uinteger v) const { Output->emplace<uinteger>(v); }
    void operator ()(floating_point v) const { Output->emplace<floating_point>(v); }

    void operator ()(string_init v) const { Output->emplace<string_view>(Slab->AllocateString(std::move(v))); }
    void operator ()(const string_view& v) const { operator ()(string_init(v.MakeView())); }

    void operator ()(wstring_init v) const { Output->emplace<wstring_view>(Slab->AllocateString(std::move(v))); }
    void operator ()(const wstring_view& v) const { operator ()(wstring_init(v.MakeView())); }

    void operator ()(string_external v) const {
#if 0
        if (AllowExternalStrings)
            Output->emplace<string_external>(v);
        else
#endif
            operator ()(v.MakeView());
    }
    void operator ()(string_literal v) const {
#if 0
        if (AllowExternalStrings)
            operator ()(string_external(v));
        else
#endif
            operator ()(v.MakeView());
    }

    void operator ()(wstring_external v) const {
#if 0
        if (AllowExternalStrings)
            Output->emplace<wstring_external>(v);
        else
#endif
            operator ()(v.MakeView());
    }
    void operator ()(wstring_literal v) const {
#if 0
        if (AllowExternalStrings)
            operator ()(wstring_external(v));
        else
#endif
            operator ()(v.MakeView());
    }

    void operator ()(const string_format& v) const {
        slab_stream stream(Slab); // format the string inplace, no temporary buffer
        FTextWriter oss(&stream);
        v(oss);
        oss << Eos;
        Output->emplace<string_view>(string_init(stream.Written().Cast<const char>()).ShiftBack(/*'\0'*/));
    }
    void operator ()(const wstring_format& v) const {
        slab_stream stream(Slab); // format the string inplace, no temporary buffer
        FWTextWriter oss(&stream);
        v(oss);
        oss << Eos;
        Output->emplace<wstring_view>(wstring_init(stream.Written().Cast<const wchar_t>()).ShiftBack(/*'\0'*/));
    }

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
    void operator ()(const TPtrRef<const array_view>& v) const NOEXCEPT { operator ()(*v); }

    void operator ()(object_init v) const {
        auto dst = Output->emplace<object_view>(Slab->AllocateView<key_value_view>(v.size())).begin();
        for (auto src = v.begin(), last = v.end(); src != last; ++src, ++dst) {
            const_cast<string_view&>(dst->key).reset(Slab->AllocateString(src->key.MakeView()));
            std::visit(value_block_inliner{ const_cast<value_view&>(dst->value), Slab }, src->value);
        }
    }
    void operator ()(const object_view& v) const {
        auto dst = Output->emplace<object_view>(Slab->AllocateView<key_value_view>(v.size())).begin();
        for (auto src = v.begin(), last = v.end(); src != last; ++src, ++dst) {
            const_cast<string_view&>(dst->key).reset(Slab->AllocateString(string_init(src->key)));
            std::visit(value_block_inliner{ const_cast<value_view&>(dst->value), Slab }, src->value);
        }
    }
    void operator ()(const TPtrRef<const object_view>& v) const NOEXCEPT { operator ()(*v); }

    template <typename _Al>
    void operator ()(const string<_Al>& v) {
        if (v.IsLiteral())
            operator()(string_external{v.ConstChar()});
        else
            operator()(v.MakeView());
    }
    template <typename _Al>
    void operator ()(const wstring<_Al>& v) {
        if (v.IsLiteral())
            operator()(wstring_external{v.ConstChar()});
        else
            operator()(v.MakeView());
    }

    template <typename _Al>
    void operator ()(const array<_Al>& v) {
        auto dst = Output->emplace<array_view>(Slab->AllocateView<value_view>(v.size())).begin();
        for (auto src = v.begin(), last = v.end(); src != last; ++src, ++dst)
            std::visit(value_block_inliner{ const_cast<value_view&>(*dst), Slab }, *src);
    }

    template <typename _Al>
    void operator ()(const object<_Al>& v) {
        auto dst = Output->emplace<object_view>(Slab->AllocateView<key_value_view>(v.size())).begin();
        for (auto src = v.begin(), last = v.end(); src != last; ++src, ++dst) {
            const_cast<string_view&>(dst->key).reset(Slab->AllocateString(src->key.MakeView()));
            std::visit(value_block_inliner{ const_cast<value_view&>(dst->value), Slab }, src->value);
        }
    }
};
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Allocator>
size_t BlockSize(const value<_Allocator>& value) NOEXCEPT {
    details::value_block_capacity capacity;
    capacity.Reserve<value_view>(1); // reserve space for root value_view, see NewBlock()
    std::visit(capacity, value);
    return capacity.Size;
}
//----------------------------------------------------------------------------
template <typename _Allocator>
value_block NewBlock(FAllocatorBlock alloc, const value<_Allocator>& value) {
    details::value_block_inliner::slab slab{ alloc };
    // first value_view is embedded in block: TRelativeView<> can't be copied/moved!
    value_view& root = *slab.AllocateView<value_view>(1).data();
    std::visit(details::value_block_inliner{ root, slab }, value);
    return { alloc };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Allocator>
value<_Allocator> NewValue(const value_init& init) {
    TBuilder<_Allocator> builder;
    builder(init);
    return builder.ToValue();
}
//----------------------------------------------------------------------------
template <typename _Allocator>
value<_Allocator> NewValue(_Allocator&& rallocator, const value_init& init) {
    TBuilder<_Allocator> builder(std::forward<_Allocator>(rallocator));
    builder(init);
    return builder.ToValue();
}
//----------------------------------------------------------------------------
template <typename _Allocator>
value<_Allocator> NewValue(const _Allocator& allocator, const value_init& init) {
    TBuilder<_Allocator> builder(allocator);
    builder(init);
    return builder.ToValue();
}
//----------------------------------------------------------------------------
template <typename _Allocator>
value<_Allocator> NewValue(const value_view& view) {
    TBuilder<_Allocator> builder;
    builder(view);
    return builder.ToValue();
}
//----------------------------------------------------------------------------
template <typename _Allocator>
value<_Allocator> NewValue(_Allocator&& rallocator, const value_view& view) {
    TBuilder<_Allocator> builder(std::forward<_Allocator>(rallocator));
    builder(view);
    return builder.ToValue();
}
//----------------------------------------------------------------------------
template <typename _Allocator>
value<_Allocator> NewValue(const _Allocator& allocator, const value_view& view) {
    TBuilder<_Allocator> builder(allocator);
    builder(view);
    return builder.ToValue();
}
//----------------------------------------------------------------------------
template <typename _Allocator>
Meta::TOptional<TPtrRef<value<_Allocator>>> XPath(object<_Allocator>& o, const string<_Allocator>& key) NOEXCEPT {
    for (key_value<_Allocator>& it : o) {
        if (it.key == key) {
            return it.value;
        }
    }
    return Meta::TOptional<TPtrRef<value<_Allocator>>>{};
}
//----------------------------------------------------------------------------
template <typename _Allocator>
Meta::TOptional<TPtrRef<const value<_Allocator>>> XPath(const object<_Allocator>& o, const string<_Allocator>& key) NOEXCEPT {
    for (const key_value<_Allocator>& it : o) {
        if (it.key == key) {
            return it.value;
        }
    }
    return Meta::TOptional<TPtrRef<const value<_Allocator>>>{};
}
//----------------------------------------------------------------------------
template <typename _Allocator>
Meta::TOptional<TPtrRef<value<_Allocator>>> XPath(value<_Allocator>& v, std::initializer_list<string<_Allocator>> path) NOEXCEPT {
    TPtrRef node{ &v };

    for (const string<_Allocator>& key : path) {
        const Meta::TOptionalReference<object<_Allocator>> obj{ std::get_if<object<_Allocator>>(node.get()) };
        if (not obj)
            break;

        node.reset();
        for (key_value<_Allocator>& it : *obj) {
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

    return Meta::TOptional<TPtrRef<value<_Allocator>>>{};
}
//----------------------------------------------------------------------------
template <typename _Allocator>
Meta::TOptional<TPtrRef<const value<_Allocator>>> XPath(const value<_Allocator>& v, std::initializer_list<string<_Allocator>> path) NOEXCEPT {
    TPtrRef node{ &v };

    for (const string<_Allocator>& key : path) {
        const Meta::TOptionalReference<const object<_Allocator>> obj{ std::get_if<object<_Allocator>>(node.get()) };
        if (not obj)
            break;

        node.reset();
        for (const key_value<_Allocator>& it : *obj) {
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

    return Meta::TOptional<TPtrRef<const value<_Allocator>>>{};
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Allocator>
TBuilder<_Allocator>::~TBuilder() {
    Assert_NoAssume(_edits.size() == 1);
}
//----------------------------------------------------------------------------a
template <typename _Allocator>
void TBuilder<_Allocator>::Reset(TPtrRef<value_type> dst) {
    Assert(dst);
    if (_edits.empty()) {
        _edits.Push(dst);
        return;
    }

    AssertMessage_NoAssume("builder is still beeing used", _edits.size() == 1);

    _edits[0] = dst;
}
//----------------------------------------------------------------------------a
template <typename _Allocator>
void TBuilder<_Allocator>::SetTextMemoization(TPtrRef<text_memoization_ansi> memoization) {
    // optional text memoization can be used to merge identical strings
    _memoization_ansi = std::move(memoization);
}
//----------------------------------------------------------------------------a
template <typename _Allocator>
void TBuilder<_Allocator>::SetTextMemoization(TPtrRef<text_memoization_wide> memoization) {
    // optional text memoization can be used to merge identical strings
    _memoization_wide = std::move(memoization);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
size_t TBuilder<_Allocator>::BlockSize() const NOEXCEPT {
    return Opaq::BlockSize(*_edits[0]);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
value_block TBuilder<_Allocator>::ToValueBlock(FAllocatorBlock alloc) {
    Assert_NoAssume(_edits.size() == 1);

    return NewBlock(alloc, *_edits[0]);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TBuilder<_Allocator>::Write(const string_format& v) {
    MEMORYSTREAM_STACKLOCAL() tmp; // use stack local stream to avoid writing TBasicTextBuilder<>
    FTextWriter writer(&tmp);
    v(writer);
    Write(string_init(tmp.MakeView().Cast<const char>()));
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TBuilder<_Allocator>::Write(const wstring_format& v) {
    MEMORYSTREAM_STACKLOCAL() tmp; // use stack local stream to avoid writing TBasicTextBuilder<>
    FWTextWriter writer(&tmp);
    v(writer);
    Write(wstring_init(tmp.MakeView().Cast<const wchar_t>()));
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TBuilder<_Allocator>::BeginArray(size_t capacity) {
    array_type arr{ Allocator_() };
    arr.reserve(capacity);
    _edits.Push(SetValue_(std::move(arr)));
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TBuilder<_Allocator>::EndArray() {
    Assert_NoAssume(not _edits.empty());
    Assert_NoAssume(std::holds_alternative<array_type>(*_edits.Peek()));

    _edits.Pop();
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TBuilder<_Allocator>::BeginObject(size_t capacity) {
    object_type obj{ Allocator_() };
    obj.reserve(capacity);

    _edits.Push(SetValue_(std::move(obj)));
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TBuilder<_Allocator>::EndObject() {
    Assert_NoAssume(not _edits.empty());
    Assert_NoAssume(std::holds_alternative<object_type>(*_edits.Peek()));

    _edits.Pop();
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TBuilder<_Allocator>::BeginKeyValue(string_type&& rkey) {
    object_type& obj = std::get<object_type>(Head_());

    _edits.Push(Emplace_Back(obj, std::move(rkey), value_type{})->value);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
void TBuilder<_Allocator>::EndKeyValue() {
    Assert_NoAssume(not _edits.empty());

    _edits.Pop();
}
//----------------------------------------------------------------------------
template <typename _Allocator>
auto TBuilder<_Allocator>::Head_() NOEXCEPT -> value_type& {
    return *_edits.Peek();
}
//----------------------------------------------------------------------------
template <typename _Allocator>
auto TBuilder<_Allocator>::Peek() NOEXCEPT -> Meta::TOptionalReference<value_type> {
    if (value_type& head = Head_(); Likely(head)) {
        if (array_type* const pArr = std::get_if<array_type>(&head)) {
            if (not pArr->empty())
                return Meta::MakeOptionalRef(pArr->back());
        } else {
            return Meta::MakeOptionalRef(head);
        }
    }
    return Meta::TOptionalReference<value_type>{};
}
//----------------------------------------------------------------------------
template <typename _Allocator>
auto TBuilder<_Allocator>::SetValue_(value_type&& rvalue) -> value_type& {
    if (value_type& head = Head_(); Likely(not head))
        return (head = std::move(rvalue));
    else
        return (*Emplace_Back(std::get<array_type>(head), std::move(rvalue)));
}
//----------------------------------------------------------------------------
template <typename _Allocator>
auto TBuilder<_Allocator>::AllocateString_(string_init v) -> string_type {
    if (_memoization_ansi)
        return _memoization_ansi->Append(v);

    return string_type(Allocator_(), v);
}
//----------------------------------------------------------------------------
template <typename _Allocator>
auto TBuilder<_Allocator>::AllocateString_(wstring_init v) -> wstring_type {
    if (_memoization_wide)
        return _memoization_wide->Append(v);

    return wstring_type(Allocator_(), v);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const array<_Allocator>& v) { return details::Print(oss, v); }
template <typename _Char, typename _Allocator>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const object<_Allocator>& v) { return details::Print(oss, v); }
template <typename _Char, typename _Allocator>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const value<_Allocator>& v) { return details::Print(oss, v); }
template <typename _Char, typename _Allocator>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const key_value<_Allocator>& v) { return details::Print(oss, v); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Opaq
} //!namespace PPE
