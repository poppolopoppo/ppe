#pragma once

#include "Core.h"

#include "IO/StringView.h"
#include "IO/TextWriter_fwd.h"
#include "Memory/MemoryProvider.h"
#include "Misc/Function.h"

namespace PPE {
class IBufferedStreamWriter;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define STACKLOCAL_TEXTWRITER(_NAME, _COUNT) \
    MALLOCA_POD(char, CONCAT(_Alloca_, _NAME), _COUNT); \
    FFixedSizeTextWriter _NAME(CONCAT(_Alloca_, _NAME).MakeView())
//----------------------------------------------------------------------------
#define STACKLOCAL_WTEXTWRITER(_NAME, _COUNT) \
    MALLOCA_POD(wchar_t, CONCAT(_Alloca_, _NAME), _COUNT); \
    FWFixedSizeTextWriter _NAME(CONCAT(_Alloca_, _NAME).MakeView())
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
using TBasicTextManipulator = TFunction<TBasicTextWriter<_Char>& (TBasicTextWriter<_Char>&)>;
using FTextManipulator = TBasicTextManipulator<char>;
using FWTextManipulator = TBasicTextManipulator<wchar_t>;
//----------------------------------------------------------------------------
template <typename _Char>
using TBasicTextManipulatorRef = TFunctionRef<TBasicTextWriter<_Char>& (TBasicTextWriter<_Char>&)>;
using FTextManipulatorRef = TBasicTextManipulatorRef<char>;
using FWTextManipulatorRef = TBasicTextManipulatorRef<wchar_t>;
//----------------------------------------------------------------------------
class PPE_CORE_API FTextFormat {
public:
    enum EBase : u32 {
        Decimal     = 0, // default
        Binary,
        Hexadecimal,
        Octal,
    };
    enum ECase : u32 {
        Original = 0,
        Lowercase,
        Uppercase,
        Capitalize,
    };
    enum EFloat : u32 {
        DefaultFloat = 0,
        FixedFloat,
        PrecisionFloat,
        ScientificFloat,
    };
    enum EPadding : u32 {
        Padding_None = 0,
        Padding_Center,
        Padding_Left,
        Padding_Right,
    };
    enum EMisc : u32 {
        BoolAlpha   = 1 << 0,
        Compact     = 1 << 1,
        Crlf        = 1 << 2,
        Escape      = 1 << 3,
        Quote       = 1 << 4,
        TruncateR   = 1 << 5,
        TruncateL   = 1 << 6,

        _Truncate   = (TruncateL | TruncateR),
    };
    ENUM_FLAGS_FRIEND(EMisc);

    STATIC_CONST_INTEGRAL(size_t, GFloatDefaultPrecision, std::numeric_limits<long double>::digits10 + 1);

    CONSTEXPR FTextFormat() NOEXCEPT {
        _base = Decimal;
        _case = Original;
        _float = DefaultFloat;
        _padding = Padding_None;
        _misc = EMisc(0);
        _width = 0;
        _precision = GFloatDefaultPrecision;
    }

    CONSTEXPR CONSTF EBase Base() const NOEXCEPT { return _base; }
    CONSTEXPR CONSTF ECase Case() const NOEXCEPT { return _case; }
    CONSTEXPR CONSTF EFloat Float() const NOEXCEPT { return _float; }
    CONSTEXPR CONSTF EPadding Padding() const NOEXCEPT { return _padding; }
    CONSTEXPR CONSTF EMisc Misc() const NOEXCEPT { return _misc; }
    CONSTEXPR CONSTF size_t Width() const NOEXCEPT { return _width; }
    CONSTEXPR CONSTF size_t Precision() const NOEXCEPT { return _precision; }

    CONSTEXPR void SetBase(EBase v) NOEXCEPT { _base = v; Assert(v == _base); }
    CONSTEXPR void SetCase(ECase v) NOEXCEPT { _case = v; Assert(v == _case); }
    CONSTEXPR void SetFloat(EFloat v) NOEXCEPT { _float = v; Assert(v == _float); }
    CONSTEXPR void SetMisc(EMisc v, bool enabled = true) NOEXCEPT { _misc = (enabled ? _misc + v : _misc - v); }
    CONSTEXPR void SetPadding(EPadding v) NOEXCEPT { _padding = v; Assert(v == _padding); }
    CONSTEXPR void SetWidth(size_t v) NOEXCEPT { _width = u8(v); Assert(v == _width); } // reset after each output
    CONSTEXPR void SetPrecision(size_t v) NOEXCEPT { _precision = u8(v); Assert(v == _precision); }

    CONSTEXPR bool Equals(const FTextFormat& other) const NOEXCEPT {
        return (_raw == other._raw);
    }

    CONSTEXPR inline friend bool operator ==(const FTextFormat& lhs, const FTextFormat& rhs) NOEXCEPT { return lhs.Equals(rhs); }
    CONSTEXPR inline friend bool operator !=(const FTextFormat& lhs, const FTextFormat& rhs) NOEXCEPT { return not lhs.Equals(rhs); }

    CONSTEXPR inline friend void swap(FTextFormat& lhs, FTextFormat& rhs) NOEXCEPT {
        std::swap(lhs._raw, rhs._raw);
    }

    static FTextWriter& DontPad(FTextWriter& s);
    static FTextManipulator Pad(EPadding padding, size_t width = INDEX_NONE, char fill = char());
    static FTextManipulator PadCenter(size_t width = INDEX_NONE, char fill = char());
    static FTextManipulator PadLeft(size_t width = INDEX_NONE, char fill = char());
    static FTextManipulator PadRight(size_t width = INDEX_NONE, char fill = char());
    static FTextManipulator SetFill(char ch);

    static FWTextWriter& DontPad(FWTextWriter& s);
    static FWTextManipulator Pad(EPadding padding, size_t width = INDEX_NONE, wchar_t fill = wchar_t());
    static FWTextManipulator PadCenter(size_t width = INDEX_NONE, wchar_t fill = wchar_t());
    static FWTextManipulator PadLeft(size_t width = INDEX_NONE, wchar_t fill = wchar_t());
    static FWTextManipulator PadRight(size_t width = INDEX_NONE, wchar_t fill = wchar_t());
    static FWTextManipulator SetFill(wchar_t ch);

    struct FFloat {
        EFloat Float;
        size_t Precision;
    };

    static CONSTEXPR FFloat Float(EFloat flt, size_t precision) {
        return FFloat{ flt, precision };
    }

    friend CONSTEXPR EBase operator ""_base (unsigned long long value) {
        switch (value) {
        case 2: return Binary;
        case 8: return Octal;
        case 10: return Decimal;
        case 16: return Hexadecimal;
        default: AssertNotReached();
        }
    }

private:
    union {
        struct {
            EBase _base         : 2;
            ECase _case         : 2;
            EFloat _float       : 2;
            EPadding _padding   : 3;
            EMisc _misc         : 7;
            
            u32 _width          : 8;
            u32 _precision      : 8;
        };

        u32 _raw;
    };
};
STATIC_ASSERT(sizeof(FTextFormat) == sizeof(u32));
//----------------------------------------------------------------------------
class PPE_CORE_API FBaseTextWriter {
public:
    FBaseTextWriter(TPtrRef<IStreamWriter> ostream) NOEXCEPT;
    ~FBaseTextWriter() NOEXCEPT;

    IStreamWriter& Stream() const { return _ostream; }

    CONSTF FTextFormat& Format() { return _format; }
    CONSTF const FTextFormat& Format() const { return _format; }

    FTextFormat SetFormat(const FTextFormat& fmt);
    FTextFormat ResetFormat();

    void Flush();

protected:
    static void BasePut(FTextWriter* oss, char ch);
    static void BasePut(FTextWriter* oss, const FStringView& str);
    static void BasePut(FTextWriter* oss, const FStringLiteral& str);
    static void BaseWrite(FTextWriter* oss, bool v);
    static void BaseWrite(FTextWriter* oss, i8 v);
    static void BaseWrite(FTextWriter* oss, i16 v);
    static void BaseWrite(FTextWriter* oss, i32 v);
    static void BaseWrite(FTextWriter* oss, i64 v);
    static void BaseWrite(FTextWriter* oss, u8 v);
    static void BaseWrite(FTextWriter* oss, u16 v);
    static void BaseWrite(FTextWriter* oss, u32 v);
    static void BaseWrite(FTextWriter* oss, u64 v);
    static void BaseWrite(FTextWriter* oss, float v);
    static void BaseWrite(FTextWriter* oss, double v);
    static void BaseWrite(FTextWriter* oss, const void* v);
    static void BaseWrite(FTextWriter* oss, const char* v);
    static void BaseWrite(FTextWriter* oss, const FStringView& v);
    static void BaseWrite(FTextWriter* oss, const FStringLiteral& v);

    static void BasePut(FWTextWriter* oss, wchar_t ch);
    static void BasePut(FWTextWriter* oss, const FWStringView& str);
    static void BasePut(FWTextWriter* oss, const FWStringLiteral& str);
    static void BaseWrite(FWTextWriter* oss, bool v);
    static void BaseWrite(FWTextWriter* oss, i8 v);
    static void BaseWrite(FWTextWriter* oss, i16 v);
    static void BaseWrite(FWTextWriter* oss, i32 v);
    static void BaseWrite(FWTextWriter* oss, i64 v);
    static void BaseWrite(FWTextWriter* oss, u8 v);
    static void BaseWrite(FWTextWriter* oss, u16 v);
    static void BaseWrite(FWTextWriter* oss, u32 v);
    static void BaseWrite(FWTextWriter* oss, u64 v);
    static void BaseWrite(FWTextWriter* oss, float v);
    static void BaseWrite(FWTextWriter* oss, double v);
    static void BaseWrite(FWTextWriter* oss, const void* v);
    static void BaseWrite(FWTextWriter* oss, const wchar_t* v);
    static void BaseWrite(FWTextWriter* oss, const FWStringView& v);
    static void BaseWrite(FWTextWriter* oss, const FWStringLiteral& v);

    TPtrRef<IStreamWriter> _ostream;
    FTextFormat _format;
};
//----------------------------------------------------------------------------
template <typename _Char>
class TBasicTextWriter : public FBaseTextWriter {
public:
    static constexpr _Char DefaultFillChar() {
        return STRING_LITERAL(_Char, ' ');
    }

    TBasicTextWriter(TPtrRef<IStreamWriter> s) noexcept
        : FBaseTextWriter(s)
        , _fillChar(DefaultFillChar())
    {}

    ~TBasicTextWriter() = default;

    CONSTF _Char FillChar() const { return _fillChar; }
    _Char SetFillChar(_Char ch) {
        Assert(_Char() != ch);
        _Char old = _fillChar;
        _fillChar = ch;
        return old;
    }

    // won't format the text :
    void Put(_Char ch) { return FBaseTextWriter::BasePut(this, ch); }
    void Put(const TBasicStringView<_Char>& str) { return FBaseTextWriter::BasePut(this, str); }
    void Put(const TBasicStringLiteral<_Char>& str) { return FBaseTextWriter::BasePut(this, str.MakeView()); }

    // will be formated using FTextFormat :
    void Write(bool v) { return FBaseTextWriter::BaseWrite(this, v); }
    void Write(i8 v) { return FBaseTextWriter::BaseWrite(this, v); }
    void Write(i16 v) { return FBaseTextWriter::BaseWrite(this, v); }
    void Write(i32 v) { return FBaseTextWriter::BaseWrite(this, v); }
    void Write(i64 v) { return FBaseTextWriter::BaseWrite(this, v); }
    void Write(u8 v) { return FBaseTextWriter::BaseWrite(this, v); }
    void Write(u16 v) { return FBaseTextWriter::BaseWrite(this, v); }
    void Write(u32 v) { return FBaseTextWriter::BaseWrite(this, v); }
    void Write(u64 v) { return FBaseTextWriter::BaseWrite(this, v); }
    void Write(float v) { return FBaseTextWriter::BaseWrite(this, v); }
    void Write(double v) { return FBaseTextWriter::BaseWrite(this, v); }
    void Write(const void* v) { return FBaseTextWriter::BaseWrite(this, v); }
    void Write(const _Char* v) { return FBaseTextWriter::BaseWrite(this, v); }
    void Write(const TBasicStringView<_Char>& v) { return FBaseTextWriter::BaseWrite(this, v); }
    void Write(const TBasicStringLiteral<_Char>& v) { return FBaseTextWriter::BaseWrite(this, v.MakeView()); }

    inline friend void swap(TBasicTextWriter& lhs, TBasicTextWriter& rhs) NOEXCEPT {
        using std::swap;
        swap(lhs._ostream, rhs._ostream);
        swap(lhs._format, rhs._format);
        swap(lhs._fillChar, rhs._fillChar);
    }

public:
    struct FFormatScope {
        TBasicTextWriter& Owner;
        const FTextFormat Format;
        const _Char FillChar;

        FFormatScope(TBasicTextWriter& owner)
            : Owner(owner)
            , Format(owner.Format())
            , FillChar(owner.FillChar())
        {}

        ~FFormatScope() {
            Owner.SetFormat(Format);
            Owner.SetFillChar(FillChar);
        }

        TBasicTextWriter& operator *() const { return Owner; }
        TBasicTextWriter* operator->() const { return (&Owner); }
    };

    FFormatScope FormatScope() {
        return FFormatScope(*this);
    }

private:
    _Char _fillChar;
};
//----------------------------------------------------------------------------
#ifndef EXPORT_PPE_RUNTIME_CORE_TEXTWRITER
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TBasicTextWriter<char>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TBasicTextWriter<wchar_t>;
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, bool v) { w.Write(v); return w; }
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, i8 v) { w.Write(v); return w; }
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, i16 v) { w.Write(v); return w; }
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, i32 v) { w.Write(v); return w; }
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, i64 v) { w.Write(v); return w; }
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, u8 v) { w.Write(v); return w; }
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, u16 v) { w.Write(v); return w; }
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, u32 v) { w.Write(v); return w; }
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, u64 v) { w.Write(v); return w; }
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, float v) { w.Write(v); return w; }
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, double v) { w.Write(v); return w; }
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, const void* v) { w.Write(v); return w; }
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, const TBasicStringView<_Char>& v) { w.Write(v); return w; }
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, const TBasicStringLiteral<_Char>& v) { w.Write(v); return w; }
//----------------------------------------------------------------------------
#ifdef PLATFORM_WINDOWS
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, long v) { w.Write(checked_cast<i32>(v)); return w; }
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, unsigned long v) { w.Write(checked_cast<u32>(v)); return w; }
#else
STATIC_ASSERT(std::is_same_v<long, i64>);
STATIC_ASSERT(std::is_same_v<unsigned long, u64>);
STATIC_ASSERT(!std::is_same_v<long long, i64> && sizeof(long long) == sizeof(i64));
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, long long v) { w.Write(checked_cast<i64>(v)); return w; }
#endif
//----------------------------------------------------------------------------
template <typename _Char, size_t _Dim>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, const _Char(&v)[_Dim]) { w.Write(MakeStringView(v)); return w; }
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, _Char v) { w.Write(TBasicStringView<_Char>(&v, 1)); return w; }
//----------------------------------------------------------------------------
template <typename _Char, typename T>
Meta::TEnableIf< // removes ambiguity between const _Char (&)[] and _Char*
    std::is_same_v<const _Char*, T> &&
    std::is_array_v<T> == false,
    TBasicTextWriter<_Char>&
>   operator <<(TBasicTextWriter<_Char>& w, T v) { w.Write(v); return w; }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& s, const FTextFormat& v);
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& s, const FTextFormat::FFloat& v);
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& s, FTextFormat::EBase v);
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& s, FTextFormat::ECase v);
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& s, FTextFormat::EFloat v);
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& s, FTextFormat::EMisc v);
template <typename _Char>
TBasicTextWriter<_Char>& operator >>(TBasicTextWriter<_Char>& s, FTextFormat::EMisc v);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& writer, const FTextManipulator& manip);
PPE_CORE_API FTextWriter& operator <<(FTextWriter& writer, const FTextManipulatorRef& manip);
inline FTextWriter& operator <<(FTextWriter& w, FTextWriter& (*f)(FTextWriter&)) { return f(w); }
//----------------------------------------------------------------------------
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& writer, const FWTextManipulator& manip);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& writer, const FWTextManipulatorRef& manip);
inline FWTextWriter& operator <<(FWTextWriter& w, FWTextWriter& (*f)(FWTextWriter&)) { return f(w); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
class TBasicFixedSizeTextWriter :
    private FMemoryViewWriter
,   public TBasicTextWriter<_Char> {
public:
    typedef TBasicTextWriter<_Char> textwriter_type;

    explicit TBasicFixedSizeTextWriter(_Char* ptr, size_t count)
        : FMemoryViewWriter(ptr, count)
        , textwriter_type(static_cast<FMemoryViewWriter*>(this))
    {}

    template <size_t _Dim>
    explicit TBasicFixedSizeTextWriter(_Char(&staticArray)[_Dim])
        : FMemoryViewWriter(staticArray)
        , textwriter_type(static_cast<FMemoryViewWriter*>(this))
    {}

    explicit TBasicFixedSizeTextWriter(const TMemoryView<_Char>& storage)
        : FMemoryViewWriter(storage)
        , textwriter_type(static_cast<FMemoryViewWriter*>(this))
    {}

    bool empty() const { return Written().empty(); }
    size_t size() const { return Written().size(); }

    const _Char* data() const { return Written().data(); }
    const _Char* c_str() { return NullTerminated().Data; }

    size_t Tell() const { return Written().size(); }
    void Reset() { FMemoryViewWriter::Reset(); }

    TBasicConstChar<_Char> NullTerminated() {
        TBasicStringView<_Char> written = Written();
        if (written.empty() or written.back() != STRING_LITERAL(_Char, '\0'))
            WritePOD(STRING_LITERAL(_Char, '\0'));
        return written.data();
    }

    TBasicStringView<_Char> Written() const {
        return FMemoryViewWriter::Written().template Cast<const _Char>();
    }

    TBasicStringView<_Char> WrittenSince(size_t off) const {
        return Written().CutStartingAt(off);
    }

    using textwriter_type::Put;
    using textwriter_type::Write;

    FMemoryViewWriter* Stream() { return static_cast<FMemoryViewWriter*>(this); }
    const FMemoryViewWriter* Stream() const { return static_cast<const FMemoryViewWriter*>(this); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#ifndef EXPORT_PPE_RUNTIME_CORE_TEXTWRITER
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) PPE::TFunction<PPE::TBasicTextWriter<char>& (PPE::TBasicTextWriter<char>&)>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) PPE::TFunction<PPE::TBasicTextWriter<wchar_t>& (PPE::TBasicTextWriter<wchar_t>&)>;
#endif

#include "IO/TextWriter-inl.h"
