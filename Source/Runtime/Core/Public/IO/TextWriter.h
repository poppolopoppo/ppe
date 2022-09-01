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
using TBasicTextManipulator = TFunction<TBasicTextWriter<_Char>&(TBasicTextWriter<_Char>&)>;
using FTextManipulator = TBasicTextManipulator<char>;
using FWTextManipulator = TBasicTextManipulator<wchar_t>;
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
        Crlf        = 1 << 1,
        TruncateR   = 1 << 2,
        TruncateL   = 1 << 3,
        //Reserved2 = 1 << 4,
        //Reserved3 = 1 << 5,

        _Truncate   = (TruncateL | TruncateR),
    };
    ENUM_FLAGS_FRIEND(EMisc);

    STATIC_CONST_INTEGRAL(size_t, GFloatDefaultPrecision, std::numeric_limits<long double>::digits10 + 1);

    FTextFormat() {
        _base = Decimal;
        _case = Original;
        _float = DefaultFloat;
        _padding = Padding_None;
        _misc = EMisc(0);
        _width = 0;
        _precision = GFloatDefaultPrecision;
    }

    CONSTF EBase Base() const { return _base; }
    CONSTF ECase Case() const { return _case; }
    CONSTF EFloat Float() const { return _float; }
    CONSTF EPadding Padding() const { return _padding; }
    CONSTF EMisc Misc() const { return _misc; }
    CONSTF size_t Width() const { return _width; }
    CONSTF size_t Precision() const { return _precision; }

    void SetBase(EBase v) { _base = v; Assert(v == _base); }
    void SetCase(ECase v) { _case = v; Assert(v == _case); }
    void SetFloat(EFloat v) { _float = v; Assert(v == _float); }
    void SetMisc(EMisc v, bool enabled = true) { _misc = (enabled ? _misc + v : _misc - v); }
    void SetPadding(EPadding v) { _padding = v; Assert(v == _padding); }
    void SetWidth(size_t v) { _width = u8(v); Assert(v == _width); } // reset after each output
    void SetPrecision(size_t v) { _precision = u8(v); Assert(v == _precision); }

    bool Equals(const FTextFormat& other) const {
        return (reinterpret_cast<const u32&>(*this) == reinterpret_cast<const u32&>(other));
    }

    inline friend bool operator ==(const FTextFormat& lhs, const FTextFormat& rhs) { return lhs.Equals(rhs); }
    inline friend bool operator !=(const FTextFormat& lhs, const FTextFormat& rhs) { return not lhs.Equals(rhs); }

    inline friend void swap(FTextFormat& lhs, FTextFormat& rhs) NOEXCEPT {
        std::swap(reinterpret_cast<u32&>(lhs), reinterpret_cast<u32&>(rhs));
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

    friend CONSTEXPR EBase operator "" _base (unsigned long long value) {
        switch (value) {
        case 2: return Binary;
        case 8: return Octal;
        case 10: return Decimal;
        case 16: return Hexadecimal;
        default: AssertNotReached();
        }
    }

private:
    EBase _base         : 2;
    ECase _case         : 2;
    EFloat _float       : 2;
    EPadding _padding   : 3;
    EMisc _misc         : 7;
    u32 _width          : 8;
    u32 _precision      : 8;
};
STATIC_ASSERT(sizeof(FTextFormat) == sizeof(u32));
//----------------------------------------------------------------------------
class PPE_CORE_API FBaseTextWriter {
public:
    FBaseTextWriter(TPtrRef<IStreamWriter> ostream);
    ~FBaseTextWriter();

    IStreamWriter& Stream() const { return _ostream; }

    CONSTF FTextFormat& Format() { return _format; }
    CONSTF const FTextFormat& Format() const { return _format; }

    FTextFormat SetFormat(const FTextFormat& fmt);
    FTextFormat ResetFormat();

    void Flush();
    void Reset();

protected:
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
    void Put(_Char ch);
    void Put(const TBasicStringView<_Char>& str);

    // will be formated using FTextFormat :
    void Write(bool v);
    void Write(i8 v);
    void Write(i16 v);
    void Write(i32 v);
    void Write(i64 v);
    void Write(u8 v);
    void Write(u16 v);
    void Write(u32 v);
    void Write(u64 v);
    void Write(float v);
    void Write(double v);
    void Write(void* v);
    void Write(const _Char* v);
    void Write(const TBasicStringView<_Char>& v);

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
template <> PPE_CORE_API void TBasicTextWriter<char>::Put(char ch);
template <> PPE_CORE_API void TBasicTextWriter<wchar_t>::Put(wchar_t ch);
//----------------------------------------------------------------------------
template <> PPE_CORE_API void TBasicTextWriter<char>::Put(const TBasicStringView<char>& str);
template <> PPE_CORE_API void TBasicTextWriter<wchar_t>::Put(const TBasicStringView<wchar_t>& str);
//----------------------------------------------------------------------------
template <> PPE_CORE_API void TBasicTextWriter<char>::Write(bool v);
template <> PPE_CORE_API void TBasicTextWriter<char>::Write(i8 v);
template <> PPE_CORE_API void TBasicTextWriter<char>::Write(i16 v);
template <> PPE_CORE_API void TBasicTextWriter<char>::Write(i32 v);
template <> PPE_CORE_API void TBasicTextWriter<char>::Write(i64 v);
template <> PPE_CORE_API void TBasicTextWriter<char>::Write(u8 v);
template <> PPE_CORE_API void TBasicTextWriter<char>::Write(u16 v);
template <> PPE_CORE_API void TBasicTextWriter<char>::Write(u32 v);
template <> PPE_CORE_API void TBasicTextWriter<char>::Write(u64 v);
template <> PPE_CORE_API void TBasicTextWriter<char>::Write(float v);
template <> PPE_CORE_API void TBasicTextWriter<char>::Write(double v);
template <> PPE_CORE_API void TBasicTextWriter<char>::Write(void* v);
template <> PPE_CORE_API void TBasicTextWriter<char>::Write(const char* v);
template <> PPE_CORE_API void TBasicTextWriter<char>::Write(const TBasicStringView<char>& v);
//----------------------------------------------------------------------------
template <> PPE_CORE_API void TBasicTextWriter<wchar_t>::Write(bool v);
template <> PPE_CORE_API void TBasicTextWriter<wchar_t>::Write(i8 v);
template <> PPE_CORE_API void TBasicTextWriter<wchar_t>::Write(i16 v);
template <> PPE_CORE_API void TBasicTextWriter<wchar_t>::Write(i32 v);
template <> PPE_CORE_API void TBasicTextWriter<wchar_t>::Write(i64 v);
template <> PPE_CORE_API void TBasicTextWriter<wchar_t>::Write(u8 v);
template <> PPE_CORE_API void TBasicTextWriter<wchar_t>::Write(u16 v);
template <> PPE_CORE_API void TBasicTextWriter<wchar_t>::Write(u32 v);
template <> PPE_CORE_API void TBasicTextWriter<wchar_t>::Write(u64 v);
template <> PPE_CORE_API void TBasicTextWriter<wchar_t>::Write(float v);
template <> PPE_CORE_API void TBasicTextWriter<wchar_t>::Write(double v);
template <> PPE_CORE_API void TBasicTextWriter<wchar_t>::Write(void* v);
template <> PPE_CORE_API void TBasicTextWriter<wchar_t>::Write(const wchar_t* v);
template <> PPE_CORE_API void TBasicTextWriter<wchar_t>::Write(const TBasicStringView<wchar_t>& v);
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
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, void* v) { w.Write(v); return w; }
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, const TBasicStringView<_Char>& v) { w.Write(v); return w; }
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
inline FTextWriter& operator <<(FTextWriter& w, FTextWriter& (*f)(FTextWriter&)) { return f(w); }
//----------------------------------------------------------------------------
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& writer, const FWTextManipulator& manip);
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

    size_t Tell() const { return Written().size(); }
    void Reset() { FMemoryViewWriter::Reset(); }

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
