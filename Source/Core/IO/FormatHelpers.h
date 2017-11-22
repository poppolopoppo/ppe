#pragma once

#include "Core/Core.h"

#include "Core/IO/StringView.h"
#include "Core/Memory/MemoryView.h"
    #include "Core/Meta/Function.h"
#include "Core/Meta/StronglyTyped.h"

#include <iosfwd>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
CORE_STRONGLYTYPED_NUMERIC_DEF(uint64_t, FCountOfElements);
void Format(char *buffer, size_t capacity, FCountOfElements count);
void Format(wchar_t *buffer, size_t capacity, FCountOfElements count);
} //!namespace Fmt
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const Fmt::FCountOfElements& count) {
    _Char buffer[32];
    Fmt::Format(buffer, lengthof(buffer), count);
    return oss << buffer;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
CORE_STRONGLYTYPED_NUMERIC_DEF(intptr_t, FPointer);
void Format(char *buffer, size_t capacity, FPointer ptr);
void Format(wchar_t *buffer, size_t capacity, FPointer ptr);
} //!namespace Fmt
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const Fmt::FPointer& ptr) {
    _Char buffer[32];
    Fmt::Format(buffer, lengthof(buffer), ptr);
    return oss << buffer;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
CORE_STRONGLYTYPED_NUMERIC_DEF(uint64_t, FSizeInBytes);
void Format(char *buffer, size_t capacity, FSizeInBytes value);
void Format(wchar_t *buffer, size_t capacity, FSizeInBytes value);
} //!namespace Fmt
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const Fmt::FSizeInBytes& size) {
    _Char buffer[32];
    Fmt::Format(buffer, lengthof(buffer), size);
    return oss << buffer;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
template <typename T, typename _Char>
struct TPadLeft {
    const T *Value = nullptr;
    size_t Count = 0;
    _Char Char = _Char(' ');
};
template <typename T, typename _Char>
TPadLeft<T, _Char> PadLeft(const T& value, size_t count, _Char ch = _Char(' ')) {
    return TPadLeft<T, _Char>{ &value, count, ch };
}
} //!namespace Fmt
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits, typename T>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const Fmt::TPadLeft<T, _Char>& pad ) {
    const _Char fill = oss.fill();
    const std::streamsize width = oss.width();
    const std::ios_base::fmtflags flags = oss.flags();
    oss << std::setfill(pad.Char) << std::right << std::setw(pad.Count) << *pad.Value;
    oss.fill(fill);
    oss.width(width);
    oss.flags(flags);
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
template <typename T, typename _Char>
struct TPadRight {
    const T *Value = nullptr;
    size_t Count = 0;
    _Char Char = _Char(' ');
};
template <typename T, typename _Char>
TPadRight<T, _Char> PadRight(const T& value, size_t count, _Char ch = _Char(' ')) {
    return TPadRight<T, _Char>{ &value, count, ch };
}
} //!namespace Fmt
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits, typename T>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const Fmt::TPadRight<T, _Char>& pad ) {
    const _Char fill = oss.fill();
    const std::streamsize width = oss.width();
    const std::ios_base::fmtflags flags = oss.flags();
    oss << std::setfill(pad.Char) << std::left << std::setw(pad.Count) << *pad.Value;
    oss.fill(fill);
    oss.width(width);
    oss.flags(flags);
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
template <typename _Char, typename _Traits, typename T>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const Fmt::TRepeater<T>& repeat ) {
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
template <typename _Char, typename _Traits, typename T>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const Fmt::TConditional<T>& cond ) {
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
    return TTernary<T>{ condition, &ifTrue, &ifFalse };
}
} //!namespace Fmt
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits, typename _True, typename _False>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const Fmt::TTernary<_True, _False>& ternary ) {
    Assert(ternary.IfTrue);
    Assert(ternary.IfFalse);
    return ((ternary.Condition)
        ? oss << ternary.IfTrue
        : oss << ternary.IfFalse );
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
std::basic_ostream<_Char>& operator <<(std::basic_ostream<_Char>& oss, const Fmt::TJoin<_Elt, _Sep>& join) {
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
std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, const Fmt::FHexDump& hexDump);
std::basic_ostream<wchar_t>& operator <<(std::basic_ostream<wchar_t>& oss, const Fmt::FHexDump& hexDump);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
struct FIndent {
    explicit FIndent(const FStringView& tab = "    ") : Tab(tab) {}
    FStringView Tab;
    int Level = 0;
    void Inc() { ++Level; }
    void Dec() { Assert(Level > 0); --Level; }
    struct FScope {
        FIndent& Indent;
        FScope(FIndent& indent) : Indent(indent) { Indent.Inc(); }
        ~FScope() { Indent.Dec(); }
    };
    static FIndent UsingTabs()  { return FIndent{ "\t" }; }
    static FIndent TwoSpaces()  { return FIndent{ "  " }; }
    static FIndent FourSpaces() { return FIndent{ "    " }; }
    static FIndent None()       { return FIndent{ "" }; }
};
} //!namespace Fmt
//----------------------------------------------------------------------------
std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, const Fmt::FIndent& indent);
std::basic_ostream<wchar_t>& operator <<(std::basic_ostream<wchar_t>& oss, const Fmt::FIndent& indent);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Fmt {
template <typename _Char>
using TBasicFormator = Meta::TFunction<void (std::basic_ostream<_Char>&)>;
using FFormator = TBasicFormator<char>;
using FWFormator = TBasicFormator<wchar_t>;
} //!namespace Fmt
//----------------------------------------------------------------------------
inline std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, const Fmt::FFormator& formator) {
    formator(oss);
    return oss;
}
//----------------------------------------------------------------------------
inline std::basic_ostream<wchar_t>& operator <<(std::basic_ostream<wchar_t>& woss, const Fmt::FWFormator& wformator) {
    wformator(woss);
    return woss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
