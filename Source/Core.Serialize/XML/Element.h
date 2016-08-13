#pragma once

#include "Core.Serialize/XML/XML.h"

#include "Core.Serialize/XML/Name.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/AssociativeVector.h"
#include "Core/IO/String.h"
#include "Core/IO/StringSlice.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace XML {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(Element);
class Element : public RefCountable {
public:
    typedef ASSOCIATIVE_VECTORINSITU(XML, Name, String, 1) attributes_type;
    typedef VECTORINSITU(XML, PElement, 1) children_type;

    Element();
    ~Element();

    Element(const Element& ) = delete;
    Element& operator =(const Element& ) = delete;

    const Name& Type() const { return _type; }
    void SetType(Name&& rvalue) { _type = std::move(rvalue); }
    void SetType(const Name& value) { _type = value; }

    const String& Text() const { return _text; }
    void SetText(String&& rvalue) { _text = std::move(rvalue); }
    void SetText(const StringSlice& value) { _text = ToString(value); }

    attributes_type& Attributes() { return _attributes; }
    const attributes_type& Attributes() const { return _attributes; }

    children_type& Children() { return _children; }
    const children_type& Children() const { return _children; }

    Element* Parent() { return _parent.get(); }
    const Element* Parent() const { return _parent.get(); }
    void SetParent(Element* value) { _parent.reset(value); }

    Element* NextSibling() { return _nextSibling.get(); }
    const Element* NextSibling() const { return _nextSibling.get(); }
    void SetNextSibling(Element* value) { _nextSibling.reset(value); }

    Element* PrevSibling() { return _prevSibling.get(); }
    const Element* PrevSibling() const { return _prevSibling.get(); }
    void SetPrevSibling(Element* value) { _prevSibling.reset(value); }

    void ToStream(std::basic_ostream<char>& oss) const;

    size_t XPath(std::initializer_list<Name> path, const std::function<void(const Element*)>& functor) const;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    Name _type;
    String _text;
    attributes_type _attributes;

    SElement _parent;
    SElement _nextSibling;
    SElement _prevSibling;

    children_type _children;
};
//----------------------------------------------------------------------------
inline std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, const Element& elt) {
    elt.ToStream(oss);
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace XML
} //!namespace Core
