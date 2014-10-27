#pragma once

#include "Core.Graphics/Device/Geometry/VertexDeclaration.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <VertexSubPartFormat _Format, VertexSubPartSemantic _Semantic>
void VertexDeclaration::AddSubPart(size_t index) {
    Assert(!Frozen());

    typedef VertexSubPartFormatTraits<_Format> format_traits;
    static_assert(format_traits::enabled::value, "Vertex subpart format is not supported");
    typedef typename format_traits::type format_type;

    VertexSubPartKey key;
    key.Reset(_Format, _Semantic, index);

    Assert(!SubPartIFP<format_type>(key));

    // in place new the vtable
    vertexsubpartentry_type *const subPartEntry = _subParts.Push(1);
    subPartEntry->first = key;
    const VertexSubPart<format_type> *subPart =
        new (&subPartEntry->second) VertexSubPart<format_type>(_sizeInBytes);

    _sizeInBytes += subPart->SizeInBytes();
}
//----------------------------------------------------------------------------
template <VertexSubPartSemantic _Semantic, typename _Class, typename T>
void VertexDeclaration::AddTypedSubPart(T _Class:: *member, size_t index) {
    Assert(!Frozen());

    typedef VertexSubPartFormatReverseTraits<T> type_traits;
    static_assert(type_traits::enabled::value, "Vertex subpart type is not supported");

    VertexSubPartKey key;
    key.Reset(static_cast<VertexSubPartFormat>(type_traits::Format), _Semantic, index);

    const size_t offset = (size_t)&(reinterpret_cast<_Class *>(nullptr)->*member);

    Assert(!SubPartIFP<T>(key));

    // in place new the vtable
    vertexsubpartentry_type *const subPartEntry = _subParts.Push(1);
    subPartEntry->first = key;
    const VertexSubPart<T> *subPart = new (&subPartEntry->second) VertexSubPart<T>(offset);

    const size_t sizeInBytes = offset + subPart->SizeInBytes();
    if (_sizeInBytes < sizeInBytes)
        _sizeInBytes = sizeInBytes;
}
//----------------------------------------------------------------------------
template <typename T>
const VertexSubPart<T> *VertexDeclaration::SubPart(const VertexSubPartKey& key) const {
    const VertexSubPart<T> *subPart = SubPartIFP<T>(key);
    AssertRelease(subPart);
    return subPart;
}
//----------------------------------------------------------------------------
template <typename T>
const VertexSubPart<T> *VertexDeclaration::SubPartIFP(const VertexSubPartKey& key) const {
    for (const vertexsubpartentry_type& it : _subParts)
        if (it.first == key) {
            const AbstractVertexSubPart *subPart = reinterpret_cast<const AbstractVertexSubPart *>(&it.second);
            return checked_cast<const VertexSubPart<T> *>(subPart);
        }

    return nullptr;
}
//----------------------------------------------------------------------------
template <VertexSubPartFormat _Format>
const TypedVertexSubPart<_Format> *VertexDeclaration::TypedSubPart(const VertexSubPartSemantic semantic, size_t index) const {
    Assert(Frozen());

    typedef VertexSubPartFormatTraits<_Format> format_traits;
    static_assert(format_traits::enabled::value, "Vertex subpart format is not supported");

    VertexSubPartKey key;
    key.Reset(_Format, semantic, index);

    return SubPart< typename format_traits::type >(key);
}
//----------------------------------------------------------------------------
template <VertexSubPartFormat _Format>
const TypedVertexSubPart<_Format> *VertexDeclaration::TypedSubPartIFP(const VertexSubPartSemantic semantic, size_t index) const {
    Assert(Frozen());

    typedef VertexSubPartFormatTraits<_Format> format_traits;
    static_assert(format_traits::enabled::value, "Vertex subpart format is not supported");

    VertexSubPartKey key;
    key.Reset(_Format, semantic, index);

    return SubPartIFP< typename format_traits::type >(key);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
VertexDeclarator<T>::VertexDeclarator(VertexDeclaration *vdecl)
:   _vdecl(vdecl) {
    Assert(_vdecl);
    Assert(!_vdecl->Frozen());
}
//----------------------------------------------------------------------------
template <typename T>
VertexDeclarator<T>::~VertexDeclarator() {
    Assert(_vdecl);
    Assert(!_vdecl->Frozen());
    _vdecl->Freeze();
    AssertRelease(_vdecl->SizeInBytes() == sizeof(T));
}
//----------------------------------------------------------------------------
template <typename T>
template <VertexSubPartSemantic _Semantic, typename _Value>
void VertexDeclarator<T>::AddTypedSubPart(_Value T:: *member, size_t index) const {
    Assert(_vdecl);
    _vdecl->AddTypedSubPart<_Semantic>(member, index);
}
//----------------------------------------------------------------------------
template <typename T>
void VertexDeclarator<T>::SetResourceName(String&& name) {
    Assert(_vdecl);
    Assert(!name.empty());
    _vdecl->SetResourceName(std::move(name));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
