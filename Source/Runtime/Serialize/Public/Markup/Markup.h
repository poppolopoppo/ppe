#pragma once

#include "Serialize.h"

#include "Lexer/Location.h"
#include "Lexer/TextHeap.h"
#include "SerializeExceptions.h"

#include "Allocator/LinearAllocator.h"
#include "Container/AssociativeVector.h"
#include "Container/IntrusiveList.h"
#include "Container/Vector.h"
#include "IO/Filename.h"
#include "IO/TextWriter_fwd.h"

namespace PPE {
class IBufferedStreamReader;
namespace Lexer { class FLexer; }
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FMarkupException : public PPE::Serialize::FSerializeException {
public:
    typedef PPE::Serialize::FSerializeException parent_type;

    FMarkupException(const char *what)
        : FMarkupException(what, Lexer::FLocation()) {}

    FMarkupException(const char *what, const Lexer::FLocation& site)
        : parent_type(what), _site(site) {}

    virtual ~FMarkupException() = default;

    const Lexer::FLocation& Site() const { return _site; }

#if USE_PPE_EXCEPTION_DESCRIPTION
    virtual FWTextWriter& Description(FWTextWriter& oss) const override final;
#endif

private:
    Lexer::FLocation _site;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FMarkup {
public:
    using FTextHeap = TTextHeap<false/* don't use padding to get smaller FElement */>;
    using FText = FTextHeap::FText;

    class PPE_SERIALIZE_API FElement {
    public:
        using attributes_type = ASSOCIATIVE_VECTOR_LINEARHEAP(FText, FText);
        using node_type = TIntrusiveListNode<FElement>;

        FElement(FMarkup& markup);

        const FText& Name() const { return _name; }
        void SetName(const FText& value) { _name = value; }

        const FText& Text() const { return _text; }
        void SetText(const FText& value) { _text = value; }

        attributes_type& Attributes() { return _attributes; }
        const attributes_type& Attributes() const { return _attributes; }

        const node_type& Depth() const { return _depth; }
        const node_type& Breadth() const { return _breadth; }

        void ToStream(FTextWriter& oss, bool minify = true) const;
        void ToStream(FWTextWriter& oss, bool minify = true) const;

    private:
        friend class FMarkup;

        ~FElement() = default;

        FElement(const FElement&) = default;
        FElement& operator =(const FElement&) = default;

        FText _name;
        FText _text;

        node_type _depth;
        node_type _breadth;

        attributes_type _attributes;
    };

public:
    virtual ~FMarkup();

    FMarkup(const FMarkup&) = delete;
    FMarkup& operator =(const FMarkup&) = delete;

    FElement& Root() { return _root; }
    const FElement& Root() const { return _root; }

    bool empty() const { return (nullptr == _root.Depth().Next); }

    FElement* MakeElement(const FText& name, FElement* parent);
    FText MakeString(const FStringView& str, bool mergeable = true);

    void ToStream(FTextWriter& oss, bool minify = false) const;
    void ToStream(FWTextWriter& oss, bool minify = false) const;

    static bool Load(FMarkup* markup, const FFilename& filename);
    static bool Load(FMarkup* markup, const FFilename& filename, IBufferedStreamReader* input);
    static bool Load(FMarkup* markup, const FFilename& filename, const FStringView& content);

protected:
    FMarkup(); // this is an abstract class, must be inherited

    virtual bool ReadHeaders(Lexer::FLexer& lexer) = 0;
    virtual void WriteHeaders(FTextWriter& oss, bool minify) const = 0;

private:
    LINEARHEAP_POOLED(Markup) _heap;
    FElement _root;
    FTextHeap _textHeap;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const Serialize::FMarkup::FText& markupText) {
    return oss << markupText.MakeView();
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const Serialize::FMarkup::FElement& markupElement) {
    markupElement.ToStream(oss);
    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const Serialize::FMarkup& markup) {
    markup.ToStream(oss);
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
