#include "stdafx.h"

#include "Markup/Markup.h"

#include "Lexer/Lexer.h"
#include "Lexer/Match.h"
#include "Lexer/Symbol.h"
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
using FMarkupStack_ = VECTORINSITU(Markup, FMarkup::FElement*, 32);
//----------------------------------------------------------------------------
static void ParseAttributesIFP_(Lexer::FLexer& lexer, FMarkup& markup, FMarkup::FElement& elt) {
    Lexer::FMatch key, value, equals;
    while (lexer.Peek(Lexer::FSymbols::Identifier)) {
        if (not lexer.Expect(key, Lexer::FSymbols::Identifier))
            AssertNotReached();

        if (not lexer.Expect(equals, Lexer::FSymbols::Assignment))
            PPE_THROW_IT(FMarkupException("missing assignment in attributes", key.Site()));

        if (not lexer.Expect(value, Lexer::FSymbols::String))
            PPE_THROW_IT(FMarkupException("expected string as value in attributes", equals.Site()));

        elt.Attributes().Emplace_AssertUnique(
            markup.MakeString(key.MakeView()),
            markup.MakeString(value.MakeView()) );
    }
}
//----------------------------------------------------------------------------
static bool ParseComments_(Lexer::FLexer& lexer) {
    lexer.EatWhiteSpaces();

    if (not lexer.ReadIFN("<!--"))
        return false;

    if (not lexer.SkipUntil("-->"))
        PPE_THROW_IT(FMarkupException("unterminated comment", lexer.SourceSite()));

    return true;
}
//----------------------------------------------------------------------------
static bool ParseTag_(Lexer::FLexer& lexer, FMarkup& markup, FMarkupStack_& visiteds) {
    lexer.EatWhiteSpaces();

    Lexer::FMatch start;
    if (not lexer.ReadIFN('<'))
        return false;

    if (lexer.ReadIFN('/')) {
        // closing tag

        if (visiteds.size() == 1/* _root is always inserted */)
            PPE_THROW_IT(FMarkupException("assymetrical closing tag", start.Site()));

        FMarkup::FElement* const closing = visiteds.pop_back_ReturnBack();
        Assert(closing);

        Lexer::FMatch type;
        if (not lexer.Expect(type, Lexer::FSymbols::Identifier))
            PPE_THROW_IT(FMarkupException("expected indentifier in closing tag", type.Site()));

        if (type.Value() != closing->Name().MakeView())
            PPE_THROW_IT(FMarkupException("mismatching closing tag", type.Site()));
    }
    else {
        // opening tag

        Assert(visiteds.size()); // thanks to FMarkup::_root

        Lexer::FMatch name;
        if (not lexer.Expect(name, Lexer::FSymbols::Identifier))
            PPE_THROW_IT(FMarkupException("expected indentifier in opening tag", name.Site()));

        const FMarkup::FText nameStr = markup.MakeString(name.Value());
        FMarkup::FElement* const opening = markup.MakeElement(nameStr, visiteds.back());

        visiteds.push_back(opening);

        ParseAttributesIFP_(lexer, markup, *opening);
    }

    if (not lexer.Expect(Lexer::FSymbols::Greater))
        PPE_THROW_IT(FMarkupException("unclosed tag", start.Site()));

    return true;
}
//----------------------------------------------------------------------------
static void ParseInnerText_(Lexer::FLexer& lexer, FMarkup& markup, FMarkup::FElement& elt) {
    lexer.EatWhiteSpaces();

    Lexer::FMatch text;
    if (not lexer.ReadUntil(text, '<'))
        PPE_THROW_IT(FMarkupException("expecting inner text", lexer.SourceSite()));

    elt.SetText(markup.MakeString(text.Value(), false));
}
//----------------------------------------------------------------------------
static void EscapeString_(FTextWriter& oss, const FMarkup::FText& str) {
    Escape(oss, str, EEscape::Hexadecimal);
}
//----------------------------------------------------------------------------
static void EscapeString_(FWTextWriter& oss, const FMarkup::FText& str) {
    Escape(oss, ToWCStr(INLINE_MALLOCA(wchar_t, str.size() + 1), str), EEscape::Unicode);
}
//----------------------------------------------------------------------------
template <typename _Char>
static void ToStream_(const FMarkup::FElement& elt, TBasicTextWriter<_Char>& oss, Fmt::TBasicIndent<_Char>& indent, bool minify) {
    using FIndentScope_ = typename Fmt::TBasicIndent<_Char>::FScope;

    Assert(not elt.Name().empty());

    oss << Fmt::Less << elt.Name();

    if (elt.Attributes().size()) {
        const FIndentScope_ indentScope(indent);
        for (const auto& it : elt.Attributes()) {
            oss << Fmt::Space << it.first << Fmt::Assignment << Fmt::DoubleQuote;
            EscapeString_(oss, it.second);
            oss << Fmt::DoubleQuote;
        }
    }

    if (elt.Depth().Next) {
        oss << Fmt::Greater;
        if (not minify)
            oss << Eol;
        {
            const FIndentScope_ indentScope(indent);

            const FMarkup::FElement* child = elt.Depth().Next;
            Assert(nullptr == child->Breadth().Prev);
            for (; child; child = child->Breadth().Next) {
                oss << indent;
                ToStream_(*child, oss, indent, minify);
                if (not minify)
                    oss << Eol;
            }
        }
        oss << indent << Fmt::Less << Fmt::Div << elt.Name() << Fmt::Greater;
    }
    else if (not elt.Text().empty()) {
        oss << Fmt::Greater
            << elt.Text()
            << Fmt::Less << Fmt::Div << elt.Name() << Fmt::Greater;
    }
    else {
        oss << Fmt::Space << Fmt::Div << Fmt::Greater;
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMarkup::FElement::FElement(FMarkup& markup)
    : _depth{ nullptr, nullptr }
    , _breadth{ nullptr, nullptr }
    , _attributes(attributes_type::allocator_type(markup._heap))
{}
//----------------------------------------------------------------------------
void FMarkup::FElement::ToStream(FTextWriter& oss, bool minify/* = true */) const {
    Fmt::FIndent indent = (minify ? Fmt::FIndent::None() : Fmt::FIndent::TwoSpaces());
    oss << FTextFormat::DefaultFloat;
    ToStream_(*this, oss, indent, minify);
    Assert(0 == indent.Level);
}
//----------------------------------------------------------------------------
void FMarkup::FElement::ToStream(FWTextWriter& oss, bool minify/* = true */) const {
    Fmt::FWIndent indent = (minify ? Fmt::FWIndent::None() : Fmt::FWIndent::TwoSpaces());
    oss << FTextFormat::DefaultFloat;
    ToStream_(*this, oss, indent, minify);
    Assert(0 == indent.Level);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMarkup::FMarkup()
    : _root(*this)
    , _textHeap(_heap)
{}
//----------------------------------------------------------------------------
FMarkup::~FMarkup() {
    Assert(nullptr == _root._depth.Prev);
    Assert(nullptr == _root._breadth.Prev);
    Assert(nullptr == _root._breadth.Next);
    _heap.ReleaseAll();
}
//----------------------------------------------------------------------------
auto FMarkup::MakeString(const FStringView& str, bool mergeable/* = true */) -> FText {
    return _textHeap.MakeText(str, mergeable);
}
//----------------------------------------------------------------------------
auto FMarkup::MakeElement(const FText& name, FElement* parent) -> FElement* {
    Assert(not name.empty());
    Assert(parent);

    FElement* const pelt = INPLACE_NEW(_heap.Allocate(sizeof(FElement)), FElement)(*this);
    pelt->_name = name;
    pelt->_depth.Prev = parent;

    // insert at the end of the list
    FElement* prev = nullptr;
    for (FElement* p = parent->_depth.Next; p; p = p->_breadth.Next)
        prev = p;

    if (prev) {
        Assert(nullptr == prev->_breadth.Next);

        pelt->_breadth.Prev = prev;
        prev->_breadth.Next = pelt;
    }
    else {
        Assert(nullptr == parent->_depth.Next);

        parent->_depth.Next = pelt;
    }

    return pelt;
}
//----------------------------------------------------------------------------
void FMarkup::ToStream(FTextWriter& oss, bool minify/* = false */) const {
    WriteHeaders(oss, minify);

    Fmt::FIndent indent = (minify ? Fmt::FIndent::None() : Fmt::FIndent::TwoSpaces());
    oss << FTextFormat::DefaultFloat;

    for (const FElement* pelt = _root.Depth().Next; pelt; pelt = pelt->Breadth().Next) {
        Assert(0 == indent.Level);
        ToStream_(*pelt, oss, indent, minify);
        oss << Eol;
    }
}
//----------------------------------------------------------------------------
void FMarkup::ToStream(FWTextWriter& oss, bool minify/* = false */) const {
    {
        STACKLOCAL_TEXTWRITER(ansi, 2048);
        WriteHeaders(ansi, minify);
        oss << ansi.Written();
    }

    Fmt::FWIndent indent = (minify ? Fmt::FWIndent::None() : Fmt::FWIndent::TwoSpaces());
    oss << FTextFormat::DefaultFloat;

    for (const FElement* pelt = _root.Depth().Next; pelt; pelt = pelt->Breadth().Next) {
        Assert(0 == indent.Level);
        ToStream_(*pelt, oss, indent, minify);
        oss << Eol;
    }
}
//----------------------------------------------------------------------------
bool FMarkup::Load(FMarkup* markup, const FFilename& filename) {
    Assert(markup);
    Assert(not filename.empty());

    EAccessPolicy policy = EAccessPolicy::Binary;
    if (filename.Extname() == FFSConstNames::Z())
        policy = policy + EAccessPolicy::Compress;

    RAWSTORAGE(FileSystem, u8) content;
    if (not VFS_ReadAll(&content, filename, policy))
        return false;

    return Load(markup, filename, content.MakeConstView().Cast<const char>());
}
//----------------------------------------------------------------------------
bool FMarkup::Load(FMarkup* markup, const FFilename& filename, const FStringView& content) {
    FMemoryViewReader reader(content.Cast<const u8>());
    return Load(markup, filename, &reader);
}
//----------------------------------------------------------------------------
bool FMarkup::Load(FMarkup* markup, const FFilename& filename, IBufferedStreamReader* input) {
    Assert(markup);
    Assert(input);

    const FWString filenameStr(filename.ToWString());
    Lexer::FLexer lexer(*input, filenameStr.MakeView(), false);

    markup->_root = FElement(*markup);
    markup->_textHeap.Clear_ForgetMemory();
    markup->_heap.DiscardAll();

    if (not markup->ReadHeaders(lexer))
        return false;

    FMarkupStack_ visiteds;
    visiteds.push_back(&markup->_root);

    PPE_TRY{
        do {
            while (ParseComments_(lexer)) NOOP();

            if (not ParseTag_(lexer, *markup, visiteds))
                ParseInnerText_(lexer, *markup, *visiteds.back());

        } while (visiteds.size() > 1);
    }
    PPE_CATCH(Lexer::FLexerException e)
    PPE_CATCH_BLOCK({
        PPE_THROW_IT(FMarkupException(e.What(), e.Match().Site()));
    })

    Assert(nullptr == markup->_root._depth.Prev);
    Assert(nullptr == markup->_root._breadth.Prev);
    Assert(nullptr == markup->_root._breadth.Next);

    return true;
}
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_DESCRIPTION
FWTextWriter& FMarkupException::Description(FWTextWriter& oss) const {
    return oss
        << MakeCStringView(What())
        << L": in markup file '"
        << _site
        << L"' !";
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
