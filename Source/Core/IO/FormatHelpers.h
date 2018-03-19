#pragma once

#include "Core/Core.h"

#include "Core/IO/StringView.h"
#include "Core/IO/TextWriter.h"
#include "Core/Memory/MemoryView.h"
#include "Core/Maths/Units.h"
#include "Core/Meta/Function.h"
#include "Core/Meta/StronglyTyped.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
CORE_STRONGLYTYPED_NUMERIC_DEF(intptr_t, FPointer);
template <typename T>
FPointer Pointer(T* p) { return FPointer{ intptr_t(p) }; }
} //!namespace Fmt
//----------------------------------------------------------------------------
CORE_API FTextWriter& operator <<(FTextWriter& oss, Fmt::FPointer ptr);
CORE_API FWTextWriter& operator <<(FWTextWriter& oss, Fmt::FPointer ptr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
CORE_STRONGLYTYPED_NUMERIC_DEF(float, FPercentage);
template <typename T, typename = Meta::TEnableIf<std::is_arithmetic_v<T>> >
FPercentage Percentage(T x, T total) { return FPercentage{ x * 100.f / total }; }
} //!namespace Fmt
//----------------------------------------------------------------------------
CORE_API FTextWriter& operator <<(FTextWriter& oss, Fmt::FPercentage prc);
CORE_API FWTextWriter& operator <<(FWTextWriter& oss, Fmt::FPercentage prc);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
CORE_STRONGLYTYPED_NUMERIC_DEF(uint64_t, FCountOfElements);
template <typename T>
Meta::TEnableIf<std::is_integral_v<T>, FCountOfElements> CountOfElements(T n) {
    return FCountOfElements{ checked_cast<T>(n) };
}
} //!namespace Fmt
//----------------------------------------------------------------------------
CORE_API FTextWriter& operator <<(FTextWriter& oss, Fmt::FCountOfElements count);
CORE_API FWTextWriter& operator <<(FWTextWriter& oss, Fmt::FCountOfElements count);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
CORE_STRONGLYTYPED_NUMERIC_DEF(uint64_t, FSizeInBytes);
template <typename T>
Meta::TEnableIf<std::is_integral_v<T>, FSizeInBytes> SizeInBytes(T n) {
    return FSizeInBytes{ checked_cast<uint64_t>(n) };
}
} //!namespace Fmt
//----------------------------------------------------------------------------
CORE_API FTextWriter& operator <<(FTextWriter& oss, Fmt::FSizeInBytes size);
CORE_API FWTextWriter& operator <<(FWTextWriter& oss, Fmt::FSizeInBytes size);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
struct FDurationInMs { double Value; };
inline FDurationInMs DurationInMs(FMicroseconds t) {
    return FDurationInMs{ t.Value() };
}
} //!namespace Fmt
//----------------------------------------------------------------------------
CORE_API FTextWriter& operator <<(FTextWriter& oss, Fmt::FDurationInMs v);
CORE_API FWTextWriter& operator <<(FWTextWriter& oss, Fmt::FDurationInMs v);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
template <typename T, typename _Char>
struct TPadded {
    const T* Outp;
    FTextFormat::EPadding Align;
    size_t Width;
    _Char FillChar;
};
template <typename T, typename _Char>
TPadded<T, _Char> PadLeft(const T& value, size_t width = INDEX_NONE, _Char ch = _Char(' ')) {
    return TPadded<T, _Char>{ &value, FTextFormat::Padding_Left, width, ch };
}
template <typename T, typename _Char>
TPadded<T, _Char> PadCenter(const T& value, size_t width = INDEX_NONE, _Char ch = _Char(' ')) {
    return TPadded<T, _Char>{ &value, FTextFormat::Padding_Center, width, ch };
}
template <typename T, typename _Char>
TPadded<T, _Char> PadRight(const T& value, size_t width = INDEX_NONE, _Char ch = _Char(' ')) {
    return TPadded<T, _Char>{ &value, FTextFormat::Padding_Right, width, ch };
}
template <typename _Char>
TPadded<TBasicStringView<_Char>, _Char> AlignLeft(const TBasicStringView<_Char>& value, size_t width = INDEX_NONE, _Char ch = _Char(' ')) {
    return TPadded<TBasicStringView<_Char>, _Char>{ &value, FTextFormat::Padding_Right, width, ch };
}
template <typename _Char>
TPadded<TBasicStringView<_Char>, _Char> AlignCenter(const TBasicStringView<_Char>& value, size_t width = INDEX_NONE, _Char ch = _Char(' ')) {
    return TPadded<TBasicStringView<_Char>, _Char>{ &value, FTextFormat::Padding_Center, width, ch };
}
template <typename _Char>
TPadded<TBasicStringView<_Char>, _Char> AlignRight(const TBasicStringView<_Char>& value, size_t width = INDEX_NONE, _Char ch = _Char(' ')) {
    return TPadded<TBasicStringView<_Char>, _Char>{ &value, FTextFormat::Padding_Left, width, ch };
}
} //!namespace Fmt
//----------------------------------------------------------------------------
template <typename _Char, typename T>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const Fmt::TPadded<T, _Char>& padded) {
    return oss << FTextFormat::Pad(padded.Align, padded.Width, padded.FillChar) << *padded.Outp;
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
TNotFirstTime<T> NotFirstTime(T&& outp) {
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
struct TRepeater {
    const T *Value = nullptr;
    size_t Count = 0;
};
template <typename T>
TRepeater<T> Repeat(const T& value, size_t count) {
    return TRepeater<T>{ &value, count };
}
} //!namespace Fmt
//----------------------------------------------------------------------------
template <typename _Char, typename T>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const Fmt::TRepeater<T>& repeat ) {
    Assert(repeat.Value);
    forrange(i, 0, repeat.Count)
        oss << *repeat.Value;
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
TConditional<T> Conditional(const T& value, bool enabled) {
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
TTernary<_True, _False> Ternary(bool condition, const _True& ifTrue, const _False& ifFalse) {
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
template <typename _Elt, typename _Sep>
struct TJoin {
    TMemoryView<_Elt> Data;
    _Sep Separator;
    TJoin(const TMemoryView<_Elt>& data, _Sep separator)
        : Data(data), Separator(separator) {}
};
template <typename T>
TJoin<T, const char*> CommaSeparated(const TMemoryView<T>& data) {
    return TJoin<T, const char*>(data, ", ");
};
} //!namespace Fmt
//----------------------------------------------------------------------------
template <typename _Char, typename _Elt, typename _Sep>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const Fmt::TJoin<_Elt, _Sep>& join) {
    if (join.Data.size()) {
        oss << join.Data.front();
        forrange(i, 1, join.Data.size())
            oss << join.Separator << join.Data[i];
    }
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
struct FHexDump {
    TMemoryView<const u8> RawData;
    size_t BytesPerRow;
    FHexDump(const TMemoryView<const u8>& rawData, size_t bytesPerRow = 16)
        : RawData(rawData), BytesPerRow(bytesPerRow) {}
};
} //!namespace Fmt
//----------------------------------------------------------------------------
CORE_API FTextWriter& operator <<(FTextWriter& oss, const Fmt::FHexDump& hexDump);
CORE_API FWTextWriter& operator <<(FWTextWriter& oss, const Fmt::FHexDump& hexDump);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
template <typename _Char>
struct TBasicIndent {
    explicit TBasicIndent(const TBasicStringView<_Char>& tab) : Tab(tab) {}
    TBasicStringView<_Char> Tab;
    int Level = 0;
    void Inc() { ++Level; }
    void Dec() { Assert(Level > 0); --Level; }
    struct FScope {
        TBasicIndent& Indent;
        FScope(TBasicIndent& indent) : Indent(indent) { Indent.Inc(); }
        ~FScope() { Indent.Dec(); }
    };
};
struct FIndent : TBasicIndent<char> {
    using TBasicIndent<char>::FScope;

    explicit FIndent(const FStringView& tab = "    ")
        : TBasicIndent<char>(tab) {}

    static FIndent UsingTabs() { return FIndent{ "\t" }; }
    static FIndent TwoSpaces() { return FIndent{ "  " }; }
    static FIndent FourSpaces() { return FIndent{ "    " }; }
    static FIndent None() { return FIndent{ "" }; }
};
struct FWIndent : TBasicIndent<wchar_t> {
    using TBasicIndent<wchar_t>::FScope;

    explicit FWIndent(const FWStringView& tab = L"    ")
        : TBasicIndent<wchar_t>(tab) {}

    static FWIndent UsingTabs() { return FWIndent{ L"\t" }; }
    static FWIndent TwoSpaces() { return FWIndent{ L"  " }; }
    static FWIndent FourSpaces() { return FWIndent{ L"    " }; }
    static FWIndent None() { return FWIndent{ L"" }; }
};
} //!namespace Fmt
//----------------------------------------------------------------------------
CORE_API FTextWriter& operator <<(FTextWriter& oss, const Fmt::TBasicIndent<char>& indent);
CORE_API FWTextWriter& operator <<(FWTextWriter& oss, const Fmt::TBasicIndent<wchar_t>& indent);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
template <typename _Char>
using TBasicFormator = Meta::TFunction<void (TBasicTextWriter<_Char>&)>;
using FFormator = TBasicFormator<char>;
using FWFormator = TBasicFormator<wchar_t>;
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
    };
} //!namespace Fmt
//----------------------------------------------------------------------------
CORE_API FTextWriter& operator <<(FTextWriter& oss, Fmt::EChar ch);
CORE_API FWTextWriter& operator <<(FWTextWriter& oss, Fmt::EChar ch);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
