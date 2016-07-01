#include "stdafx.h"

#include "Element.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace XML {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void PrintElement_(std::basic_ostream<char>& oss, const XML::Element* elt, size_t level = 0) {
    forrange(i, 0, level)
        oss << "  ";
    oss << "<" << elt->Type();

    for (const auto& it : elt->Attributes())
        oss << " " << it.first << "=\"" << it.second << "\"";

    if (elt->Children().empty()) {
        if (elt->Text().size()) {
            oss << ">" << elt->Text() << "</" << elt->Type() << ">" << std::endl;
        }
        else {
            oss << "/>" << std::endl;
        }
    }
    else {
        Assert(elt->Text().empty());
        oss << ">" << std::endl;

        for (const XML::PElement& child : elt->Children())
            PrintElement_(oss, child.get(), level + 1);

        forrange(i, 0, level)
            oss << "  ";
        oss << "</" << elt->Type() << ">" << std::endl;
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(XML, Element, );
//----------------------------------------------------------------------------
Element::Element() {}
//----------------------------------------------------------------------------
Element::~Element() {
    // Detach from sibling linked list before deleting (needed for safe counting)
    if (_prevSibling) {
        Assert(this == _prevSibling->_nextSibling);
        _prevSibling->_nextSibling = _nextSibling;
    }
    if (_nextSibling) {
        Assert(this == _nextSibling->_prevSibling);
        _nextSibling->_prevSibling = _prevSibling;
    }
}
//----------------------------------------------------------------------------
void Element::ToStream(std::basic_ostream<char>& oss) const {
    PrintElement_(oss, this);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace XML
} //!namespace Core
