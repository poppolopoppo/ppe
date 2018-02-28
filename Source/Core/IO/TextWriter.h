#pragma once

#include "Core/Core.h"

#include "Core/IO/StringView.h"
#include "Core/IO/TextWriter_fwd.h"
#include "Core/Memory/MemoryProvider.h"
#include "Core/Meta/Function.h"

namespace Core {
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
using TBasicTextManipulator = Meta::TFunction<TBasicTextWriter<_Char>&(TBasicTextWriter<_Char>&)>;
using FTextManipulator = TBasicTextManipulator<char>;
using FWTextManipulator = TBasicTextManipulator<wchar_t>;
//----------------------------------------------------------------------------
class CORE_API FTextFormat {
public:
    enum EBase : u32 {
        Decimal = 0, // default
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
        BoolAlpha = 1 << 0,
        //Reserved0 = 1 << 1,
        //Reserved1 = 1 << 2,
        //Reserved2 = 1 << 3,
        //Reserved3 = 1 << 4,
        //Reserved4 = 1 << 5,
        //Reserved5 = 1 << 6,
        //Reserved6 = 1 << 7,
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

    EBase Base() const { return _base; }
    ECase Case() const { return _case; }
    EFloat Float() const { return _float; }
    EPadding Padding() const { return _padding; }
    EMisc Misc() const { return _misc; }
    size_t Width() const { return _width; }
    size_t Precision() const { return _precision; }

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

    inline friend void swap(FTextFormat& lhs, FTextFormat& rhs) {
        std::swap(reinterpret_cast<u32&>(lhs), reinterpret_cast<u32&>(rhs));
    }

    template <typename _Char>
    static TBasicTextWriter<_Char>& DontPad(TBasicTextWriter<_Char>& s);
    template <typename _Char>
    static TBasicTextManipulator<_Char> Pad(EPadding padding, size_t width, _Char fill);
    template <typename _Char>
    static TBasicTextManipulator<_Char> PadCenter(size_t width, _Char fill);
    template <typename _Char>
    static TBasicTextManipulator<_Char> PadLeft(size_t width, _Char fill);
    template <typename _Char>
    static TBasicTextManipulator<_Char> PadRight(size_t width, _Char fill);
    template <typename _Char>
    static TBasicTextManipulator<_Char> SetFill(_Char ch);

    struct FFloat {
        EFloat Float;
        size_t Precision;
    };

    static FFloat Float(EFloat flt, size_t precision) {
        return FFloat{ flt, precision };
    }

private:
    EBase _base         : 2;
    ECase _case         : 2;
    EFloat _float       : 2;
    EPadding _padding   : 2;
    EMisc _misc         : 8;
    u32 _width          : 8;
    u32 _precision      : 8;
};
STATIC_ASSERT(sizeof(FTextFormat) == sizeof(u32));
//----------------------------------------------------------------------------
class CORE_API FBaseTextWriter {
public:
    FBaseTextWriter(IBufferedStreamWriter* ostream);
    ~FBaseTextWriter();

    IBufferedStreamWriter* Stream() const { return _ostream; }

    FTextFormat& Format() { return _format; }
    const FTextFormat& Format() const { return _format; }

    FTextFormat SetFormat(const FTextFormat& fmt);
    FTextFormat ResetFormat();

    void Flush();
    void Reset();

protected:
    IBufferedStreamWriter* _ostream;
    FTextFormat _format;
};
//----------------------------------------------------------------------------
template <typename _Char>
class TBasicTextWriter : public FBaseTextWriter {
public:
    static constexpr _Char DefaultFillChar();

    TBasicTextWriter(IBufferedStreamWriter* s) noexcept
        : FBaseTextWriter(s)
        , _fillChar(DefaultFillChar())
    {}

    ~TBasicTextWriter() {}

    _Char FillChar() const { return _fillChar; }
    _Char SetFillChar(_Char ch) { _Char old = _fillChar; _fillChar = ch; return old; }

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
    void Write(const void* v);
    void Write(const _Char* v);
    void Write(const TBasicStringView<_Char>& v);

    inline friend TBasicTextWriter& operator <<(TBasicTextWriter& w, bool v) { w.Write(v); return w; }
    inline friend TBasicTextWriter& operator <<(TBasicTextWriter& w, i8 v) { w.Write(v); return w; }
    inline friend TBasicTextWriter& operator <<(TBasicTextWriter& w, i16 v) { w.Write(v); return w; }
    inline friend TBasicTextWriter& operator <<(TBasicTextWriter& w, i32 v) { w.Write(v); return w; }
    inline friend TBasicTextWriter& operator <<(TBasicTextWriter& w, i64 v) { w.Write(v); return w; }
    inline friend TBasicTextWriter& operator <<(TBasicTextWriter& w, u8 v) { w.Write(v); return w; }
    inline friend TBasicTextWriter& operator <<(TBasicTextWriter& w, u16 v) { w.Write(v); return w; }
    inline friend TBasicTextWriter& operator <<(TBasicTextWriter& w, u32 v) { w.Write(v); return w; }
    inline friend TBasicTextWriter& operator <<(TBasicTextWriter& w, u64 v) { w.Write(v); return w; }
    inline friend TBasicTextWriter& operator <<(TBasicTextWriter& w, float v) { w.Write(v); return w; }
    inline friend TBasicTextWriter& operator <<(TBasicTextWriter& w, double v) { w.Write(v); return w; }
    inline friend TBasicTextWriter& operator <<(TBasicTextWriter& w, const void* v) { w.Write(v); return w; }
    inline friend TBasicTextWriter& operator <<(TBasicTextWriter& w, const _Char* v) { w.Write(v); return w; }
    inline friend TBasicTextWriter& operator <<(TBasicTextWriter& w, const TBasicStringView<_Char>& v) { w.Write(v); return w; }

    // need those for complete support of integral types and remove ambiguous calls :
    inline friend TBasicTextWriter& operator <<(TBasicTextWriter& w, long v) { w.Write(checked_cast<i32>(v)); return w; }
    inline friend TBasicTextWriter& operator <<(TBasicTextWriter& w, unsigned long v) { w.Write(checked_cast<u32>(v)); return w; }

    template <size_t _Dim>
    inline friend TBasicTextWriter& operator <<(TBasicTextWriter& w, const _Char(&v)[_Dim]) { w.Write(MakeStringView(v)); return w; }
    inline friend TBasicTextWriter& operator <<(TBasicTextWriter& w, _Char v) { w.Write(TBasicStringView<_Char>(&v, 1)); return w; }

    using FManipulator = TBasicTextManipulator<_Char>;
    inline friend TBasicTextWriter& operator <<(TBasicTextWriter& w, const FManipulator& f) { return f(w); }
    inline friend TBasicTextWriter& operator <<(TBasicTextWriter& w, TBasicTextWriter& (*f)(TBasicTextWriter&)) { return f(w); }

    inline friend void swap(TBasicTextWriter& lhs, TBasicTextWriter& rhs) {
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

        TBasicTextWriter& operator *() const {
            return Owner;
        }
    };

    FFormatScope FormatScope() {
        return FFormatScope(*this);
    }

private:
    _Char _fillChar;
};
//----------------------------------------------------------------------------
template <> constexpr char TBasicTextWriter<char>::DefaultFillChar() { return ' '; }
template <> constexpr wchar_t TBasicTextWriter<wchar_t>::DefaultFillChar() { return L' '; }
//----------------------------------------------------------------------------
template <> CORE_API void TBasicTextWriter<char>::Put(char ch);
template <> CORE_API void TBasicTextWriter<wchar_t>::Put(wchar_t ch);
//----------------------------------------------------------------------------
template <> CORE_API void TBasicTextWriter<char>::Put(const TBasicStringView<char>& str);
template <> CORE_API void TBasicTextWriter<wchar_t>::Put(const TBasicStringView<wchar_t>& str);
//----------------------------------------------------------------------------
template <> CORE_API void TBasicTextWriter<char>::Write(bool v);
template <> CORE_API void TBasicTextWriter<char>::Write(i8 v);
template <> CORE_API void TBasicTextWriter<char>::Write(i16 v);
template <> CORE_API void TBasicTextWriter<char>::Write(i32 v);
template <> CORE_API void TBasicTextWriter<char>::Write(i64 v);
template <> CORE_API void TBasicTextWriter<char>::Write(u8 v);
template <> CORE_API void TBasicTextWriter<char>::Write(u16 v);
template <> CORE_API void TBasicTextWriter<char>::Write(u32 v);
template <> CORE_API void TBasicTextWriter<char>::Write(u64 v);
template <> CORE_API void TBasicTextWriter<char>::Write(float v);
template <> CORE_API void TBasicTextWriter<char>::Write(double v);
template <> CORE_API void TBasicTextWriter<char>::Write(const void* v);
template <> CORE_API void TBasicTextWriter<char>::Write(const char* v);
template <> CORE_API void TBasicTextWriter<char>::Write(const TBasicStringView<char>& v);
//----------------------------------------------------------------------------
template <> CORE_API void TBasicTextWriter<wchar_t>::Write(bool v);
template <> CORE_API void TBasicTextWriter<wchar_t>::Write(i8 v);
template <> CORE_API void TBasicTextWriter<wchar_t>::Write(i16 v);
template <> CORE_API void TBasicTextWriter<wchar_t>::Write(i32 v);
template <> CORE_API void TBasicTextWriter<wchar_t>::Write(i64 v);
template <> CORE_API void TBasicTextWriter<wchar_t>::Write(u8 v);
template <> CORE_API void TBasicTextWriter<wchar_t>::Write(u16 v);
template <> CORE_API void TBasicTextWriter<wchar_t>::Write(u32 v);
template <> CORE_API void TBasicTextWriter<wchar_t>::Write(u64 v);
template <> CORE_API void TBasicTextWriter<wchar_t>::Write(float v);
template <> CORE_API void TBasicTextWriter<wchar_t>::Write(double v);
template <> CORE_API void TBasicTextWriter<wchar_t>::Write(const void* v);
template <> CORE_API void TBasicTextWriter<wchar_t>::Write(const wchar_t* v);
template <> CORE_API void TBasicTextWriter<wchar_t>::Write(const TBasicStringView<wchar_t>& v);
//----------------------------------------------------------------------------
extern CORE_API template class TBasicTextWriter<char>;
extern CORE_API template class TBasicTextWriter<wchar_t>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TBasicTextWriter<char>& Crlf(TBasicTextWriter<char>& s);
TBasicTextWriter<wchar_t>& Crlf(TBasicTextWriter<wchar_t>& s);
//----------------------------------------------------------------------------
TBasicTextWriter<char>& Eol(TBasicTextWriter<char>& s);
TBasicTextWriter<wchar_t>& Eol(TBasicTextWriter<wchar_t>& s);
//----------------------------------------------------------------------------
TBasicTextWriter<char>& Eos(TBasicTextWriter<char>& s);
TBasicTextWriter<wchar_t>& Eos(TBasicTextWriter<wchar_t>& s);
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Endl(TBasicTextWriter<_Char>& s);
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

    using textwriter_type::Write;

    FMemoryViewWriter* Stream() { return static_cast<FMemoryViewWriter*>(this); }
    const FMemoryViewWriter* Stream() const { return static_cast<const FMemoryViewWriter*>(this); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/IO/TextWriter-inl.h"
