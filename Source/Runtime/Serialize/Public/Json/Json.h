#pragma once

#include "Serialize.h"

#include "Lexer/Location.h"
#include "SerializeExceptions.h"

#include "Allocator/SlabHeap.h"
#include "Allocator/SlabAllocator.h"
#include "External/imgui/imgui.git/imgui_internal.h"
#include "IO/FileSystem_fwd.h"
#include "IO/StringView.h"
#include "Misc/OpaqueBuilder.h"

// #include <variant>

namespace PPE {
class IBufferedStreamReader;
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FJsonException : public PPE::Serialize::FSerializeException {
public:
    typedef FSerializeException parent_type;

    FJsonException(const char *what)
        : FJsonException(what, Lexer::FLocation()) {}

    FJsonException(const char *what, const Lexer::FLocation& site)
        : parent_type(what), _site(site) {}

    const Lexer::FLocation& Site() const { return _site; }

#if USE_PPE_EXCEPTION_DESCRIPTION
    PPE_SERIALIZE_API virtual FTextWriter& Description(FTextWriter& oss) const override final;
#endif

private:
    Lexer::FLocation _site;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FJson {
public:
    using FSlabHeap = SLABHEAP(Json);
    using allocator_type = SLAB_ALLOCATOR(Json);

    using FValue = Opaq::value<allocator_type>;

    using FNull = Opaq::nil;
    using FBool = Opaq::boolean;
    using FInteger = Opaq::integer;
    using FFloat = Opaq::floating_point;
    using FText = Opaq::string<allocator_type>;
    using FWText = Opaq::wstring<allocator_type>;
    using FArray = Opaq::array<allocator_type>;
    using FObject = Opaq::object<allocator_type>;

    using FBuilder = Opaq::TBuilder<allocator_type>;
    using FTextMemoizationAnsi = FBuilder::text_memoization_ansi;
    using FTextMemoizationWide = FBuilder::text_memoization_wide;

    struct FAllocator {
        FSlabHeap Heap;
        FTextMemoizationAnsi AnsiText{ Heap };
        FTextMemoizationWide WideText{ Heap };

        FAllocator() = default;
        PPE_SERIALIZE_API ~FAllocator();
    };

    using FHeapRef = TPtrRef<FAllocator>;

    template <typename T>
    static CONSTEXPR bool IsKnownType{
        std::is_same_v<T, FNull> ||
        std::is_same_v<T, FBool> ||
        std::is_same_v<T, FInteger> ||
        std::is_same_v<T, FFloat> ||
        std::is_same_v<T, FText> ||
        std::is_same_v<T, FArray> ||
        std::is_same_v<T, FObject>
        };

    static const FText Id;
    static const FText Ref;
    static const FText Class;
    static const FText Export;
    static const FText Inner;
    static const FText TopObject;
    static const FText TypeId;

    static CONSTEXPR FStringLiteral Literal_Id = "$id";
    static CONSTEXPR FStringLiteral Literal_Ref = "$ref";
    static CONSTEXPR FStringLiteral Literal_Class = "_class";
    static CONSTEXPR FStringLiteral Literal_Export = "_export";
    static CONSTEXPR FStringLiteral Literal_Inner = "_inner";
    static CONSTEXPR FStringLiteral Literal_TopObject = "_topObject";
    static CONSTEXPR FStringLiteral Literal_TypeId = "_typeId";

    NODISCARD static bool IsReservedKeyword(const FJson::FText& str) NOEXCEPT;

public:
    explicit FJson(FHeapRef alloc) NOEXCEPT
    :   _alloc(alloc) {
        Assert_NoAssume(_alloc);
    }

    ~FJson();

    FJson(FJson&& ) = default;
    FJson& operator =(FJson&& ) = default;

    NODISCARD FValue& Root() { return _root; }
    NODISCARD const FValue& Root() const { return _root; }

    NODISCARD auto Allocator() const { return TSlabAllocator{ _alloc->Heap }; }

    NODISCARD FSlabHeap& Heap() { return _alloc->Heap; }
    NODISCARD FTextMemoizationAnsi& AnsiText() { return _alloc->AnsiText; }
    NODISCARD FTextMemoizationWide& WideText() { return _alloc->WideText; }

    NODISCARD const FSlabHeap& Heap() const { return _alloc->Heap; }
    NODISCARD const FTextMemoizationAnsi& AnsiText() const { return _alloc->AnsiText; }
    NODISCARD const FTextMemoizationWide& WideText() const { return _alloc->WideText; }

    void Clear_ForgetMemory();
    void Clear_ReleaseMemory();

    void ToStream(FTextWriter& oss, bool minify = false) const;
    void ToStream(FWTextWriter& oss, bool minify = false) const;

    NODISCARD static bool Load(FJson* json, const FFilename& filename);
    NODISCARD static bool Load(FJson* json, const FFilename& filename, IBufferedStreamReader* input);
    NODISCARD static bool Load(FJson* json, const FWStringView& filename, IBufferedStreamReader* input);
    NODISCARD static bool Load(FJson* json, const FWStringView& filename, const FStringView& content);

    NODISCARD static bool Append(FJson* json, const FWStringView& filename, IBufferedStreamReader* input);
    NODISCARD static bool Append(FJson* json, const FWStringView& filename, const FStringView& content);

private:
    FHeapRef _alloc;
    FValue _root;
};
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const FJson& json) {
    json.ToStream(oss);
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline Serialize::FJson::FText operator "" _json (const char* str, size_t len) {
    Unused(len);
    FStringLiteral literal;
    literal.Data = str;
    literal.Length = len;
    return Serialize::FJson::FText::MakeForeignText(literal);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
