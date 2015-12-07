#pragma once

#include "Core.RTTI/MetaProperty.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core.RTTI/MetaTypePromote.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
MetaTypedProperty<T>::MetaTypedProperty(const MetaPropertyName& name, Flags attributes)
:   MetaProperty(name, attributes) {}
//----------------------------------------------------------------------------
template <typename T>
MetaTypedProperty<T>::~MetaTypedProperty() {}
//----------------------------------------------------------------------------
template <typename T>
MetaTypeInfo MetaTypedProperty<T>::TypeInfo() const {
    return RTTI::TypeInfo< T >();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(RTTI, MetaWrappedProperty<T COMMA _Accessor>, template <typename T COMMA typename _Accessor>)
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
MetaWrappedProperty<T, _Accessor>::MetaWrappedProperty(const MetaPropertyName& name, Flags attributes, accessor_type&& accessor)
:   typed_property_type(name, attributes)
,   accessor_type(std::move(accessor)) {}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
MetaWrappedProperty<T, _Accessor>::~MetaWrappedProperty() {}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
bool MetaWrappedProperty<T, _Accessor>::IsDefaultValue(const MetaObject *object) const {
    Assert(object);

    const wrapped_type& wrapped = accessor_type::GetReference(object);
    return meta_type_traits::IsDefaultValue(wrapped);
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
void MetaWrappedProperty<T, _Accessor>::GetCopy(const MetaObject *object, wrapper_type& dst) const {
    Assert(object);

    T value;
    accessor_type::GetCopy(object, value);
    meta_type_traits::WrapMove(dst, std::move(value));
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
void MetaWrappedProperty<T, _Accessor>::GetMove(MetaObject *object, wrapper_type& dst) const {
    Assert(!(MetaProperty::ReadOnly & _attributes));
    Assert(object);

    T value;
    accessor_type::GetMove(object, value);
    meta_type_traits::WrapMove(dst, std::move(value));
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
void MetaWrappedProperty<T, _Accessor>::SetMove(MetaObject *object, wrapper_type&& src) const {
    Assert(!(MetaProperty::ReadOnly & _attributes));
    Assert(object);

    T value;
    meta_type_traits::UnwrapMove(value, std::move(src));
    accessor_type::SetMove(object, std::move(value));
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
void MetaWrappedProperty<T, _Accessor>::SetCopy(MetaObject *object, const wrapper_type& src) const {
    Assert(!(MetaProperty::ReadOnly & _attributes));
    Assert(object);

    T value;
    meta_type_traits::UnwrapCopy(value, src);
    accessor_type::SetMove(object, std::move(value));
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
MetaAtom *MetaWrappedProperty<T, _Accessor>::WrapMove(MetaObject *src) const {
    Assert(!(MetaProperty::ReadOnly & _attributes));
    Assert(src);

    T value;
    accessor_type::GetMove(src, value);
    return MakeAtom(std::move(value));
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
MetaAtom *MetaWrappedProperty<T, _Accessor>::WrapCopy(const MetaObject *src) const {
    Assert(src);

    T value;
    accessor_type::GetCopy(src, value);
    return MakeAtom(std::move(value));
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
bool MetaWrappedProperty<T, _Accessor>::UnwrapMove(MetaObject *dst, MetaAtom *src) const {
    Assert(dst);
    Assert(src);

    MetaWrappedAtom<T> tmp;
    if (false == tmp.Traits()->AssignMove(&tmp, src))
        return false;

    T wrapped;
    meta_type_traits::UnwrapMove(wrapped, std::move(tmp.Wrapper()));
    accessor_type::SetMove(dst, std::move(wrapped));

    return true;
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
bool MetaWrappedProperty<T, _Accessor>::UnwrapCopy(MetaObject *dst, const MetaAtom *src) const {
    Assert(dst);
    Assert(src);

    MetaWrappedAtom<T> tmp;
    if (false == tmp.Traits()->AssignCopy(&tmp, src))
        return false;

    T wrapped;
    meta_type_traits::UnwrapMove(wrapped, std::move(tmp.Wrapper()));
    accessor_type::SetMove(dst, std::move(wrapped));

    return true;
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
void MetaWrappedProperty<T, _Accessor>::MoveTo(MetaObject *object, MetaAtom *atom) const {
    Assert(!(MetaProperty::ReadOnly & _attributes));
    Assert(object);
    Assert(atom);
    Assert(atom->TypeInfo().Id == meta_type::TypeId);

    T value;
    accessor_type::GetMove(object, value);

    typed_atom_type *const dst = checked_cast<typed_atom_type *>(atom);
    meta_type_traits::WrapMove(dst->Wrapper(), std::move(value));
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
void MetaWrappedProperty<T, _Accessor>::CopyTo(const MetaObject *object, MetaAtom *atom) const {
    Assert(object);
    Assert(atom);
    Assert(atom->TypeInfo().Id == meta_type::TypeId);

    T value;
    accessor_type::GetCopy(object, value);

    typed_atom_type *const dst = checked_cast<typed_atom_type *>(atom);
    meta_type_traits::WrapMove(dst->Wrapper(), std::move(value));
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
void MetaWrappedProperty<T, _Accessor>::MoveFrom(MetaObject *object, MetaAtom *atom) const {
    Assert(!(MetaProperty::ReadOnly & _attributes));
    Assert(object);
    Assert(atom);
    Assert(atom->TypeInfo().Id == meta_type::TypeId);

    T value;
    typed_atom_type *const src = checked_cast<typed_atom_type *>(atom);
    meta_type_traits::UnwrapMove(value, std::move(src->Wrapper()));

    accessor_type::SetMove(object, std::move(value));
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
void MetaWrappedProperty<T, _Accessor>::CopyFrom(MetaObject *object, const MetaAtom *atom) const {
    Assert(!(MetaProperty::ReadOnly & _attributes));
    Assert(object);
    Assert(atom);
    Assert(atom->TypeInfo().Id == meta_type::TypeId);

    T value;
    const typed_atom_type *src = checked_cast<const typed_atom_type *>(atom);
    meta_type_traits::UnwrapCopy(value, src->Wrapper());

    accessor_type::SetMove(object, std::move(value));
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
void MetaWrappedProperty<T, _Accessor>::Move(MetaObject *dst, MetaObject *src) const {
    Assert(src);
    Assert(dst);

    accessor_type::GetReference(dst) = std::move(accessor_type::GetReference(src));
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
void MetaWrappedProperty<T, _Accessor>::Copy(MetaObject *dst, const MetaObject *src) const {
    Assert(src);
    Assert(dst);

    T& pdst = accessor_type::GetReference(dst);
    const T& psrc = accessor_type::GetReference(src);

    pdst = psrc;
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
void MetaWrappedProperty<T, _Accessor>::Swap(MetaObject *lhs, MetaObject *rhs) const {
    Assert(lhs);
    Assert(rhs);

    T& plhs = accessor_type::GetReference(lhs);
    T& prhs = accessor_type::GetReference(rhs);

    using std::swap;
    using Core::swap;
    swap(plhs, prhs);
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
bool MetaWrappedProperty<T, _Accessor>::Equals(const MetaObject *lhs, const MetaObject *rhs) const {
    Assert(lhs);
    Assert(rhs);

    const T& plhs = accessor_type::GetReference(lhs);
    const T& prhs = accessor_type::GetReference(rhs);

    return (plhs == prhs);
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
void *MetaWrappedProperty<T, _Accessor>::RawPtr(MetaObject *obj) const {
    Assert(obj);

    return &accessor_type::GetReference(obj);
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
const void *MetaWrappedProperty<T, _Accessor>::RawPtr(const MetaObject *obj) const {
    Assert(obj);

    return &accessor_type::GetReference(obj);
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
size_t MetaWrappedProperty<T, _Accessor>::HashValue(const MetaObject *object) const {
    Assert(object);

    const T& value = accessor_type::GetReference(object);

    using Core::hash_value;
    return hash_tuple(u64(meta_type::TypeId), value);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
