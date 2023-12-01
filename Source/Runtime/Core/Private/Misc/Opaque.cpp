// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Misc/Opaque.h"

#include "IO/DummyStreamWriter.h"
#include "IO/StringBuilder.h"
#include "IO/TextWriter.h"
#include "Memory/PtrRef.h"
#include "Misc/Function.h"

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

    void Yield(TFunction<void(TBasicTextWriter<_Char>&)> fmt) {
        fmt(*Output);
    }
    template <typename _Char2>
    void Yield(TFunction<void(TBasicTextWriter<_Char2>&)> fmt) {
        MEMORYSTREAM_STACKLOCAL() tmp;
        TBasicTextWriter<_Char2> oss(&tmp);
        fmt(oss);
        operator ()(TBasicStringView<_Char2>(tmp.MakeView().Cast<const _Char2>()));
    }

    void operator ()(std::monostate) NOEXCEPT { Print("nil"); }

    void operator ()(boolean v) { Print(v ? STRING_LITERAL(_Char, "true") : STRING_LITERAL(_Char, "false")); }
    void operator ()(integer v) { Print(v); }
    void operator ()(uinteger v) { Print(v); }
    void operator ()(floating_point v) { Print(v); }

    void operator ()(string_init v) { Print(STRING_LITERAL(_Char, '"'), v, STRING_LITERAL(_Char, '"')); }
    void operator ()(wstring_init v) { Print(STRING_LITERAL(_Char, '"'), v, STRING_LITERAL(_Char, '"')); }

    void operator ()(const string_view& v) { operator ()(string_init(v)); }
    void operator ()(const wstring_view& v) { operator ()(wstring_init(v)); }

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
};
//----------------------------------------------------------------------------
// Convert a value_init/value<> to a value_view using a single allocation
//----------------------------------------------------------------------------
struct value_block_inliner {

    struct slab {
        FAllocatorBlock Block;
        size_t OffsetInBlock{ 0 };

        explicit slab(FAllocatorBlock block) NOEXCEPT : Block(block) {}

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
            return true;
        }

        virtual class IBufferedStreamWriter* ToBufferedO() NOEXCEPT override final { return nullptr; }
    };

    TPtrRef<value_view> Output;
    TPtrRef<slab> Slab;

    explicit value_block_inliner(value_view& output, slab& slab) NOEXCEPT
    :   Output(output)
    ,   Slab(slab)
    {}

    void operator ()(std::monostate) const { Assert_NoAssume(Output->valueless_by_exception()); }

    void operator ()(boolean v) const { Output->emplace<boolean>(v); }
    void operator ()(integer v) const { Output->emplace<integer>(v); }
    void operator ()(uinteger v) const { Output->emplace<uinteger>(v); }
    void operator ()(floating_point v) const { Output->emplace<floating_point>(v); }

    void operator ()(string_init v) const { Output->emplace<string_view>(Slab->AllocateString(v)); }
    void operator ()(const string_view& v) const { operator ()(string_init(v.MakeView())); }

    void operator ()(wstring_init v) const { Output->emplace<wstring_view>(Slab->AllocateString(v)); }
    void operator ()(const wstring_view& v) const { operator ()(wstring_init(v.MakeView())); }

    void operator ()(const string_format& v) const {
        slab_stream stream(Slab);
        FTextWriter oss(&stream);
        v(oss);
        oss << Eos;
        Output->emplace<string_view>(string_init(stream.Written().Cast<const char>()).ShiftBack(/*'\0'*/));
    }
    void operator ()(const wstring_format& v) const {
        slab_stream stream(Slab);
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
            const_cast<string_view&>(dst->key).reset(Slab->AllocateString(src->key));
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
};
//----------------------------------------------------------------------------
} //!namespace details
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Opaq
} //!namespace PPE
