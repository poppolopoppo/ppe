#pragma once

#include "Core.Serialize/XML/XML.h"

#include "Core.Serialize/XML/Name.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/AssociativeVector.h"
#include "Core/IO/String.h"
#include "Core/IO/StringView.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace XML {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(Element);
class FElement : public FRefCountable {
public:
    typedef ASSOCIATIVE_VECTORINSITU(XML, FName, FString, 1) attributes_type;
    typedef VECTORINSITU(XML, PElement, 1) children_type;

    FElement();
    ~FElement();

    FElement(const FElement& ) = delete;
    FElement& operator =(const FElement& ) = delete;

    const FName& Type() const { return _type; }
    void SetType(FName&& rvalue) { _type = std::move(rvalue); }
    void SetType(const FName& value) { _type = value; }

    const FString& Text() const { return _text; }
    void SetText(FString&& rvalue) { _text = std::move(rvalue); }
    void SetText(const FStringView& value) { _text = ToString(value); }

    attributes_type& Attributes() { return _attributes; }
    const attributes_type& Attributes() const { return _attributes; }

    children_type& Children() { return _children; }
    const children_type& Children() const { return _children; }

    FElement* Parent() { return _parent.get(); }
    const FElement* Parent() const { return _parent.get(); }
    void SetParent(FElement* value) { _parent.reset(value); }

    FElement* NextSibling() { return _nextSibling.get(); }
    const FElement* NextSibling() const { return _nextSibling.get(); }
    void SetNextSibling(FElement* value) { _nextSibling.reset(value); }

    FElement* PrevSibling() { return _prevSibling.get(); }
    const FElement* PrevSibling() const { return _prevSibling.get(); }
    void SetPrevSibling(FElement* value) { _prevSibling.reset(value); }

    void ToStream(std::basic_ostream<char>& oss) const;

    FStringView operator [](const XML::FName& name) const; // return the attribute IFP

    const FElement* XPath(const TMemoryView<const FName>& path) const;
    size_t XPath(const TMemoryView<const FName>& path, const Meta::TFunction<void(const FElement&)>& functor) const;

    const FElement* ChildXPath(const TMemoryView<const FName>& path) const;
    size_t ChildXPath(const TMemoryView<const FName>& path, const Meta::TFunction<void(const FElement&)>& functor) const;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    FName _type;
    FString _text;
    attributes_type _attributes;

    SElement _parent;
    SElement _nextSibling;
    SElement _prevSibling;

    children_type _children;
};
//----------------------------------------------------------------------------
inline std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, const FElement& elt) {
    elt.ToStream(oss);
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace XML
} //!namespace Core
