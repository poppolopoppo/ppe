#include "stdafx.h"

#include "Document.h"

#include "Element.h"

#include "Lexer/Lexer.h"
#include "Lexer/Match.h"
#include "Lexer/Symbol.h"
#include "Lexer/Symbols.h"

#include "Core/Container/RawStorage.h"
#include "Core/Container/Vector.h"

namespace Core {
namespace XML {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace  {
//----------------------------------------------------------------------------
static void Expect_(Lexer::Lexer& lexer, Lexer::Match& eaten, const Lexer::Symbol* symbol) {
    if (not lexer.Expect(eaten, symbol))
        CORE_THROW_IT(XMLException("unexpected token", eaten.Site()));
}
//----------------------------------------------------------------------------
static void ReadHeader_(Lexer::Lexer& lexer, String& version, String& encoding, String& standalone) {
    Lexer::Match eaten;
    Expect_(lexer, eaten, Lexer::Symbols::Less);
    Expect_(lexer, eaten, Lexer::Symbols::Question);
    Expect_(lexer, eaten, Lexer::Symbols::Identifier);

    if (not EqualsI("xml", eaten.MakeView()))
        CORE_THROW_IT(XMLException("invalid document type", eaten.Site()));

    const Lexer::Match* poken;
    while ((poken = lexer.Peek(Lexer::Symbols::Identifier)) ) {
        Expect_(lexer, eaten, Lexer::Symbols::Identifier);

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
            CORE_THROW_IT(XMLException("invalid document attribute", eaten.Site()));
        }

        Assert(pValue);

        Expect_(lexer, eaten, Lexer::Symbols::Assignment);
        Expect_(lexer, eaten, Lexer::Symbols::String);

        *pValue = std::move(eaten.Value());
    }

    Expect_(lexer, eaten, Lexer::Symbols::Question);
    Expect_(lexer, eaten, Lexer::Symbols::Greater);
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
bool Document::Load(Document* document, const Filename& filename, const StringView& content) {
    Assert(document);

    document->_root.reset();
    document->_version.clear();
    document->_encoding.clear();
    document->_standalone.clear();
    document->_byIdentifier.clear();

    Lexer::Lexer lexer(content, MakeView(filename.ToWString()), false);
    ReadHeader_(lexer, document->_version, document->_encoding, document->_standalone);

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
        ReadElement_(const Lexer::Match& start)
            : Element(new XML::Element())
            , Site(start.Site()) {
            Assert(start.Symbol() == Lexer::Symbols::Identifier);
            Element->SetType(start.MakeView());
        }
    };

    VECTORINSITU_THREAD_LOCAL(XML, ReadElement_, 32) visited;
    visited.reserve(32);

    Lexer::Match eaten;
    const Lexer::Match* poken;
    while ((poken = lexer.Peek()) && poken->Symbol() != Lexer::Symbols::Eof) {
        if (poken->Symbol() != Lexer::Symbols::Less) {
            // XML inner text
            const Lexer::Location site = poken->Site();
            if (visited.empty())
                CORE_THROW_IT(XMLException("text without tag", site));

            lexer.Seek(poken->Offset());
            if (not lexer.ReadUntil(eaten, '<'))
                CORE_THROW_IT(XMLException("unterminated text", site));

            visited.back().Element->SetText(std::move(eaten.Value()));
        }
        else {
            Expect_(lexer, eaten, Lexer::Symbols::Less);

            // XML comment
            if ((poken = lexer.Peek(Lexer::Symbols::Not)) ) {
                Expect_(lexer, eaten, Lexer::Symbols::Not);
                Expect_(lexer, eaten, Lexer::Symbols::Decrement);

                if (lexer.SkipUntil('-'))
                    CORE_THROW_IT(XMLException("unterminated comment", eaten.Site()));

                Expect_(lexer, eaten, Lexer::Symbols::Decrement);
                Expect_(lexer, eaten, Lexer::Symbols::Greater);
                continue;
            }

            // XML close tag ?
            if ((poken = lexer.Peek(Lexer::Symbols::Div)) ) {
                Expect_(lexer, eaten, Lexer::Symbols::Div);
                if (visited.empty())
                    CORE_THROW_IT(XMLException("no opened tag", eaten.Site()));

                Expect_(lexer, eaten, Lexer::Symbols::Identifier);

                if (not EqualsI(MakeStringView(visited.back().Element->Type()), eaten.MakeView()))
                    CORE_THROW_IT(XMLException("mismatching closing tag", eaten.Site()));

                visited.back().RegisterIFN(keyId, document->_byIdentifier);

                visited.pop_back();

                Expect_(lexer, eaten, Lexer::Symbols::Greater);
                continue;
            }

            Expect_(lexer, eaten, Lexer::Symbols::Identifier);

            ReadElement_ it(eaten);

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
            while ((poken = lexer.Peek(Lexer::Symbols::Identifier)) ) {
                Expect_(lexer, eaten, Lexer::Symbols::Identifier);

                const XML::Name key(eaten.MakeView());

                bool added = false;
                auto value = it.Element->Attributes().FindOrAdd(key, &added);
                if (not added)
                    CORE_THROW_IT(XMLException("redefining block attribute", eaten.Site()));

                Expect_(lexer, eaten, Lexer::Symbols::Assignment);
                Expect_(lexer, eaten, Lexer::Symbols::String);

                value->second = std::move(eaten.Value());
            }

            poken = lexer.Peek();
            if (poken && poken->Symbol() == Lexer::Symbols::Div) {
                // XML short tag
                Expect_(lexer, eaten, Lexer::Symbols::Div);
                Expect_(lexer, eaten, Lexer::Symbols::Greater);

                it.RegisterIFN(keyId, document->_byIdentifier);

                RemoveRef_AssertGreaterThanZero(it.Element);
            }
            else if (poken && poken->Symbol() == Lexer::Symbols::Greater) {
                // XML unclosed tag
                Expect_(lexer, eaten, Lexer::Symbols::Greater);
                visited.emplace_back(std::move(it));
            }
            else {
                CORE_THROW_IT(XMLException("unterminated xml tag", eaten.Site()));
            }
        }
    }

    if (visited.size())
        CORE_THROW_IT(XMLException("unterminated xml block", visited.back().Site));

    return true;
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
