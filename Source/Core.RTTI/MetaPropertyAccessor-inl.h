#pragma once

#include "Core.RTTI/MetaPropertyAccessor.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
MetaFieldAccessor<T>::MetaFieldAccessor(ptrdiff_t fieldOffset, size_t fieldSize)
:   _fieldOffset(fieldOffset) {
    Assert(sizeof(T) == fieldSize);
}
//----------------------------------------------------------------------------
template <typename T>
T& MetaFieldAccessor<T>::GetReference(MetaObject *object) const {
    return FieldRef_(object);
}
//----------------------------------------------------------------------------
template <typename T>
const T& MetaFieldAccessor<T>::GetReference(const MetaObject *object) const {
    return FieldRef_(object);
}
//----------------------------------------------------------------------------
template <typename T>
void MetaFieldAccessor<T>::GetCopy(const MetaObject *object, T& dst) const {
    dst = FieldRef_(object);
}
//----------------------------------------------------------------------------
template <typename T>
void MetaFieldAccessor<T>::GetMove(MetaObject *object, T& dst) const {
    dst = std::move(FieldRef_(object));
}
//----------------------------------------------------------------------------
template <typename T>
void MetaFieldAccessor<T>::SetMove(MetaObject *object, T&& src) const {
    FieldRef_(object) = std::move(src);
}
//----------------------------------------------------------------------------
template <typename T>
void MetaFieldAccessor<T>::SetCopy(MetaObject *object, const T& src) const {
    FieldRef_(object) = src;
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE T& MetaFieldAccessor<T>::FieldRef_(MetaObject *object) const {
    Assert(object);
    return *reinterpret_cast<T *>((char *)object + _fieldOffset);
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE const T& MetaFieldAccessor<T>::FieldRef_(const MetaObject *object) const {
    Assert(object);
    return *reinterpret_cast<const T *>((const char *)object + _fieldOffset);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Class>
MetaMemberAccessor<T, _Class>::MetaMemberAccessor(getter_type getter, mover_type mover, setter_type setter)
:   _getter(getter), _mover(mover), _setter(setter) {
    Assert(_getter);
    Assert(_mover);
    Assert(_setter);
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
T& MetaMemberAccessor<T, _Class>::GetReference(MetaObject *object) const {
    Assert(object);
    return object->_getter();
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
const T& MetaMemberAccessor<T, _Class>::GetReference(const MetaObject *object) const {
    Assert(object);
    return checked_cast<_Class *>(object)->_getter();
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
void MetaMemberAccessor<T, _Class>::GetCopy(const MetaObject *object, T& dst) const {
    Assert(object);
    dst = checked_cast<_Class *>(object)->_getter();
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
void MetaMemberAccessor<T, _Class>::GetMove(MetaObject *object, T& dst) const {
    Assert(object);
    dst = std::move(checked_cast<_Class *>(object)->_getter());
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
void MetaMemberAccessor<T, _Class>::SetMove(MetaObject *object, T&& src) const {
    checked_cast<_Class *>(object)->_mover(std::move(src));
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
void MetaMemberAccessor<T, _Class>::SetCopy(MetaObject *object, const T& src) const {
    checked_cast<_Class *>(object)->_setter(src);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Class>
MetaDelegateAccessor<T, _Class>::MetaDelegateAccessor(getter_type&& getter, mover_type&& mover, setter_type&& setter)
:   _getter(std::move(getter))
,   _mover(std::move(mover))
,   _setter(std::move(setter)) {}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
T& MetaDelegateAccessor<T, _Class>::GetReference(MetaObject *object) const {
    return _getter(checked_cast<_Class *>(object));
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
const T& MetaDelegateAccessor<T, _Class>::GetReference(const MetaObject *object) const {
    return _getter(checked_cast<_Class *>(const_cast<MetaObject *>(object)));
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
void MetaDelegateAccessor<T, _Class>::GetCopy(const MetaObject *object, T& dst) const {
    Assert(object);
    dst = _getter(checked_cast<_Class *>(const_cast<MetaObject *>(object)));
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
void MetaDelegateAccessor<T, _Class>::GetMove(MetaObject *object, T& dst) const {
    dst = std::move(_getter(checked_cast<_Class *>(object)));
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
void MetaDelegateAccessor<T, _Class>::SetMove(MetaObject *object, T&& src) const {
    Assert(object);
    _mover(checked_cast<_Class *>(object), std::move(src));
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
void MetaDelegateAccessor<T, _Class>::SetCopy(MetaObject *object, const T& src) const {
    Assert(object);
    _setter(checked_cast<_Class *>(object), src);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
