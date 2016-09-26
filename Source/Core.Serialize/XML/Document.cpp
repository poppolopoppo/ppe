#include "stdafx.h"

#include "Document.h"

#include "Element.h"

#include "Lexer/LookAheadReader.h"

#include "Core/Container/RawStorage.h"
#include "Core/Container/Vector.h"
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
static void ExpectChar_(Lexer::LookAheadReader& reader, char expected) {
    Assert(0 != expected);

    reader.EatWhiteSpaces();

    if (expected != reader.Peek())
        CORE_THROW_IT(XMLException("unexpected token", reader.SourceSite()));

    reader.Read();
}
//----------------------------------------------------------------------------
static void ExpectToken_(Lexer::LookAheadReader& reader, const StringView& id) {
    Assert(id.size());

    reader.EatWhiteSpaces();

    bool succeed = true;
    forrange(i, 0, id.size()) {
        if (not EqualsI(reader.Read(), id[i]))
            CORE_THROW_IT(XMLException("unexpected idenitifier", reader.SourceSite()));
    }
}
//----------------------------------------------------------------------------
static bool ReadIdentifier_(Lexer::LookAheadReader& reader, String& id) {
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
static void ExpectIdentifier_(Lexer::LookAheadReader& reader, String& id) {
    const Lexer::Location site = reader.SourceSite();
    if (false == ReadIdentifier_(reader, id))
        CORE_THROW_IT(XMLException("expected an identitfer", site));
}
//----------------------------------------------------------------------------
static bool ReadString_(Lexer::LookAheadReader& reader, String& str) {
    Assert(str.empty());

    reader.EatWhiteSpaces();

    if (reader.Peek() != Token_::Quote)
        return false;

    const Lexer::Location site = reader.SourceSite();

    reader.SeekFwd(1);

    while (reader.Peek() != Token_::Quote)
        str += reader.Read();

    if (reader.Read() != Token_::Quote)
        CORE_THROW_IT(XMLException("unclosed string", site));

    return true;
}
//----------------------------------------------------------------------------
static void ExpectString_(Lexer::LookAheadReader& reader, String& str) {
    const Lexer::Location site = reader.SourceSite();
    if (false == ReadString_(reader, str))
        CORE_THROW_IT(XMLException("expected a string value", site));
}
//----------------------------------------------------------------------------
static char PeekChar_(Lexer::LookAheadReader& reader) {
    reader.EatWhiteSpaces();
    return reader.Peek();
}
//----------------------------------------------------------------------------
static void ReadHeader_(Lexer::LookAheadReader& reader, String& version, String& encoding, String& standalone) {
    ExpectChar_(reader, Token_::Less);
    ExpectChar_(reader, Token_::Question);
    ExpectToken_(reader, "xml");

    String eaten;
    while (ReadIdentifier_(reader, eaten) ) {
        String* pValue = nullptr;
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
            CORE_THROW_IT(XMLException("invalid document attribute", reader.SourceSite()));
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
Document::Document() {}
//----------------------------------------------------------------------------
Document::~Document() {}
//----------------------------------------------------------------------------
const Element* Document::FindById(const StringView& Id) const {
    Assert(!Id.empty());

    // some exporter append a leading '#'
    const StringView query = ('#' == Id.front() ? Id.ShiftFront() : Id);

    SElement elt;
    return (TryGetValue(_byIdentifier, query, &elt) ? elt.get() : nullptr);
}
//----------------------------------------------------------------------------
const Element* Document::XPath(const MemoryView<const Name>& path) const {
    return (_root ? _root->XPath(path) : nullptr );
}
//----------------------------------------------------------------------------
size_t Document::XPath(const MemoryView<const Name>& path, const std::function<void(const Element&)>& functor) const {
    return (_root ? _root->XPath(path, functor) : 0 );
}
//----------------------------------------------------------------------------
bool Document::Load(Document* document, const Filename& filename) {
    Assert(document);
    Assert(not filename.empty());

    RAWSTORAGE_THREAD_LOCAL(FileSystem, u8) content;
    if (not VFS_ReadAll(&content, filename, AccessPolicy::Binary))
        return false;

    return Load(document, filename, content.MakeConstView().Cast<const char>() );
}
//----------------------------------------------------------------------------
bool Document::Load(Document* document, const Filename& filename, IStreamReader* input) {
    Assert(document);
    Assert(input);

    document->_root.reset();
    document->_version.clear();
    document->_encoding.clear();
    document->_standalone.clear();
    document->_byIdentifier.clear();

    const WString filenameStr(filename.ToWString());
    Lexer::LookAheadReader reader(input, filenameStr.c_str());
    ReadHeader_(reader, document->_version, document->_encoding, document->_standalone);

    if (document->_version.empty())
        document->_version = "1.0";
    if (document->_encoding.empty())
        document->_encoding = "utf-8";
    if (document->_standalone.empty())
        document->_standalone = "yes";

    const XML::Name keyId("id");

    struct ReadElement_ {
        PElement Element;
        Lexer::Location Site;

        void RegisterIFN(const XML::Name& key, byidentifier_type& ids) {
            const auto elementId = Element->Attributes().Find(key);
            if (Element->Attributes().end() != elementId) {
                if (Insert_ReturnIfExists(  ids,
                                            MakeStringView(elementId->second),
                                            SElement(Element.get())) ) {
                    CORE_THROW_IT(XMLException("an element with the same id alread exists", Site));
                }
            }
        }

        ReadElement_() {}
        ReadElement_(const String& start, const Lexer::Location& site)
            : Element(new XML::Element())
            , Site(site) {
            Element->SetType(XML::Name(start.MakeView()) );
        }
    };

    VECTORINSITU_THREAD_LOCAL(XML, ReadElement_, 32) visited;
    visited.reserve(32);

    String eaten;
    while (char poken = PeekChar_(reader)) {
        if (poken != Token_::Less) {
            // XML inner text
            const Lexer::Location site = reader.SourceSite();
            if (visited.empty())
                CORE_THROW_IT(XMLException("text without tag", site));

            if (not reader.ReadUntil(eaten, '<'))
                CORE_THROW_IT(XMLException("unterminated text", site));

            visited.back().Element->SetText(std::move(eaten));
        }
        else {
            ExpectChar_(reader, Token_::Less);

            // XML comment
            if (PeekChar_(reader) == Token_::Not) {
                ExpectChar_(reader, Token_::Not);
                ExpectChar_(reader, Token_::Minus);
                ExpectChar_(reader, Token_::Minus);

                const Lexer::Location site = reader.SourceSite();

                if (not reader.SkipUntil(Token_::Minus))
                    CORE_THROW_IT(XMLException("unterminated comment", site));

                ExpectChar_(reader, Token_::Minus);
                ExpectChar_(reader, Token_::Minus);
                ExpectChar_(reader, Token_::Greater);

                continue;
            }

            // XML close tag ?
            if (PeekChar_(reader) == Token_::Div) {
                ExpectChar_(reader, Token_::Div);

                if (visited.empty())
                    CORE_THROW_IT(XMLException("no opened tag", reader.SourceSite()));

                ExpectIdentifier_(reader, eaten);

                if (not EqualsI(visited.back().Element->Type().MakeView(), eaten.MakeView()) )
                    CORE_THROW_IT(XMLException("mismatching closing tag", reader.SourceSite()) );

                eaten.clear();

                visited.back().RegisterIFN(keyId, document->_byIdentifier);
                visited.pop_back();

                ExpectChar_(reader, Token_::Greater);

                continue;
            }

            ExpectIdentifier_(reader, eaten);

            ReadElement_ it(eaten, reader.SourceSite());
            eaten.clear();

            // Attach new element to parent and siblings IFP
            if (visited.size()) {
                Element* const parent = visited.back().Element.get();
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
                const XML::Name key(eaten.MakeView());
                eaten.clear();

                bool added = false;
                auto value = it.Element->Attributes().FindOrAdd(key, &added);
                if (not added)
                    CORE_THROW_IT(XMLException("redefining block attribute", reader.SourceSite()) );

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
                CORE_THROW_IT(XMLException("unterminated xml tag", reader.SourceSite()) );
            }
        }
    }

    if (visited.size())
        CORE_THROW_IT(XMLException("unterminated xml block", visited.back().Site));

    return true;
}
//----------------------------------------------------------------------------
bool Document::Load(Document* document, const Filename& filename, const StringView& content) {
    MemoryViewReader reader(content.Cast<const u8>());
    return Load(document, filename, &reader);
}
//----------------------------------------------------------------------------
void Document::ToStream(std::basic_ostream<char>& oss) const {
    oss << "<?xml version=\"" << _version
        << "\" encoding=\"" << _encoding
        << "\" standalone=\"" << _standalone
        << "\" ?>" << std::endl;

    if (_root)
        _root->ToStream(oss);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace XML
} //!namespace Core
