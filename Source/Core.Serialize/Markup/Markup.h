#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core.Serialize/Exceptions.h"
#include "Core.Serialize/Lexer/Location.h"
#include "Core.Serialize/Lexer/TextHeap.h"

#include "Core/Allocator/LinearHeapAllocator.h"
#include "Core/Container/AssociativeVector.h"
#include "Core/Container/IntrusiveList.h"
#include "Core/Container/Vector.h"
#include "Core/IO/FS/Filename.h"
#include "Core/IO/TextWriter_fwd.h"

namespace Core {
class IBufferedStreamReader;
namespace Lexer { class FLexer; }
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMarkupException : public Core::Serialize::FSerializeException {
public:
    typedef Core::Serialize::FSerializeException parent_type;

    FMarkupException(const char *what)
        : FMarkupException(what, Lexer::FLocation()) {}

    FMarkupException(const char *what, const Lexer::FLocation& site)
        : parent_type(what), _site(site) {}

    virtual ~FMarkupException() {}

    const Lexer::FLocation& Site() const { return _site; }

private:
    Lexer::FLocation _site;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CORE_SERIALIZE_API FMarkup {
public:
    using FTextHeap = TTextHeap<false/* don't use padding to get smaller FElement */>;
    using FText = FTextHeap::FText;

    class CORE_SERIALIZE_API FElement {
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

        ~FElement() {}

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
    LINEARHEAP(Markup) _heap;
    FElement _root;
    FTextHeap _textHeap;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
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
} //!namespace Core
