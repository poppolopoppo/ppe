#pragma once

#include "Core.RTTI/Property/MetaPropertyAccessor.h"

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
MetaFieldAccessor<T>::~MetaFieldAccessor() {}
//----------------------------------------------------------------------------
template <typename T>
MetaFieldAccessor<T>::MetaFieldAccessor(MetaFieldAccessor&& rvalue)
:   _fieldOffset(rvalue._fieldOffset) {}
//----------------------------------------------------------------------------
template <typename T>
auto MetaFieldAccessor<T>::operator =(MetaFieldAccessor&& rvalue) -> MetaFieldAccessor& {
    _fieldOffset = rvalue._fieldOffset;
    return *this;
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
MetaFunctionAccessor<T, _Class>::MetaFunctionAccessor(getter_type&& getter, mover_type&& mover, setter_type&& setter)
:   _getter(std::move(getter))
,   _mover(std::move(mover))
,   _setter(std::move(setter)) {}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
MetaFunctionAccessor<T, _Class>::~MetaFunctionAccessor() {}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
MetaFunctionAccessor<T, _Class>::MetaFunctionAccessor(MetaFunctionAccessor&& rvalue)
:   _getter(std::move(rvalue._getter))
,   _mover(std::move(rvalue._mover))
,   _setter(std::move(rvalue._setter)) {}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
auto MetaFunctionAccessor<T, _Class>::operator =(MetaFunctionAccessor&& rvalue) -> MetaFunctionAccessor& {
    _getter = std::move(rvalue._getter);
    _mover = std::move(rvalue._mover);
    _setter = std::move(rvalue._setter);
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
T& MetaFunctionAccessor<T, _Class>::GetReference(MetaObject *object) const {
    return _getter(checked_cast<_Class *>(object));
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
const T& MetaFunctionAccessor<T, _Class>::GetReference(const MetaObject *object) const {
    return _getter(checked_cast<_Class *>(const_cast<MetaObject *>(object)));
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
void MetaFunctionAccessor<T, _Class>::GetCopy(const MetaObject *object, T& dst) const {
    Assert(object);
    dst = _getter(checked_cast<_Class *>(const_cast<MetaObject *>(object)));
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
void MetaFunctionAccessor<T, _Class>::GetMove(MetaObject *object, T& dst) const {
    dst = std::move(_getter(checked_cast<_Class *>(object)));
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
void MetaFunctionAccessor<T, _Class>::SetMove(MetaObject *object, T&& src) const {
    Assert(object);
    _mover(checked_cast<_Class *>(object), std::move(src));
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
void MetaFunctionAccessor<T, _Class>::SetCopy(MetaObject *object, const T& src) const {
    Assert(object);
    _setter(checked_cast<_Class *>(object), src);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
