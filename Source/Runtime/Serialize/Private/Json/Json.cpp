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
#include "Meta/Utility.h"

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
struct FJsonParser_ {
    Lexer::FLexer Lexer;
    FJson::FBuilder Builder;

    bool ParseValue();

    bool ParseObject() {
        Builder.BeginObject();
        DEFERRED{ Builder.EndObject(); };

        if (Lexer.ReadIFN(Lexer::FSymbols::RBrace)) // quick reject for empty object
            return true;

        Lexer::FMatch key;
        for (bool notFirst = false;; notFirst = true) {
            if (notFirst && not Lexer.ReadIFN(Lexer::FSymbols::Comma))
                break;

            if (not Lexer.Expect(key, Lexer::FSymbols::String))
                break;

            if (not Lexer.Expect(Lexer::FSymbols::Colon))
                PPE_THROW_IT(FJsonException("missing comma", key.Site()));

            Builder.KeyValue(key.Value(), [&]() {
                if (not ParseValue())
                    PPE_THROW_IT(FJsonException("missing value", key.Site()));
            });
        }

        return Lexer.Expect(Lexer::FSymbols::RBrace);
    }

    bool ParseArray() {
        Builder.BeginArray();
        DEFERRED{ Builder.EndArray(); };

        if (Lexer.ReadIFN(Lexer::FSymbols::RBracket)) // quick reject for empty array
            return true;

        for (bool notFirst = false;; notFirst = true) {
            if (notFirst && not Lexer.ReadIFN(Lexer::FSymbols::Comma))
                break;

            if (not ParseValue())
                return false;
        }

        return Lexer.Expect(Lexer::FSymbols::RBracket);
    }
};
//----------------------------------------------------------------------------
bool FJsonParser_::ParseValue() {
    Lexer::FMatch read;
    if (not Lexer.Read(read))
        return false;

    bool unexpectedToken = false;

    if (read.Symbol() == Lexer::FSymbols::LBrace) {
        return ParseObject();
    }
    else if (read.Symbol() == Lexer::FSymbols::LBracket) {
        return ParseArray();
    }
    else if (read.Symbol() == Lexer::FSymbols::String) {
        Builder.Write(read.Value().MakeView());
    }
    else if (read.Symbol() == Lexer::FSymbols::Integer) {
        FJson::FInteger num;
        if (not Atoi(&num, read.Value().MakeView(), 10))
            PPE_THROW_IT(FJsonException("malformed int", read.Site()));

        Builder.Write(num);
    }
    else if (read.Symbol() == Lexer::FSymbols::Unsigned) {
        u64 u;
        if (not Atoi(&u, read.Value().MakeView(), 10))
            PPE_THROW_IT(FJsonException("malformed int", read.Site()));

        Builder.Write(u);
    }
    else if (read.Symbol() == Lexer::FSymbols::Float) {
        FJson::FFloat fp;
        if (not Atod(&fp, read.Value().MakeView()))
            PPE_THROW_IT(FJsonException("malformed float", read.Site()));

        Builder.Write(fp);
    }
    // unary minus ?
    else if (read.Symbol() == Lexer::FSymbols::Sub) {
        const Lexer::FMatch* const peek = Lexer.Peek();

        unexpectedToken = true;

        if (peek && peek->Symbol() == Lexer::FSymbols::Integer) {
            if (ParseValue()) {
                FJson::FInteger& num = std::get<FJson::FInteger>(*Builder.Peek());
                num = -num;
                unexpectedToken = false;
            }
        }
        else if (peek && peek->Symbol() == Lexer::FSymbols::Unsigned) {
            AssertNotReached(); // -unsigned isn't supported !
        }
        else if (peek && peek->Symbol() == Lexer::FSymbols::Float) {
            if (ParseValue()) {
                FJson::FFloat& dbl = std::get<FJson::FFloat>(*Builder.Peek());
                dbl = -dbl;
                unexpectedToken = false;
            }
        }
    }
    else if (read.Symbol() == Lexer::FSymbols::True) {
        Builder.Write(true);
    }
    else if (read.Symbol() == Lexer::FSymbols::False) {
        Builder.Write(false);
    }
    else if (read.Symbol() == Lexer::FSymbols::Null) {
        Builder.Write(FJson::FNull{});
    }
    else {
        unexpectedToken = true;
    }

    if (unexpectedToken)
        PPE_THROW_IT(FJsonException("unexpected token", read.Site()));

    return true;
}
//----------------------------------------------------------------------------
static bool ParseJson_(FJson::FValue* dst, FJson& doc, const FWStringView& filename, IBufferedStreamReader& src) {
    Assert(dst);
    FJsonParser_ parser{
        Lexer::FLexer(src, filename, false),
        FJson::FBuilder(dst, doc.Heap()),
    };

    parser.Builder.SetTextMemoization(doc.AnsiText());
    parser.Builder.SetTextMemoization(doc.WideText());

    PPE_TRY{
        if (not parser.ParseValue())
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
FJson::~FJson()  {
    Clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
void FJson::ToStream(FTextWriter& oss, bool minify/* = false */) const {
    const FTextWriter::FFormatScope format(oss);
    oss << FTextFormat::Escape;
    if (minify)
        oss << FTextFormat::Compact;
    oss << _root;
}
//----------------------------------------------------------------------------
void FJson::ToStream(FWTextWriter& oss, bool minify/* = false */) const {
    const FWTextWriter::FFormatScope format(oss);
    oss << FTextFormat::Escape;
    if (minify)
        oss << FTextFormat::Compact;
    oss << _root;
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

    TPtrRef<FValue> dst;
    if (json->_root.Nil()) {
        dst = json->_root;
    }
    else if (FArray* pArr = std::get_if<FArray>(&json->_root)) {
        dst = pArr->push_back_Default();
    }
    else {
        FArray newRoot{ json->Heap() };
        newRoot.push_back(std::move(json->_root));

        json->_root = std::move(newRoot);
        dst = std::get<FArray>(json->_root).push_back_Default();
    }

    return ParseJson_(dst, *json, filename, *input);
}
//----------------------------------------------------------------------------
/*static*/ const FJson::FText FJson::Id{ Literal_Id };
/*static*/ const FJson::FText FJson::Ref{ Literal_Ref };
/*static*/ const FJson::FText FJson::Class{ Literal_Class };
/*static*/ const FJson::FText FJson::Export{ Literal_Export };
/*static*/ const FJson::FText FJson::Inner{ Literal_Inner };
/*static*/ const FJson::FText FJson::TopObject{ Literal_TopObject };
/*static*/ const FJson::FText FJson::TypeId{ Literal_TypeId };
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
    AnsiText.clear_ReleaseMemory();
    WideText.clear_ReleaseMemory();
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
