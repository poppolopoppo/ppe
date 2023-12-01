// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Json/Json.h"

#include "Lexer/Lexer.h"
#include "Lexer/Match.h"
#include "Lexer/Symbols.h"

#include "MetaObject.h"

#include "Container/AssociativeVector.h"
#include "Container/RawStorage.h"
#include "Container/Vector.h"
#include "IO/ConstNames.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/TextWriter.h"
#include "Memory/MemoryProvider.h"
#include "VirtualFileSystem.h"
#include "IO/Filename.h"
#include "Meta/Functor.h"
#include "RTTI/NativeTraits.h"

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void EscapeJson_(FTextWriter& oss, const FJson::FText& str) {
    Escape(oss, str.MakeView(), EEscape::Unicode);
}
//----------------------------------------------------------------------------
static void EscapeJson_(FWTextWriter& oss, const FJson::FText& str) {
    Escape(oss, ToWCStr(INLINE_MALLOCA(wchar_t, str.size() + 1), str), EEscape::Unicode);
}
//----------------------------------------------------------------------------
static bool ParseValue_(Lexer::FLexer& lexer, FJson& doc, FJson::FValue& value);
//----------------------------------------------------------------------------
static bool ParseObject_(Lexer::FLexer& lexer, FJson& doc, FJson::FValue& value) {
    FJson::FObject& object = value.Construct<FJson::FObject>(doc.Heap());

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

        const FJson::FText name = doc.MakeText(key.Value());
        FJson::FValue& v = object.Add(name);
        if (not ParseValue_(lexer, doc, v))
            PPE_THROW_IT(FJsonException("missing value", key.Site()));
    }

    return lexer.Expect(Lexer::FSymbols::RBrace);
}
//----------------------------------------------------------------------------
static bool ParseArray_(Lexer::FLexer& lexer, FJson& doc, FJson::FValue& value) {
    FJson::FArray& arr = value.Construct<FJson::FArray>(doc.Heap());

    if (lexer.ReadIFN(Lexer::FSymbols::RBracket)) // quick reject for empty array
        return true;

    for (bool notFirst = false;; notFirst = true) {
        if (notFirst && not lexer.ReadIFN(Lexer::FSymbols::Comma))
            break;

        if (not ParseValue_(lexer, doc, *Emplace_Back(arr)) )
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
        value.Assign(doc.MakeText(read.Value(), false));
    }
    else if (read.Symbol() == Lexer::FSymbols::Integer) {
        FJson::FInteger& num = value.Construct<FJson::FInteger>();
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
        FJson::FFloat& fp = value.Construct<FJson::FFloat>();
        if (not Atod(&fp, read.Value().MakeView()))
            PPE_THROW_IT(FJsonException("malformed float", read.Site()));
    }
    // unary minus ?
    else if (read.Symbol() == Lexer::FSymbols::Sub) {
        const Lexer::FMatch* const peek = lexer.Peek();

        unexpectedToken = true;

        if (peek && peek->Symbol() == Lexer::FSymbols::Integer) {
            if (ParseValue_(lexer, doc, value)) {
                FJson::FInteger& num = value.Get<FJson::FInteger>();
                num = -num;
                unexpectedToken = false;
            }
        }
        else if (peek && peek->Symbol() == Lexer::FSymbols::Unsigned) {
            AssertNotReached(); // -unsigned isn't supported !
        }
        else if (peek && peek->Symbol() == Lexer::FSymbols::Float) {
            if (ParseValue_(lexer, doc, value)) {
                FJson::FFloat& dbl = value.Get<FJson::FFloat>();
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
template <typename _Char>
static void ToStream_(const FJson::FValue& value, TBasicTextWriter<_Char>& oss, Fmt::TBasicIndent<_Char>& indent, bool minify) {
    Meta::Visit(value.Data,
        [](const std::monostate&) {
            AssertNotReached();
        },
        [&oss](const FJson::FNull&) {
            oss << "null";
        },
        [&oss](const auto& value) {
            oss << value;
        },
        [&oss](const FJson::FText& value) {
            oss << Fmt::DoubleQuote;
            EscapeJson_(oss, value);
            oss << Fmt::DoubleQuote;
        },
        [&oss, &indent, minify](const FJson::FArray& value) {
            if (not value.empty()) {
                oss << Fmt::LBracket;
                if (not minify) oss << Eol;
                {
                    const typename Fmt::TBasicIndent<_Char>::FScope scopeIndent(indent);

                    size_t n = value.size();
                    for (const auto& item : value) {
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
        },
        [&oss, &indent, minify](const FJson::FObject& value) {
            if (not value.empty()) {
                oss << Fmt::LBrace;
                if (not minify) oss << Eol;
                {
                    const typename Fmt::TBasicIndent<_Char>::FScope scopeIndent(indent);

                    size_t n = value.size();
                    for (const auto& member : value) {
                        oss << indent << Fmt::DoubleQuote;
                        EscapeJson_(oss, member.first);
                        oss << Fmt::DoubleQuote << Fmt::Colon;
                        if (not minify)
                            oss << Fmt::Space;
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
        });
}
//----------------------------------------------------------------------------
static bool ParseJson_(FJson::FValue* dst, FJson& doc, const FWStringView& filename, IBufferedStreamReader& src) {
    Assert(dst);
    Lexer::FLexer lexer(src, filename, false);

    PPE_TRY{
        if (not ParseValue_(lexer, doc, *dst))
            return false;
    }
    PPE_CATCH(Lexer::FLexerException e)
    PPE_CATCH_BLOCK({
        PPE_THROW_IT(FJsonException(e.What(), e.Match().Site()));
    })

    return true;
}
//----------------------------------------------------------------------------
} //!namespace Json_
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// FJson::FValue
//----------------------------------------------------------------------------
void FJson::FValue::Construct(FJson& doc, EType type) {
    switch (type) {
    case Null: Construct<FNull>(); break;
    case Bool: Construct<FBool>(); break;
    case Integer: Construct<FInteger>(); break;
    case Float: Construct<FFloat>(); break;
    case String: Construct<FText>(); break;
    case Array: Construct<FArray>(doc.Heap()); break;
    case Object: Construct<FObject>(doc.Heap()); break;
    }
}
//----------------------------------------------------------------------------
hash_t FJson::FValue::HashValue() const NOEXCEPT {
    return Meta::Visit(Data,
        [](const auto& x) -> hash_t {
            return hash_value(x);
        },
        [](const std::monostate&) -> hash_t {
            AssertNotReached();
        });
}
//----------------------------------------------------------------------------
void FJson::FValue::Reset() NOEXCEPT {
    Data = std::monostate{};
}
//----------------------------------------------------------------------------
bool FJson::FValue::Equals(const FValue& other) const NOEXCEPT {
    return (Data == other.Data);
}
//----------------------------------------------------------------------------
// FJson
//----------------------------------------------------------------------------
FJson::~FJson()  {
    Clear_ReleaseMemory();
}
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
bool FJson::Load(FJson* json, const FFilename& filename, IBufferedStreamReader* input) {
    STACKLOCAL_ASSUMEPOD_ARRAY(wchar_t, tmp, FileSystem::MaxPathLength);
    return Load(json, filename.ToWCStr(tmp), input);
}
//----------------------------------------------------------------------------
bool FJson::Load(FJson* json, const FWStringView& filename, const FStringView& content) {
    FMemoryViewReader reader(content.Cast<const u8>());
    return Load(json, filename, &reader);
}
//----------------------------------------------------------------------------
bool FJson::Load(FJson* json, const FWStringView& filename, IBufferedStreamReader* input) {
    Assert(json);
    Assert(input);

    json->_root.Reset();
    return ParseJson_(&json->_root, *json, filename, *input);
}
//----------------------------------------------------------------------------
bool FJson::Append(FJson* json, const FWStringView& filename, const FStringView& content) {
    FMemoryViewReader reader(content.Cast<const u8>());
    return Append(json, filename, &reader);
}
//----------------------------------------------------------------------------
bool FJson::Append(FJson* json, const FWStringView& filename, IBufferedStreamReader* input) {
    Assert(json);
    Assert(input);

    TPtrRef<FJson::FValue> dst;
    if (json->_root.Valid()) {
        FJson::FArray* arrIFP{ json->_root.AsArray() };
        if (not arrIFP) {
            FJson::FArray newRoot{ json->Heap() };
            newRoot.Emplace(std::move(json->_root));
            json->_root = std::move(newRoot);
            arrIFP = &json->_root.ToArray();
        }
        Assert(arrIFP);
        dst = *Emplace_Back(*arrIFP);
    }
    else {
        dst = json->_root;
    }

    return ParseJson_(dst, *json, filename, *input);
}
//----------------------------------------------------------------------------
/*static*/ const FJson::FText FJson::Id{ LiteralText("$id") };
/*static*/ const FJson::FText FJson::Ref{ LiteralText("$ref") };
/*static*/ const FJson::FText FJson::Class{ LiteralText("_class") };
/*static*/ const FJson::FText FJson::Export{ LiteralText("_export") };
/*static*/ const FJson::FText FJson::Inner{ LiteralText("_inner") };
/*static*/ const FJson::FText FJson::TopObject{ LiteralText("_topObject") };
/*static*/ const FJson::FText FJson::TypeId{ LiteralText("_typeId") };
//----------------------------------------------------------------------------
bool FJson::IsReservedKeyword(const FJson::FText& str) NOEXCEPT {
    return (
        (str == Id) ||
        (str == Class) ||
        (str == Export) ||
        (str == Inner) ||
        (str == Ref) ||
        (str == TopObject) ||
        (str == TypeId) );
}
//----------------------------------------------------------------------------
void FJson::Clear_ForgetMemory() {
    // this method will leak *all* memory: you must clear the heap afterward !
    INPLACE_NEW(&_root, FValue); // forget holded value without calling dtor
}
//----------------------------------------------------------------------------
void FJson::Clear_ReleaseMemory() {
    _root.Reset();
}
//----------------------------------------------------------------------------
FJson::FAllocator::~FAllocator() {
    // can't track all blocks need to clear all
    Text.Clear_ForgetMemory();
    Heap.DiscardAll();
}
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_DESCRIPTION
FTextWriter& FJsonException::Description(FTextWriter& oss) const {
    return oss
        << MakeCStringView(What())
        << ": in json file '"
        << _site
        << "' !";
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
