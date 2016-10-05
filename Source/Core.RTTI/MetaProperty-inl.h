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
TMetaTypedProperty<T>::TMetaTypedProperty(const FName& name, EFlags attributes)
:   FMetaProperty(name, attributes) {}
//----------------------------------------------------------------------------
template <typename T>
TMetaTypedProperty<T>::~TMetaTypedProperty() {}
//----------------------------------------------------------------------------
template <typename T>
FMetaTypeInfo TMetaTypedProperty<T>::TypeInfo() const {
    return RTTI::TypeInfo< T >();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(RTTI, TMetaWrappedProperty<T COMMA _Accessor>, template <typename T COMMA typename _Accessor>)
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
TMetaWrappedProperty<T, _Accessor>::TMetaWrappedProperty(const FName& name, EFlags attributes, accessor_type&& accessor)
:   typed_property_type(name, attributes)
,   accessor_type(std::move(accessor)) {}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
TMetaWrappedProperty<T, _Accessor>::~TMetaWrappedProperty() {}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
bool TMetaWrappedProperty<T, _Accessor>::IsDefaultValue(const FMetaObject *object) const {
    Assert(object);

    const wrapped_type& wrapped = accessor_type::GetReference(object);
    return meta_type_traits::IsDefaultValue(wrapped);
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
void TMetaWrappedProperty<T, _Accessor>::GetCopy(const FMetaObject *object, wrapper_type& dst) const {
    Assert(object);

    T value;
    accessor_type::GetCopy(object, value);
    meta_type_traits::WrapMove(dst, std::move(value));
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
void TMetaWrappedProperty<T, _Accessor>::GetMove(FMetaObject *object, wrapper_type& dst) const {
    Assert(!(FMetaProperty::ReadOnly & _attributes));
    Assert(object);

    T value;
    accessor_type::GetMove(object, value);
    meta_type_traits::WrapMove(dst, std::move(value));
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
void TMetaWrappedProperty<T, _Accessor>::SetMove(FMetaObject *object, wrapper_type&& src) const {
    Assert(!(FMetaProperty::ReadOnly & _attributes));
    Assert(object);

    T value;
    meta_type_traits::UnwrapMove(value, std::move(src));
    accessor_type::SetMove(object, std::move(value));
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
void TMetaWrappedProperty<T, _Accessor>::SetCopy(FMetaObject *object, const wrapper_type& src) const {
    Assert(!(FMetaProperty::ReadOnly & _attributes));
    Assert(object);

    T value;
    meta_type_traits::UnwrapCopy(value, src);
    accessor_type::SetMove(object, std::move(value));
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
FMetaAtom *TMetaWrappedProperty<T, _Accessor>::WrapMove(FMetaObject *src) const {
    Assert(!(FMetaProperty::ReadOnly & _attributes));
    Assert(src);

    T value;
    accessor_type::GetMove(src, value);
    return MakeAtom(std::move(value));
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
FMetaAtom *TMetaWrappedProperty<T, _Accessor>::WrapCopy(const FMetaObject *src) const {
    Assert(src);

    T value;
    accessor_type::GetCopy(src, value);
    return MakeAtom(std::move(value));
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
bool TMetaWrappedProperty<T, _Accessor>::UnwrapMove(FMetaObject *dst, FMetaAtom *src) const {
    Assert(dst);
    Assert(src);

    typename TMetaAtomWrapper<T>::type tmp;
    if (false == Traits()->AssignMove(&tmp, src))
        return false;

    T wrapped;
    meta_type_traits::UnwrapMove(wrapped, std::move(tmp.Wrapper()));
    accessor_type::SetMove(dst, std::move(wrapped));

    return true;
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
bool TMetaWrappedProperty<T, _Accessor>::UnwrapCopy(FMetaObject *dst, const FMetaAtom *src) const {
    Assert(dst);
    Assert(src);

    typename TMetaAtomWrapper<T>::type tmp;
    if (false == Traits()->AssignCopy(&tmp, src))
        return false;

    T wrapped;
    meta_type_traits::UnwrapMove(wrapped, std::move(tmp.Wrapper()));
    accessor_type::SetMove(dst, std::move(wrapped));

    return true;
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
void TMetaWrappedProperty<T, _Accessor>::MoveTo(FMetaObject *object, FMetaAtom *atom) const {
    Assert(!(FMetaProperty::ReadOnly & _attributes));
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
void TMetaWrappedProperty<T, _Accessor>::CopyTo(const FMetaObject *object, FMetaAtom *atom) const {
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
void TMetaWrappedProperty<T, _Accessor>::MoveFrom(FMetaObject *object, FMetaAtom *atom) const {
    Assert(!(FMetaProperty::ReadOnly & _attributes));
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
void TMetaWrappedProperty<T, _Accessor>::CopyFrom(FMetaObject *object, const FMetaAtom *atom) const {
    Assert(!(FMetaProperty::ReadOnly & _attributes));
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
void TMetaWrappedProperty<T, _Accessor>::Move(FMetaObject *dst, FMetaObject *src) const {
    Assert(src);
    Assert(dst);

    accessor_type::GetReference(dst) = std::move(accessor_type::GetReference(src));
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
void TMetaWrappedProperty<T, _Accessor>::Copy(FMetaObject *dst, const FMetaObject *src) const {
    Assert(src);
    Assert(dst);

    T& pdst = accessor_type::GetReference(dst);
    const T& psrc = accessor_type::GetReference(src);

    pdst = psrc;
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
void TMetaWrappedProperty<T, _Accessor>::Swap(FMetaObject *lhs, FMetaObject *rhs) const {
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
bool TMetaWrappedProperty<T, _Accessor>::Equals(const FMetaObject *lhs, const FMetaObject *rhs) const {
    Assert(lhs);
    Assert(rhs);

    const T& plhs = accessor_type::GetReference(lhs);
    const T& prhs = accessor_type::GetReference(rhs);

    return (plhs == prhs);
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
bool TMetaWrappedProperty<T, _Accessor>::DeepEquals(const FMetaObject *lhs, const FMetaObject *rhs) const {
    Assert(lhs);
    Assert(rhs);

    const T& plhs = accessor_type::GetReference(lhs);
    const T& prhs = accessor_type::GetReference(rhs);

    return meta_type_traits::DeepEquals(plhs, prhs);
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
void *TMetaWrappedProperty<T, _Accessor>::RawPtr(FMetaObject *obj) const {
    Assert(obj);

    return &accessor_type::GetReference(obj);
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
const void *TMetaWrappedProperty<T, _Accessor>::RawPtr(const FMetaObject *obj) const {
    Assert(obj);

    return &accessor_type::GetReference(obj);
}
//----------------------------------------------------------------------------
template <typename T, typename _Accessor>
size_t TMetaWrappedProperty<T, _Accessor>::HashValue(const FMetaObject *object) const {
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
