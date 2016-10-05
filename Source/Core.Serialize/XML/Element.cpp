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
static void PrintElement_(std::basic_ostream<char>& oss, const XML::FElement* elt, size_t level = 0) {
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
template <typename _It>
static size_t XPath_(_It first, _It last, const XML::FElement& elt, const std::function<void(const FElement&)>& functor) {
    Assert(first != last);

    if (*first != elt.Type())
        return 0;

    ++first;

    if (first == last) {
        functor(elt);
        return 1;
    }
    else {
        size_t count = 0;
        for (const PElement& child : elt.Children()) {
            Assert(child);
            count += XPath_(first, last, *child.get(), functor);
        }

        return count;
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(XML, FElement, );
//----------------------------------------------------------------------------
FElement::FElement() {}
//----------------------------------------------------------------------------
FElement::~FElement() {
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
void FElement::ToStream(std::basic_ostream<char>& oss) const {
    PrintElement_(oss, this);
}
//----------------------------------------------------------------------------
FStringView FElement::operator [](const XML::FName& name) const {
    Assert(!name.empty());

    const FString* pvalue = _attributes.GetIFP(name);
    return (pvalue ? MakeStringView(*pvalue) : FStringView());
}
//----------------------------------------------------------------------------
const FElement* FElement::XPath(const TMemoryView<const FName>& path) const {
    Assert(!path.empty());

    const FElement* result = nullptr;
    XPath_(path.begin(), path.end(), *this, [&result](const FElement& elt) {
        result = &elt;
    });

    return result;
}
//----------------------------------------------------------------------------
size_t FElement::XPath(const TMemoryView<const FName>& path, const std::function<void(const FElement&)>& functor) const {
    Assert(!path.empty());

    return XPath_(path.begin(), path.end(), *this, functor);
}
//----------------------------------------------------------------------------
const FElement* FElement::ChildXPath(const TMemoryView<const FName>& path) const {
    const FElement* result = nullptr;
    for (const PElement& child : _children) {
        result = child->XPath(path);
        if (result)
            break;
    }
    return result;
}
//----------------------------------------------------------------------------
size_t FElement::ChildXPath(const TMemoryView<const FName>& path, const std::function<void(const FElement&)>& functor) const {
    size_t count = 0;
    for (const PElement& child : _children) {
        count += child->XPath(path, functor);
    }
    return count;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace XML
} //!namespace Core
