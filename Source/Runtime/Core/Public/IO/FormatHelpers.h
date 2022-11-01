#pragma once

#include "Core.h"

#include "Container/Appendable.h"
#include "Container/TupleTie.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"
#include "Memory/MemoryView.h"
#include "Memory/PtrRef.h"
#include "Maths/Units.h"
#include "Misc/Function.h"
#include "Meta/StronglyTyped.h"
#include "Meta/TypeInfo.h"

#include <initializer_list>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
struct FOffset {
    u64 Value;
};
template <typename T>
Meta::TEnableIf<std::is_integral_v<T>, FOffset> Offset(T i) NOEXCEPT {
    return FOffset{ static_cast<u64>(i) };
}
} //!namespace Fmt
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, Fmt::FOffset off);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, Fmt::FOffset off);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
struct FPointer {
    void* Value;
};
template <typename T>
FPointer Pointer(const T* p) NOEXCEPT { return FPointer{ (void*)p }; }
} //!namespace Fmt
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, Fmt::FPointer ptr);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, Fmt::FPointer ptr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
PPE_STRONGLYTYPED_NUMERIC_DEF(float, FPercentage);
template <typename T, typename = Meta::TEnableIf<std::is_arithmetic_v<T>> >
FPercentage Percentage(T x, T total) NOEXCEPT { return FPercentage{ x * 100.f / total }; }
} //!namespace Fmt
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, Fmt::FPercentage prc);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, Fmt::FPercentage prc);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
PPE_STRONGLYTYPED_NUMERIC_DEF(u64, FCountOfElements);
template <typename T>
Meta::TEnableIf<std::is_integral_v<T>, FCountOfElements> CountOfElements(T n) NOEXCEPT {
    return FCountOfElements{ checked_cast<u64>(n) };
}
} //!namespace Fmt
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, Fmt::FCountOfElements count);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, Fmt::FCountOfElements count);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
PPE_STRONGLYTYPED_NUMERIC_DEF(uint64_t, FSizeInBytes);
template <typename T>
Meta::TEnableIf<std::is_integral_v<T>, FSizeInBytes> SizeInBytes(T n) NOEXCEPT {
    return FSizeInBytes{ checked_cast<uint64_t>(n) };
}
} //!namespace Fmt
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, Fmt::FSizeInBytes size);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, Fmt::FSizeInBytes size);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
struct FDurationInMs { double Value; };
inline FDurationInMs DurationInMs(FMicroseconds t) NOEXCEPT {
    return FDurationInMs{ t.Value() };
}
} //!namespace Fmt
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, Fmt::FDurationInMs v);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, Fmt::FDurationInMs v);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
template <typename T, typename _Quote>
struct TQuoted {
    T Value;
    _Quote Quote;
};
template <typename T, typename _Quote>
auto Quoted(const T& value, _Quote quote) NOEXCEPT {
    return TQuoted<TPtrRef<const Meta::TDecay<T>>, _Quote>{ value, quote };
}
template <typename T, typename _Quote>
auto Quoted(T&& rvalue, _Quote quote) NOEXCEPT {
    return TQuoted<Meta::TDecay<T>, _Quote>{ std::move(rvalue), quote };
}
} //!namespace Fmt
//----------------------------------------------------------------------------
template <typename _Char, typename T, typename _Quote>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const Fmt::TQuoted<T, _Quote>& quoted) {
    return oss << quoted.Quote << quoted.Value << quoted.Quote;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
template <typename T, typename _Char>
struct TPadded {
    T Outp;
    FTextFormat::EPadding Align;
    size_t Width;
    _Char FillChar;
};
template <typename T, typename _Char>
auto PadLeft(const T& value, size_t width = INDEX_NONE, _Char ch = _Char(' ')) NOEXCEPT {
    return TPadded<TPtrRef<const Meta::TDecay<T>>, _Char>{ value, FTextFormat::Padding_Left, width, ch };
}
template <typename T, typename _Char>
auto PadCenter(const T& value, size_t width = INDEX_NONE, _Char ch = _Char(' ')) NOEXCEPT {
    return TPadded<TPtrRef<const Meta::TDecay<T>>, _Char>{ value, FTextFormat::Padding_Center, width, ch };
}
template <typename T, typename _Char>
auto PadRight(const T& value, size_t width = INDEX_NONE, _Char ch = _Char(' ')) NOEXCEPT {
    return TPadded<TPtrRef<const Meta::TDecay<T>>, _Char>{ value, FTextFormat::Padding_Right, width, ch };
}
template <typename _Char, size_t _Dim>
auto PadLeft(const _Char (&value)[_Dim], size_t width = INDEX_NONE, _Char ch = _Char(' ')) NOEXCEPT {
    return TPadded<TBasicStringView<_Char>, _Char>{ MakeStringView(value), FTextFormat::Padding_Left, width, ch };
}
template <typename _Char, size_t _Dim>
auto PadCenter(const _Char (&value)[_Dim], size_t width = INDEX_NONE, _Char ch = _Char(' ')) NOEXCEPT {
    return TPadded<TBasicStringView<_Char>, _Char>{ MakeStringView(value), FTextFormat::Padding_Center, width, ch };
}
template <typename _Char, size_t _Dim>
auto PadRight(const _Char (&value)[_Dim], size_t width = INDEX_NONE, _Char ch = _Char(' ')) NOEXCEPT {
    return TPadded<TBasicStringView<_Char>, _Char>{ MakeStringView(value), FTextFormat::Padding_Right, width, ch };
}
template <typename _Char>
auto AlignLeft(const TBasicStringView<_Char>& value, size_t width = INDEX_NONE, _Char ch = _Char(' ')) NOEXCEPT {
    return TPadded<TPtrRef<const TBasicStringView<_Char>>, _Char>{ &value, FTextFormat::Padding_Right, width, ch };
}
template <typename _Char>
auto AlignCenter(const TBasicStringView<_Char>& value, size_t width = INDEX_NONE, _Char ch = _Char(' ')) NOEXCEPT {
    return TPadded<TPtrRef<const TBasicStringView<_Char>>, _Char>{ value, FTextFormat::Padding_Center, width, ch };
}
template <typename _Char>
auto AlignRight(const TBasicStringView<_Char>& value, size_t width = INDEX_NONE, _Char ch = _Char(' ')) NOEXCEPT {
    return TPadded<TPtrRef<const TBasicStringView<_Char>>, _Char>{ value, FTextFormat::Padding_Left, width, ch };
}
} //!namespace Fmt
//----------------------------------------------------------------------------
template <typename _Char, typename T>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const Fmt::TPadded<T, _Char>& padded) {
    return oss << FTextFormat::Pad(padded.Align, padded.Width, padded.FillChar) << padded.Outp;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
template <typename T>
struct TNotFirstTime {
    T Outp;
    bool First = true;
};
template <typename T>
TNotFirstTime<T> NotFirstTime(T&& outp) NOEXCEPT {
    return TNotFirstTime<T>{ std::move(outp) };
}
} //!namespace Fmt
//----------------------------------------------------------------------------
template <typename _Char, typename T>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, Fmt::TNotFirstTime<T>& notFirstTime ) {
    if (Unlikely(notFirstTime.First))
        notFirstTime.First = false;
    else
        oss << notFirstTime.Outp;
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
template <typename T>
struct TNotLastTime {
    T Outp;
    size_t Count;
    size_t Index = 0;
};
template <typename T>
TNotLastTime<T> NotLastTime(T&& outp, size_t count) NOEXCEPT {
    return TNotLastTime<T>{ std::move(outp), count };
}
} //!namespace Fmt
//----------------------------------------------------------------------------
template <typename _Char, typename T>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, Fmt::TNotLastTime<T>& notLastTime ) {
    if (Likely(++notLastTime.Index < notLastTime.Count))
        oss << notLastTime.Outp;
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
template <typename T>
struct TRepeater {
    T Value;
    size_t Count;
};
template <typename T>
TRepeater<const T&> Repeat(const T& value, size_t count) NOEXCEPT {
    return TRepeater<const T&>{ &value, count };
}
template <typename T>
TRepeater<T> Repeat(T&& rvalue, size_t count) NOEXCEPT {
    return TRepeater<T>{ std::move(rvalue), count };
}
} //!namespace Fmt
//----------------------------------------------------------------------------
template <typename _Char, typename T>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const Fmt::TRepeater<T>& repeat ) {
    forrange(i, 0, repeat.Count)
        oss << repeat.Value;
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
template <typename T>
struct TConditional {
    const T* Value = nullptr;
    bool Enabled = false;
};
template <typename T>
TConditional<T> Conditional(const T& value, bool enabled) NOEXCEPT {
    return TConditional<T>{ &value, enabled };
}
} //!namespace Fmt
//----------------------------------------------------------------------------
template <typename _Char, typename T>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const Fmt::TConditional<T>& cond) {
    Assert(cond.Value);
    if (cond.Enabled)
        oss << *cond.Value;
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
template <typename _True, typename _False>
struct TTernary {
    bool Condition = false;
    const _True* IfTrue = nullptr;
    const _False* IfFalse = nullptr;
};
template <typename _True, typename _False>
TTernary<_True, _False> Ternary(bool condition, const _True& ifTrue, const _False& ifFalse) NOEXCEPT {
    return TTernary<_True, _False>{ condition, &ifTrue, &ifFalse };
}
} //!namespace Fmt
//----------------------------------------------------------------------------
template <typename _Char, typename _True, typename _False>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const Fmt::TTernary<_True, _False>& ternary ) {
    Assert(ternary.IfTrue);
    Assert(ternary.IfFalse);
    return ((ternary.Condition)
        ? oss << (*ternary.IfTrue)
        : oss << (*ternary.IfFalse) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
template <typename _It, typename _Sep>
struct TJoin {
    _It First;
    _It Last;
    _Sep Separator;
    TJoin(_It first, _It last, _Sep separator) NOEXCEPT
    :   First(first), Last(last), Separator(separator) {}
};
template <typename T, typename _Sep>
auto Join(std::initializer_list<T> list, _Sep separator) NOEXCEPT {
    using iterator = typename std::initializer_list<T>::iterator;
    return TJoin<iterator, _Sep>(list.begin(), list.end(), separator);
}
template <typename _It, typename _Sep>
auto Join(_It first, _It last, _Sep separator) NOEXCEPT {
    return TJoin<_It, _Sep>(first, last, separator);
}
template <typename _It, typename _Sep>
auto Join(const TIterable<_It>& iterable, _Sep separator) NOEXCEPT {
    return TJoin<_It, _Sep>(iterable.begin(), iterable.end(), separator);
}
template <typename T, typename _Sep>
auto Join(const TMemoryView<T>& view, _Sep separator) NOEXCEPT {
    return TJoin<typename TMemoryView<T>::iterator, _Sep>(view.begin(), view.end(), separator);
}
template <typename T>
auto CommaSeparated(std::initializer_list<T> list) NOEXCEPT {
    return Join(list.begin(), list.end(), MakeStringView(", "));
}
template <typename T>
auto CommaSeparated(const TMemoryView<T>& data) NOEXCEPT {
    return Join(data.begin(), data.end(), MakeStringView(", "));
};
template <typename T>
auto CommaSeparatedW(const TMemoryView<T>& data) NOEXCEPT {
    return Join(data.begin(), data.end(), MakeStringView(L", "));
};
} //!namespace Fmt
//----------------------------------------------------------------------------
template <typename _Char, typename _It, typename _Sep>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const Fmt::TJoin<_It, _Sep>& join) {
    forrange(it, join.First, join.Last) {
        if (it != join.First)
            oss << join.Separator;
        oss << *it;
    }
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
template <typename... _Args>
struct TStruct {
    FConstChar Name;
    TTuple<_Args...> Tuple;
    TStruct() = default;
    TStruct(FConstChar name, TTuple<_Args...>&& tuple) NOEXCEPT
        : Name(name), Tuple(std::move(tuple))
    {}
};
template <typename T, size_t _Depth = 0>
constexpr auto Struct(const T& trivial, std::integral_constant<size_t, _Depth> = {});
template <typename T, size_t _Depth = 0>
constexpr auto Struct(const T* ptr, std::integral_constant<size_t, _Depth> = {});
template <typename... _Args, size_t _Depth = 0>
constexpr auto Struct(const TTuple<_Args...>& tuple, std::integral_constant<size_t, _Depth> = {});
template <typename... _Args, size_t _Depth = 0>
constexpr auto Struct(FConstChar name, const TTuple<_Args...>& tuple, std::integral_constant<size_t, _Depth> = {});
template <typename T, size_t _Depth>
constexpr auto Struct(const T& trivial, std::integral_constant<size_t, _Depth>) {
    IF_CONSTEXPR(_Depth < 3 && Meta::is_type_complete_v<T>) {
        IF_CONSTEXPR(has_tie_as_tuple_v<T> && struct_num_fields_v<T> > 1) {
            return Struct(Meta::type_info<T>.name, tie_as_tuple(trivial), std::integral_constant<size_t, _Depth + 1>{});
        }
        else {
            return trivial;
        }
    }
    else {
        return trivial;
    }
}
template <typename T, size_t _Depth>
constexpr auto Struct(const T* ptr, std::integral_constant<size_t, _Depth>) {
    IF_CONSTEXPR(_Depth < 3 && Meta::is_type_complete_v<T>) {
        IF_CONSTEXPR(has_tie_as_tuple_v<T> && struct_num_fields_v<T> > 1) {
            if (ptr)
                return Struct(Meta::type_info<T>.name, tie_as_tuple(*ptr), std::integral_constant<size_t, _Depth + 1>{});
            return decltype(Struct(Meta::type_info<T>.name, tie_as_tuple(*ptr), std::integral_constant<size_t, _Depth + 1>{})){};
        }
        else {
            return ptr;
        }
    }
    else {
        return ptr;
    }
}
template <typename... _Args, size_t _Depth>
constexpr auto Struct(FConstChar name, const TTuple<_Args...>& tuple, std::integral_constant<size_t, _Depth> depth) {
    return Meta::static_for<sizeof...(_Args)>([&](auto... index) CONSTEXPR {
        return TStruct{ name, std::make_tuple(Struct(std::get<index>(tuple), depth)...) };
    });
}
template <typename... _Args, size_t _Depth>
constexpr auto Struct(const TTuple<_Args...>& tuple, std::integral_constant<size_t, _Depth> depth) {
    return Struct(FConstChar{}, tuple, depth);
}
template <typename... _Args>
auto MakeStruct(FConstChar name, _Args... args) {
    return Struct(name, std::make_tuple(std::forward<_Args>(args)...));
}
} //!namespace Fmt
//----------------------------------------------------------------------------
template <typename _Char, typename... _Args>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const Fmt::TStruct<_Args...>& fmt) {
    Meta::static_for<sizeof...(_Args)>([&](auto... index) {
        auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, ", "));
        oss << fmt.Name.MakeView() << STRING_LITERAL(_Char, '{');
        if (fmt.Name.Data)
            oss << Endl;
        FOLD_EXPR(oss << sep << std::get<index>(fmt.Tuple));
        oss << STRING_LITERAL(_Char, '}');
    });
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
struct FHexDump {
    TMemoryView<const u8> RawData;
    u32 BytesPerRow;
    bool Absolute = false;
    FHexDump(const TMemoryView<const u8>& rawData, u32 bytesPerRow = 16, bool absolute = true) NOEXCEPT
        : RawData(rawData), BytesPerRow(bytesPerRow), Absolute(absolute) {}
};
template <typename T>
FHexDump HexDump(const TMemoryView<T>& data, u32 bytesPerRow = 16, bool absolute = true) NOEXCEPT {
    return FHexDump(data.template Cast<const u8>(), bytesPerRow, absolute);
}
template <typename T>
FHexDump HexDump(T* data, size_t n, u32 bytesPerRow = 16, bool absolute = true) NOEXCEPT {
    return HexDump(TMemoryView<T>(data, n), bytesPerRow, absolute);
}
} //!namespace Fmt
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, const Fmt::FHexDump& hexDump);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, const Fmt::FHexDump& hexDump);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
struct FBase64 {
    TMemoryView<const u8> RawData;
    explicit FBase64(const TMemoryView<const u8>& rawData) NOEXCEPT
    :   RawData(rawData) {}
};
template <typename T>
FBase64 Base64(const TMemoryView<T>& data) NOEXCEPT {
    return FBase64(data.template Cast<const u8>());
}
template <typename T>
FBase64 Base64(T* data, size_t n) NOEXCEPT {
    return FBase64(TMemoryView<T>(data, n));
}
} //!namespace Fmt
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API size_t Base64EncodeSize(const FRawMemoryConst& src) NOEXCEPT;
NODISCARD PPE_CORE_API size_t Base64EncodeSize(const FRawMemoryConst& src) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API void Base64Encode(const FRawMemoryConst& src, const TAppendable<char>& dst) NOEXCEPT;
PPE_CORE_API void Base64Encode(const FRawMemoryConst& src, const TAppendable<wchar_t>& dst) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API void Base64Encode(const FRawMemoryConst& src, const TMemoryView<char>& dst) NOEXCEPT;
PPE_CORE_API void Base64Encode(const FRawMemoryConst& src, const TMemoryView<wchar_t>& dst) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API size_t Base64DecodeSize(const FStringView& src) NOEXCEPT;
NODISCARD PPE_CORE_API size_t Base64DecodeSize(const FWStringView& src) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool Base64Decode(const FStringView& src, const TAppendable<u8>& dst) NOEXCEPT;
NODISCARD PPE_CORE_API bool Base64Decode(const FWStringView& src, const TAppendable<u8>& dst) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool Base64Decode(const FStringView& src, const TMemoryView<u8>& dst) NOEXCEPT;
NODISCARD PPE_CORE_API bool Base64Decode(const FWStringView& src, const TMemoryView<u8>& dst) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, const Fmt::FBase64& b64);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, const Fmt::FBase64& b64);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
template <typename _Char>
struct TBasicIndent {
    explicit TBasicIndent(const TBasicStringView<_Char>& tab) : Tab(tab) {}
    TBasicStringView<_Char> Tab;
    int Level = 0;
    TBasicIndent& Inc() { ++Level; return *this; }
    TBasicIndent& Dec() { Assert(Level > 0); --Level; return *this; }
    struct FScope {
        TBasicIndent& Indent;
        FScope(TBasicIndent& indent) NOEXCEPT : Indent(indent) { Indent.Inc(); }
        ~FScope() { Indent.Dec(); }
    };
};
struct FIndent : TBasicIndent<char> {
    using TBasicIndent<char>::FScope;

    explicit FIndent(const FStringView& tab = "    ") NOEXCEPT
        : TBasicIndent<char>(tab) {}

    static FIndent UsingTabs() { return FIndent{ "\t" }; }
    static FIndent TwoSpaces() { return FIndent{ "  " }; }
    static FIndent FourSpaces() { return FIndent{ "    " }; }
    static FIndent None() { return FIndent{ "" }; }
};
struct FWIndent : TBasicIndent<wchar_t> {
    using TBasicIndent<wchar_t>::FScope;

    explicit FWIndent(const FWStringView& tab = L"    ") NOEXCEPT
        : TBasicIndent<wchar_t>(tab) {}

    static FWIndent UsingTabs() { return FWIndent{ L"\t" }; }
    static FWIndent TwoSpaces() { return FWIndent{ L"  " }; }
    static FWIndent FourSpaces() { return FWIndent{ L"    " }; }
    static FWIndent None() { return FWIndent{ L"" }; }
};
} //!namespace Fmt
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, const Fmt::TBasicIndent<char>& indent);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, const Fmt::TBasicIndent<wchar_t>& indent);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
template <typename _Char>
using TBasicFormator = TFunction<void (TBasicTextWriter<_Char>&)>;
using FFormator = TBasicFormator<char>;
using FWFormator = TBasicFormator<wchar_t>;
template <typename _Char, typename _Lambda>
TBasicFormator<_Char> Formator(_Lambda&& lmb) {
    return TBasicFormator<_Char>{ std::move(lmb) };
}
} //!namespace Fmt
//----------------------------------------------------------------------------
inline FTextWriter& operator <<(FTextWriter& oss, const Fmt::FFormator& formator) {
    formator(oss);
    return oss;
}
//----------------------------------------------------------------------------
inline FWTextWriter& operator <<(FWTextWriter& woss, const Fmt::FWFormator& wformator) {
    wformator(woss);
    return woss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
    enum EChar {
        LBrace,
        RBrace,
        LBracket,
        RBracket,
        LParenthese,
        RParenthese,
        Comma,
        Colon,
        SemiColon,
        Dot,
        Dollar,
        Question,
        Add,
        Sub,
        Mul,
        Div,
        Mod,
        Pow,
        Increment,
        Decrement,
        LShift,
        RShift,
        And,
        Or,
        Not,
        Xor,
        Complement,
        Assignment,
        Equals,
        NotEquals,
        Less,
        LessOrEqual,
        Greater,
        GreaterOrEqual,
        DotDot,
        Sharp,
        Quote,
        DoubleQuote,
        Space,
        Tab,
        Zero,
        A,
        a,
        Eol,
    };
} //!namespace Fmt
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, Fmt::EChar ch);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, Fmt::EChar ch);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
