#include "stdafx.h"

#include "Document.h"

#include "Element.h"

#include "Lexer/LookAheadReader.h"

#include "Core/Container/RawStorage.h"
#include "Core/Container/Vector.h"
#include "Core/IO/FS/ConstNames.h"
#include "Core/IO/FS/Filename.h"
#include "Core/IO/TextWriter.h"
#include "Core/IO/VirtualFileSystem.h"
#include "Core/Memory/MemoryProvider.h"

namespace Core {
namespace XML {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace  {
//----------------------------------------------------------------------------
namespace Token_ {
STATIC_CONST_INTEGRAL(char, Assignment, '=');
STATIC_CONST_INTEGRAL(char, Div,        '/');
STATIC_CONST_INTEGRAL(char, Greater,    '>');
STATIC_CONST_INTEGRAL(char, Less,       '<');
STATIC_CONST_INTEGRAL(char, Minus,      '-');
STATIC_CONST_INTEGRAL(char, Not,        '!');
STATIC_CONST_INTEGRAL(char, Question,   '?');
STATIC_CONST_INTEGRAL(char, Quote,      '"');
} //!Token_
//----------------------------------------------------------------------------
static bool IsIdentifierCharset_(char ch) {
    return (IsAlnum(ch) || '_' == ch);
}
//----------------------------------------------------------------------------
static void ExpectChar_(Lexer::FLookAheadReader& reader, char expected) {
    Assert(0 != expected);

    reader.EatWhiteSpaces();

    if (expected != reader.Peek())
        CORE_THROW_IT(FXMLException("unexpected token", reader.SourceSite()));

    reader.Read();
}
//----------------------------------------------------------------------------
static void ExpectToken_(Lexer::FLookAheadReader& reader, const FStringView& id) {
    Assert(id.size());

    reader.EatWhiteSpaces();

    forrange(i, 0, id.size()) {
        if (not EqualsI(reader.Read(), id[i]))
            CORE_THROW_IT(FXMLException("unexpected idenitifier", reader.SourceSite()));
    }
}
//----------------------------------------------------------------------------
static bool ReadIdentifier_(Lexer::FLookAheadReader& reader, FString& id) {
    Assert(id.empty());

    reader.EatWhiteSpaces();

    if (not IsAlpha(reader.Peek()) )
        return false;

    id += reader.Read();
    while (IsIdentifierCharset_(reader.Peek()) )
        id += reader.Read();

    return true;
}
//----------------------------------------------------------------------------
static void ExpectIdentifier_(Lexer::FLookAheadReader& reader, FString& id) {
    const Lexer::FLocation site = reader.SourceSite();
    if (false == ReadIdentifier_(reader, id))
        CORE_THROW_IT(FXMLException("expected an identitfer", site));
}
//----------------------------------------------------------------------------
static bool ReadString_(Lexer::FLookAheadReader& reader, FString& str) {
    Assert(str.empty());

    reader.EatWhiteSpaces();

    if (reader.Peek() != Token_::Quote)
        return false;

    const Lexer::FLocation site = reader.SourceSite();

    reader.SeekFwd(1);

    while (reader.Peek() != Token_::Quote)
        str += reader.Read();

    if (reader.Read() != Token_::Quote)
        CORE_THROW_IT(FXMLException("unclosed string", site));

    return true;
}
//----------------------------------------------------------------------------
static void ExpectString_(Lexer::FLookAheadReader& reader, FString& str) {
    const Lexer::FLocation site = reader.SourceSite();
    if (false == ReadString_(reader, str))
        CORE_THROW_IT(FXMLException("expected a string value", site));
}
//----------------------------------------------------------------------------
static char PeekChar_(Lexer::FLookAheadReader& reader) {
    reader.EatWhiteSpaces();
    return reader.Peek();
}
//----------------------------------------------------------------------------
static void ReadHeader_(Lexer::FLookAheadReader& reader, FString& version, FString& encoding, FString& standalone) {
    ExpectChar_(reader, Token_::Less);
    ExpectChar_(reader, Token_::Question);
    ExpectToken_(reader, "xml");

    FString eaten;
    while (ReadIdentifier_(reader, eaten) ) {
        FString* pValue = nullptr;
        if (EqualsI("version", eaten.MakeView())) {
            pValue = &version;
        }
        else if (EqualsI("encoding", eaten.MakeView())) {
            pValue = &encoding;
        }
        else if (EqualsI("standalone", eaten.MakeView())) {
            pValue = &standalone;
        }
        else {
            CORE_THROW_IT(FXMLException("invalid document attribute", reader.SourceSite()));
        }

        Assert(pValue);

        ExpectChar_(reader, Token_::Assignment);

        eaten.clear();

        ExpectString_(reader, eaten);

        *pValue = std::move(eaten);
    }

    ExpectChar_(reader, Token_::Question);
    ExpectChar_(reader, Token_::Greater);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDocument::FDocument() {}
//----------------------------------------------------------------------------
FDocument::~FDocument() {}
//----------------------------------------------------------------------------
const FElement* FDocument::FindById(const FStringView& Id) const {
    Assert(!Id.empty());

    // some exporter append a leading '#'
    const FStringView query = ('#' == Id.front() ? FStringView(Id.ShiftFront()) : Id);

    SElement elt;
    return (TryGetValue(_byIdentifier, query, &elt) ? elt.get() : nullptr);
}
//----------------------------------------------------------------------------
const FElement* FDocument::XPath(const TMemoryView<const FName>& path) const {
    return (_root ? _root->XPath(path) : nullptr );
}
//----------------------------------------------------------------------------
size_t FDocument::XPath(const TMemoryView<const FName>& path, const Meta::TFunction<void(const FElement&)>& functor) const {
    return (_root ? _root->XPath(path, functor) : 0 );
}
//----------------------------------------------------------------------------
bool FDocument::Load(FDocument* document, const FFilename& filename) {
    Assert(document);
    Assert(not filename.empty());

    EAccessPolicy policy = EAccessPolicy::Binary;
    if (filename.Extname() == FFSConstNames::Xmlz())
        policy = policy + EAccessPolicy::Compress;

    RAWSTORAGE_THREAD_LOCAL(FileSystem, u8) content;
    if (not VFS_ReadAll(&content, filename, policy))
        return false;

    return Load(document, filename, content.MakeConstView().Cast<const char>() );
}
//----------------------------------------------------------------------------
bool FDocument::Load(FDocument* document, const FFilename& filename, IBufferedStreamReader* input) {
    Assert(document);
    Assert(input);

    document->_root.reset();
    document->_version.clear();
    document->_encoding.clear();
    document->_standalone.clear();
    document->_byIdentifier.clear();

    const FWString filenameStr(filename.ToWString());
    Lexer::FLookAheadReader reader(input, filenameStr.c_str());

    if (reader.Peek(0) == '<' && reader.Peek(1) == '?')
        ReadHeader_(reader, document->_version, document->_encoding, document->_standalone);

    if (document->_version.empty())
        document->_version = "1.0";
    if (document->_encoding.empty())
        document->_encoding = "utf-8";
    if (document->_standalone.empty())
        document->_standalone = "yes";

    const XML::FName keyId("id");

    struct FReadElement_ {
        PElement Element;
        Lexer::FLocation Site;

        void RegisterIFN(const XML::FName& key, byidentifier_type& ids) {
            const auto elementId = Element->Attributes().Find(key);
            if (Element->Attributes().end() != elementId) {
                if (Insert_ReturnIfExists(  ids,
                                            MakeStringView(elementId->second),
                                            SElement(Element.get())) ) {
                    CORE_THROW_IT(FXMLException("an element with the same id alread exists", Site));
                }
            }
        }

        FReadElement_() {}
        FReadElement_(const FString& start, const Lexer::FLocation& site)
            : Element(new XML::FElement())
            , Site(site) {
            Element->SetType(XML::FName(start.MakeView()) );
        }
    };

    VECTORINSITU_THREAD_LOCAL(XML, FReadElement_, 32) visited;
    visited.reserve(32);

    FString eaten;
    while (char poken = PeekChar_(reader)) {
        if (poken != Token_::Less) {
            // XML inner text
            const Lexer::FLocation site = reader.SourceSite();
            if (visited.empty())
                CORE_THROW_IT(FXMLException("text without tag", site));

            if (not reader.ReadUntil(eaten, '<'))
                CORE_THROW_IT(FXMLException("unterminated text", site));

            visited.back().Element->SetText(std::move(eaten));
        }
        else {
            ExpectChar_(reader, Token_::Less);

            // XML comment
            if (PeekChar_(reader) == Token_::Not) {
                ExpectChar_(reader, Token_::Not);
                ExpectChar_(reader, Token_::Minus);
                ExpectChar_(reader, Token_::Minus);

                const Lexer::FLocation site = reader.SourceSite();

                if (not reader.SkipUntil(Token_::Minus))
                    CORE_THROW_IT(FXMLException("unterminated comment", site));

                ExpectChar_(reader, Token_::Minus);
                ExpectChar_(reader, Token_::Minus);
                ExpectChar_(reader, Token_::Greater);

                continue;
            }

            // XML close tag ?
            if (PeekChar_(reader) == Token_::Div) {
                ExpectChar_(reader, Token_::Div);

                if (visited.empty())
                    CORE_THROW_IT(FXMLException("no opened tag", reader.SourceSite()));

                ExpectIdentifier_(reader, eaten);

                if (not EqualsI(visited.back().Element->Type().MakeView(), eaten.MakeView()) )
                    CORE_THROW_IT(FXMLException("mismatching closing tag", reader.SourceSite()) );

                eaten.clear();

                visited.back().RegisterIFN(keyId, document->_byIdentifier);
                visited.pop_back();

                ExpectChar_(reader, Token_::Greater);

                continue;
            }

            ExpectIdentifier_(reader, eaten);

            FReadElement_ it(eaten, reader.SourceSite());
            eaten.clear();

            // Attach new element to parent and siblings IFP
            if (visited.size()) {
                FElement* const parent = visited.back().Element.get();
                it.Element->SetParent(parent);

                auto& children = parent->Children();
                if (children.size()) {
                    children.back()->SetNextSibling(it.Element.get());
                    it.Element->SetPrevSibling(children.back().get());
                }

                children.push_back(it.Element);
            }
            else {
                Assert(nullptr == document->Root());
                document->_root = it.Element;
            }

            // XML attributes
            while (ReadIdentifier_(reader, eaten)) {
                const XML::FName key(eaten.MakeView());
                eaten.clear();

                bool added = false;
                auto value = it.Element->Attributes().FindOrAdd(key, &added);
                if (not added)
                    CORE_THROW_IT(FXMLException("redefining block attribute", reader.SourceSite()) );

                ExpectChar_(reader, Token_::Assignment);
                ExpectString_(reader, eaten);

                value->second = std::move(eaten);
            }

            poken = PeekChar_(reader);

            if (Token_::Div == poken) {
                // XML short tag
                ExpectChar_(reader, Token_::Div);
                ExpectChar_(reader, Token_::Greater);

                it.RegisterIFN(keyId, document->_byIdentifier);

                RemoveRef_AssertGreaterThanZero(it.Element);
            }
            else if (Token_::Greater == poken) {
                // XML unclosed tag
                ExpectChar_(reader, Token_::Greater);
                visited.emplace_back(std::move(it));
            }
            else {
                CORE_THROW_IT(FXMLException("unterminated xml tag", reader.SourceSite()) );
            }
        }
    }

    if (visited.size())
        CORE_THROW_IT(FXMLException("unterminated xml block", visited.back().Site));

    return true;
}
//----------------------------------------------------------------------------
bool FDocument::Load(FDocument* document, const FFilename& filename, const FStringView& content) {
    FMemoryViewReader reader(content.Cast<const u8>());
    return Load(document, filename, &reader);
}
//----------------------------------------------------------------------------
void FDocument::ToStream(FTextWriter& oss) const {
    oss << "<?xml version=\"" << _version
        << "\" encoding=\"" << _encoding
        << "\" standalone=\"" << _standalone
        << "\" ?>" << Eol;

    if (_root)
        _root->ToStream(oss);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace XML
} //!namespace Core
