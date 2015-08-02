#pragma once

#include "Core/Core.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MetaObject;
//----------------------------------------------------------------------------
template <typename T>
class MetaPropertyAccessor {
public:
    MetaPropertyAccessor() {}
    ~MetaPropertyAccessor() {}

    MetaPropertyAccessor(const MetaPropertyAccessor&) = delete;
    MetaPropertyAccessor& operator =(const MetaPropertyAccessor&) = delete;

    T& GetReference(MetaObject *object) const = delete;
    const T& GetReference(const MetaObject *object) const = delete;

    void GetCopy(const MetaObject *object, T& dst) const = delete;
    void GetMove(MetaObject *object, T& dst) const = delete;

    void SetMove(MetaObject *object, T&& src) const = delete;
    void SetCopy(MetaObject *object, const T& src) const = delete;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class MetaFieldAccessor : public MetaPropertyAccessor<T> {
public:
    MetaFieldAccessor(ptrdiff_t fieldOffset, size_t fieldSize);
    ~MetaFieldAccessor();

    MetaFieldAccessor(MetaFieldAccessor&& rvalue);
    MetaFieldAccessor& operator =(MetaFieldAccessor&& rvalue);

    T& GetReference(MetaObject *object) const;
    const T& GetReference(const MetaObject *object) const;

    void GetCopy(const MetaObject *object, T& dst) const;
    void GetMove(MetaObject *object, T& dst) const;

    void SetMove(MetaObject *object, T&& src) const;
    void SetCopy(MetaObject *object, const T& src) const;

private:
    T& FieldRef_(MetaObject *object) const;
    const T& FieldRef_(const MetaObject *object) const;

    ptrdiff_t _fieldOffset;
};
//----------------------------------------------------------------------------
template <typename _Class, typename T>
MetaFieldAccessor<T> MakeFieldAccessor(T _Class::* field) {
    static_assert(  std::is_base_of<MetaObject, _Class>::value,
                    "_Class must be derived from RTTI::MetaObject");

    const ptrdiff_t offset = (ptrdiff_t)&(((_Class*)0)->*field);
    return MetaFieldAccessor<T>(offset, sizeof(T));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Class>
class MetaFunctionAccessor : public MetaPropertyAccessor<T> {
public:
    static_assert(  std::is_base_of<MetaObject, _Class>::value,
                    "_Class must be derived from RTTI::MetaObject");

    typedef std::function<T&(_Class *)> getter_type;
    typedef std::function<void(_Class *, T&&)> mover_type;
    typedef std::function<void(_Class *, const T&)> setter_type;

    MetaFunctionAccessor(getter_type&& getter, mover_type&& mover, setter_type&& setter);
    ~MetaFunctionAccessor();

    MetaFunctionAccessor(MetaFunctionAccessor&& rvalue);
    MetaFunctionAccessor& operator =(MetaFunctionAccessor&& rvalue);

    T& GetReference(MetaObject *object) const;
    const T& GetReference(const MetaObject *object) const;

    void GetCopy(const MetaObject *object, T& dst) const;
    void GetMove(MetaObject *object, T& dst) const;

    void SetMove(MetaObject *object, T&& src) const;
    void SetCopy(MetaObject *object, const T& src) const;

private:
    getter_type _getter;
    mover_type  _mover;
    setter_type _setter;
};
//----------------------------------------------------------------------------
template <typename T, typename _Class>
MetaFunctionAccessor<T, _Class> MakeFunctionAccessor(
    std::function<T&(_Class *)>&& getter,
    std::function<void(_Class *, T&&)>&& mover,
    std::function<void(_Class *, const T&)>&& setter) {
    return MetaFunctionAccessor<T, _Class>(std::move(getter), std::move(mover), std::move(setter));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

#include "Core.RTTI/Property/MetaPropertyAccessor-inl.h"
