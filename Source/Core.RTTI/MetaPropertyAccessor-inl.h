#pragma once

#include "Core.RTTI/MetaPropertyAccessor.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
TMetaFieldAccessor<T>::TMetaFieldAccessor(ptrdiff_t fieldOffset, size_t fieldSize)
:   _fieldOffset(fieldOffset) {
    UNUSED(fieldSize);
    Assert(sizeof(T) == fieldSize);
}
//----------------------------------------------------------------------------
template <typename T>
T& TMetaFieldAccessor<T>::GetReference(FMetaObject *object) const {
    return FieldRef_(object);
}
//----------------------------------------------------------------------------
template <typename T>
const T& TMetaFieldAccessor<T>::GetReference(const FMetaObject *object) const {
    return FieldRef_(object);
}
//----------------------------------------------------------------------------
template <typename T>
void TMetaFieldAccessor<T>::GetCopy(const FMetaObject *object, T& dst) const {
    dst = FieldRef_(object);
}
//----------------------------------------------------------------------------
template <typename T>
void TMetaFieldAccessor<T>::GetMove(FMetaObject *object, T& dst) const {
    dst = std::move(FieldRef_(object));
}
//----------------------------------------------------------------------------
template <typename T>
void TMetaFieldAccessor<T>::SetMove(FMetaObject *object, T&& src) const {
    FieldRef_(object) = std::move(src);
}
//----------------------------------------------------------------------------
template <typename T>
void TMetaFieldAccessor<T>::SetCopy(FMetaObject *object, const T& src) const {
    FieldRef_(object) = src;
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE T& TMetaFieldAccessor<T>::FieldRef_(FMetaObject *object) const {
    Assert(object);
    return *reinterpret_cast<T *>((char *)object + _fieldOffset);
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE const T& TMetaFieldAccessor<T>::FieldRef_(const FMetaObject *object) const {
    Assert(object);
    return *reinterpret_cast<const T *>((const char *)object + _fieldOffset);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Class>
TMetaMemberAccessor<T, _Class>::TMetaMemberAccessor(getter_type getter, setter_type setter)
:   _getter(getter), _setter(setter) {
    Assert(_getter);
    Assert(_setter);
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
T& TMetaMemberAccessor<T, _Class>::GetReference(FMetaObject *object) const {
    Assert(object);
    return const_cast<T&>((checked_cast<const _Class*>(object)->*_getter)());
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
const T& TMetaMemberAccessor<T, _Class>::GetReference(const FMetaObject *object) const {
    Assert(object);
    return (checked_cast<const _Class*>(object)->*_getter)();
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
void TMetaMemberAccessor<T, _Class>::GetCopy(const FMetaObject *object, T& dst) const {
    dst = GetReference(object);
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
void TMetaMemberAccessor<T, _Class>::GetMove(FMetaObject *object, T& dst) const {
    dst = std::move(GetReference(object));
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
void TMetaMemberAccessor<T, _Class>::SetMove(FMetaObject *object, T&& src) const {
    GetReference(object) = std::move(src);
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
void TMetaMemberAccessor<T, _Class>::SetCopy(FMetaObject *object, const T& src) const {
    (checked_cast<_Class*>(object)->*_setter)(src);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Class>
TMetaDelegateAccessor<T, _Class>::TMetaDelegateAccessor(getter_type&& getter, mover_type&& mover, setter_type&& setter)
:   _getter(std::move(getter))
,   _mover(std::move(mover))
,   _setter(std::move(setter)) {}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
T& TMetaDelegateAccessor<T, _Class>::GetReference(FMetaObject *object) const {
    return _getter(checked_cast<_Class *>(object));
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
const T& TMetaDelegateAccessor<T, _Class>::GetReference(const FMetaObject *object) const {
    return _getter(checked_cast<_Class *>(const_cast<FMetaObject *>(object)));
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
void TMetaDelegateAccessor<T, _Class>::GetCopy(const FMetaObject *object, T& dst) const {
    Assert(object);
    dst = _getter(checked_cast<_Class *>(const_cast<FMetaObject *>(object)));
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
void TMetaDelegateAccessor<T, _Class>::GetMove(FMetaObject *object, T& dst) const {
    dst = std::move(_getter(checked_cast<_Class *>(object)));
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
void TMetaDelegateAccessor<T, _Class>::SetMove(FMetaObject *object, T&& src) const {
    Assert(object);
    _mover(checked_cast<_Class *>(object), std::move(src));
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
void TMetaDelegateAccessor<T, _Class>::SetCopy(FMetaObject *object, const T& src) const {
    Assert(object);
    _setter(checked_cast<_Class *>(object), src);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
