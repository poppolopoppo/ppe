#include "stdafx.h"

#include "Json/Json.h"

#include "Lexer/Lexer.h"
#include "Lexer/Match.h"
#include "Lexer/Symbols.h"

#include "Container/AssociativeVector.h"
#include "Container/RawStorage.h"
#include "Container/Vector.h"
#include "IO/ConstNames.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/TextWriter.h"
#include "Memory/MemoryProvider.h"
#include "VirtualFileSystem.h"

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
STATIC_ASSERT(sizeof(FJson::FValue) == sizeof(RTTI::FAny));
//----------------------------------------------------------------------------
static void EscapeString_(FTextWriter& oss, const FJson::FText& str) {
    Escape(oss, str.MakeView(), EEscape::Unicode);
}
//----------------------------------------------------------------------------
static void EscapeString_(FTextWriter& oss, const RTTI::FName& token) {
    Escape(oss, token.MakeView(), EEscape::Unicode);
}
//----------------------------------------------------------------------------
static void EscapeString_(FWTextWriter& oss, const FJson::FText& str) {
    Escape(oss, ToWCStr(INLINE_MALLOCA(wchar_t, str.size() + 1), str), EEscape::Unicode);
}
//----------------------------------------------------------------------------
static void EscapeString_(FWTextWriter& oss, const RTTI::FName& token) {
    Escape(oss, ToWCStr(INLINE_MALLOCA(wchar_t, token.size() + 1), token.MakeView()), EEscape::Unicode);
}
//----------------------------------------------------------------------------
static bool ParseValue_(Lexer::FLexer& lexer, FJson& doc, FJson::FValue& value);
//----------------------------------------------------------------------------
static bool ParseObject_(Lexer::FLexer& lexer, FJson& doc, FJson::FValue& value) {
    FJson::FObject& object = value.MakeDefault_AssumeNotValid<FJson::FObject>();

    if (lexer.ReadIFN(Lexer::FSymbols::RBrace)) // quick reject for empty object
        return true;

    Lexer::FMatch key;
    for (bool notFirst = false;; notFirst = true) {
        if (notFirst && not lexer.ReadIFN(Lexer::FSymbols::Comma))
            break;

        if (not lexer.Expect(key, Lexer::FSymbols::String))
            break;

        if (not lexer.Expect(Lexer::FSymbols::Colon))
            PPE_THROW_IT(FJsonException("missing comma", key.Site()));

        FJson::FValue& v = object.Add(RTTI::FName{ key.Value() });
        if (not ParseValue_(lexer, doc, v))
            PPE_THROW_IT(FJsonException("missing value", key.Site()));
    }

    return lexer.Expect(Lexer::FSymbols::RBrace);
}
//----------------------------------------------------------------------------
static bool ParseArray_(Lexer::FLexer& lexer, FJson& doc, FJson::FValue& value) {
    FJson::FArray& arr = value.MakeDefault_AssumeNotValid<FJson::FArray>();

    if (lexer.ReadIFN(Lexer::FSymbols::RBracket)) // quick reject for empty array
        return true;

    for (bool notFirst = false;; notFirst = true) {
        if (notFirst && not lexer.ReadIFN(Lexer::FSymbols::Comma))
            break;

        arr.push_back_Default();
        if (not ParseValue_(lexer, doc, arr.back()) )
            return false;
    }

    return lexer.Expect(Lexer::FSymbols::RBracket);
}
//----------------------------------------------------------------------------
static bool ParseValue_(Lexer::FLexer& lexer, FJson& doc, FJson::FValue& value) {
    Lexer::FMatch read;
    if (not lexer.Read(read))
        return false;

    bool unexpectedToken = false;

    if (read.Symbol() == Lexer::FSymbols::LBrace) {
        return ParseObject_(lexer, doc, value);
    }
    else if (read.Symbol() == Lexer::FSymbols::LBracket) {
        return ParseArray_(lexer, doc, value);
    }
    else if (read.Symbol() == Lexer::FSymbols::String) {
        value.Assign(std::move(read.Value()));
    }
    else if (read.Symbol() == Lexer::FSymbols::Integer) {
        FJson::FInteger& num = value.MakeDefault_AssumeNotValid<FJson::FInteger>();
        if (not Atoi(&num, read.Value().MakeView(), 10))
            PPE_THROW_IT(FJsonException("malformed int", read.Site()));
    }
    else if (read.Symbol() == Lexer::FSymbols::Unsigned) {
        u64 u;
        if (not Atoi(&u, read.Value().MakeView(), 10))
            PPE_THROW_IT(FJsonException("malformed int", read.Site()));
        value.Assign(checked_cast<FJson::FInteger>(u));
    }
    else if (read.Symbol() == Lexer::FSymbols::Float) {
        FJson::FFloat& fp = value.MakeDefault_AssumeNotValid<FJson::FFloat>();
        if (not Atod(&fp, read.Value().MakeView()))
            PPE_THROW_IT(FJsonException("malformed float", read.Site()));
    }
    // unary minus ?
    else if (read.Symbol() == Lexer::FSymbols::Sub) {
        const Lexer::FMatch* const peek = lexer.Peek();

        unexpectedToken = true;

        if (peek && peek->Symbol() == Lexer::FSymbols::Integer) {
            if (ParseValue_(lexer, doc, value)) {
                FJson::FInteger& num = value.FlatData<FJson::FInteger>();
                num = -num;
                unexpectedToken = false;
            }
        }
        else if (peek && peek->Symbol() == Lexer::FSymbols::Unsigned) {
            AssertNotReached(); // -unsigned isn't supported !
        }
        else if (peek && peek->Symbol() == Lexer::FSymbols::Float) {
            if (ParseValue_(lexer, doc, value)) {
                FJson::FFloat& dbl = value.FlatData<FJson::FFloat>();
                dbl = -dbl;
                unexpectedToken = false;
            }
        }
    }
    else if (read.Symbol() == Lexer::FSymbols::True) {
        value.Assign(true);
    }
    else if (read.Symbol() == Lexer::FSymbols::False) {
        value.Assign(false);
    }
    else if (read.Symbol() == Lexer::FSymbols::Null) {
        value.Assign(RTTI::PMetaObject{});
    }
    else {
        unexpectedToken = true;
    }

    if (unexpectedToken)
        PPE_THROW_IT(FJsonException("unexpected token", read.Site()));

    return true;
}
//----------------------------------------------------------------------------
constexpr RTTI::FTypeId Json_TypeId_Null{ RTTI::MakeTypeInfos<RTTI::PMetaObject>().Id() };
constexpr RTTI::FTypeId Json_TypeId_Bool{ RTTI::MakeTypeInfos<FJson::FBool>().Id() };
constexpr RTTI::FTypeId Json_TypeId_Integer{ RTTI::MakeTypeInfos<FJson::FInteger>().Id() };
constexpr RTTI::FTypeId Json_TypeId_Float{ RTTI::MakeTypeInfos<FJson::FFloat>().Id() };
constexpr RTTI::FTypeId Json_TypeId_String{ RTTI::MakeTypeInfos<FJson::FText>().Id() };
constexpr RTTI::FTypeId Json_TypeId_Array{ RTTI::MakeTypeInfos<FJson::FArray>().Id() };
constexpr RTTI::FTypeId Json_TypeId_Object{ RTTI::MakeTypeInfos<FJson::FObject>().Id() };
//----------------------------------------------------------------------------
template <typename _Char>
static void ToStream_(const FJson::FValue& value, TBasicTextWriter<_Char>& oss, Fmt::TBasicIndent<_Char>& indent, bool minify) {
    switch (value.TypeId()) {
    case Json_TypeId_Null:
        oss << "null";
        break;
    case Json_TypeId_Bool:
        oss << value.FlatData<FJson::FBool>();
        break;
    case Json_TypeId_Integer:
        oss << value.FlatData<FJson::FInteger>();
        break;
    case Json_TypeId_Float:
        oss << value.FlatData<FJson::FFloat>();
        break;
    case Json_TypeId_String:
        oss << Fmt::DoubleQuote;
        EscapeString_(oss, value.FlatData<FJson::FText>());
        oss << Fmt::DoubleQuote;
        break;
    case Json_TypeId_Array:
    {
        const FJson::FArray& arr = value.FlatData<FJson::FArray>();
        if (not arr.empty()) {
            oss << Fmt::LBracket;
            if (not minify)
                oss << Eol;
            {
                const typename Fmt::TBasicIndent<_Char>::FScope scopeIndent(indent);

                size_t n = arr.size();
                for (const auto& item : arr) {
                    oss << indent;
                    ToStream_(item, oss, indent, minify);
                    if (--n)
                        oss << Fmt::Comma;
                    if (not minify)
                        oss << Eol;
                }
            }
            oss << indent << Fmt::RBracket;
        }
        else {
            oss << Fmt::LBracket << Fmt::RBracket;
        }
    }
        break;
    case Json_TypeId_Object:
    {
        const FJson::FObject& obj = value.FlatData<FJson::FObject>();
        if (not obj.empty()) {
            oss << Fmt::LBrace;
            if (not minify)
                oss << Eol;
            {
                const typename Fmt::TBasicIndent<_Char>::FScope scopeIndent(indent);

                size_t n = obj.size();
                for (const auto& member : obj) {
                    oss << indent << Fmt::DoubleQuote;
                    EscapeString_(oss, member.first);
                    oss << Fmt::DoubleQuote << Fmt::Colon << Fmt::Space;
                    ToStream_(member.second, oss, indent, minify);
                    if (--n)
                        oss << Fmt::Comma;
                    if (not minify)
                        oss << Eol;
                }
            }
            oss << indent << Fmt::RBrace;
        }
        else {
            oss << Fmt::LBrace << Fmt::RBrace;
        }
        break;
    }
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
static RTTI::PTypeTraits MakeTraits_(FJson::EType type) NOEXCEPT {
    switch (type) {
    case FJson::Null:     return RTTI::MakeTraits<FJson::FNull>();
    case FJson::Bool:     return RTTI::MakeTraits<FJson::FBool>();
    case FJson::Integer:  return RTTI::MakeTraits<FJson::FInteger>();
    case FJson::Float:    return RTTI::MakeTraits<FJson::FFloat>();
    case FJson::String:   return RTTI::MakeTraits<FJson::FText>();
    case FJson::Array:    return RTTI::MakeTraits<FJson::FArray>();
    case FJson::Object:   return RTTI::MakeTraits<FJson::FObject>();
    }
    AssertNotReached();
}
//----------------------------------------------------------------------------
} //!namespace Json_
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FJson::FValue::FValue(EType type) noexcept
:   RTTI::FAny(MakeTraits_(type))
{}
//----------------------------------------------------------------------------
void FJson::ToStream(FTextWriter& oss, bool minify/* = false */) const {
    Fmt::FIndent indent = (minify ? Fmt::FIndent::None() : Fmt::FIndent::TwoSpaces());
    oss << FTextFormat::DefaultFloat
        << FTextFormat::BoolAlpha;
    ToStream_(_root, oss, indent, minify);
    Assert(0 == indent.Level);
}
//----------------------------------------------------------------------------
void FJson::ToStream(FWTextWriter& oss, bool minify/* = false */) const {
    Fmt::FWIndent indent = (minify ? Fmt::FWIndent::None() : Fmt::FWIndent::TwoSpaces());
    oss << FTextFormat::DefaultFloat
        << FTextFormat::BoolAlpha;
    ToStream_(_root, oss, indent, minify);
    Assert(0 == indent.Level);
}
//----------------------------------------------------------------------------
bool FJson::Load(FJson* json, const FFilename& filename) {
    Assert(json);
    Assert(not filename.empty());

    EAccessPolicy policy = EAccessPolicy::Binary;
    if (filename.Extname() == FFSConstNames::Z())
        policy = policy + EAccessPolicy::Compress;

    RAWSTORAGE(FileSystem, u8) content;
    if (not VFS_ReadAll(&content, filename, policy))
        return false;

    return Load(json, filename.ToWString(), content.MakeConstView().Cast<const char>());
}
//----------------------------------------------------------------------------
bool FJson::Load(FJson* json, const FWStringView& filename, const FStringView& content) {
    FMemoryViewReader reader(content.Cast<const u8>());
    return Load(json, filename, &reader);
}
//----------------------------------------------------------------------------
bool FJson::Load(FJson* json, const FFilename& filename, IBufferedStreamReader* input) {
    STACKLOCAL_ASSUMEPOD_ARRAY(wchar_t, tmp, FileSystem::MaxPathLength);
    return Load(json, filename.ToWCStr(tmp), input);
}
//----------------------------------------------------------------------------
bool FJson::Load(FJson* json, const FWStringView& filename, IBufferedStreamReader* input) {
    Assert(json);
    Assert(input);

    Lexer::FLexer lexer(*input, filename, false);

    json->_root.Reset();

    PPE_TRY{
        if (not ParseValue_(lexer, *json, json->_root))
            return false;
    }
    PPE_CATCH(Lexer::FLexerException e)
    PPE_CATCH_BLOCK({
        PPE_THROW_IT(FJsonException(e.What(), e.Match().Site()));
    })

    return true;
}
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_DESCRIPTION
FWTextWriter& FJsonException::Description(FWTextWriter& oss) const {
    return oss
        << MakeCStringView(What())
        << L": in json file '"
        << _site
        << L"' !";
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
