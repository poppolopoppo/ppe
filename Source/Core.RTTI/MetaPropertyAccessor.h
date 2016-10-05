#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/Exceptions.h"

#include "Core/Meta/Delegate.h"

namespace Core {
namespace RTTI {
class FMetaObject;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TMetaFieldAccessor {
public:
    TMetaFieldAccessor(ptrdiff_t fieldOffset, size_t fieldSize);

    T& GetReference(FMetaObject *object) const;
    const T& GetReference(const FMetaObject *object) const;

    void GetCopy(const FMetaObject *object, T& dst) const;
    void GetMove(FMetaObject *object, T& dst) const;

    void SetMove(FMetaObject *object, T&& src) const;
    void SetCopy(FMetaObject *object, const T& src) const;

private:
    T& FieldRef_(FMetaObject *object) const;
    const T& FieldRef_(const FMetaObject *object) const;

    ptrdiff_t _fieldOffset;
};
//----------------------------------------------------------------------------
template <typename _Class, typename T>
TMetaFieldAccessor<T> MakeFieldAccessor(T _Class::* field) {
    static_assert(  std::is_base_of<FMetaObject, _Class>::value,
                    "_Class must be derived from RTTI::FMetaObject");

    const ptrdiff_t offset = (ptrdiff_t)&(((_Class*)0)->*field);
    return TMetaFieldAccessor<T>(offset, sizeof(T));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Class>
class TMetaMemberAccessor {
public:
    typedef const T& (_Class::* getter_type)() const;
    typedef void (_Class::* setter_type)(const T& );

    TMetaMemberAccessor(getter_type getter, setter_type setter);

    T& GetReference(FMetaObject *object) const;
    const T& GetReference(const FMetaObject *object) const;

    void GetCopy(const FMetaObject *object, T& dst) const;
    void GetMove(FMetaObject *object, T& dst) const;

    void SetMove(FMetaObject *object, T&& src) const;
    void SetCopy(FMetaObject *object, const T& src) const;

private:
    getter_type _getter;
    setter_type _setter;
};
//----------------------------------------------------------------------------
template <typename T, typename _Class>
TMetaMemberAccessor<T, _Class> MakeMemberAccessor(
    const T& (_Class::* getter)() const,
    void (_Class::* setter)(const T& ) ) {
    static_assert(  std::is_base_of<FMetaObject, _Class>::value,
                    "_Class must be derived from RTTI::FMetaObject");
    return TMetaMemberAccessor<T, _Class>(getter, setter);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Class>
class TMetaDelegateAccessor {
public:
    static_assert(  std::is_base_of<FMetaObject, _Class>::value,
                    "_Class must be derived from RTTI::FMetaObject");

    typedef TDelegate<T&   (*)(_Class* )> getter_type;
    typedef TDelegate<void (*)(_Class*, T&& )> mover_type;
    typedef TDelegate<void (*)(_Class*, const T& )> setter_type;

    TMetaDelegateAccessor(getter_type&& getter, mover_type&& mover, setter_type&& setter);

    T& GetReference(FMetaObject *object) const;
    const T& GetReference(const FMetaObject *object) const;

    void GetCopy(const FMetaObject *object, T& dst) const;
    void GetMove(FMetaObject *object, T& dst) const;

    void SetMove(FMetaObject *object, T&& src) const;
    void SetCopy(FMetaObject *object, const T& src) const;

private:
    getter_type _getter;
    mover_type  _mover;
    setter_type _setter;
};
//----------------------------------------------------------------------------
template <typename T, typename _Class>
TMetaDelegateAccessor<T, _Class> MakeDelegateAccessor(
    TDelegate<T&   (*)(_Class* )>&& getter,
    TDelegate<void (*)(_Class*, T&& )>&& mover,
    TDelegate<void (*)(_Class*, const T& )>&& setter) {
    return TMetaDelegateAccessor<T, _Class>(std::move(getter), std::move(mover), std::move(setter));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Class>
class TMetaDeprecatedAccessor {
public:
    static_assert(  std::is_base_of<FMetaObject, _Class>::value,
                    "_Class must be derived from RTTI::FMetaObject");

    TMetaDeprecatedAccessor() {}

    T& GetReference(FMetaObject *object) const { throw FPropertyException("deprecated"); }
    const T& GetReference(const FMetaObject *object) const { throw FPropertyException("deprecated"); }

    void GetCopy(const FMetaObject *object, T& dst) const { throw FPropertyException("deprecated"); }
    void GetMove(FMetaObject *object, T& dst) const { throw FPropertyException("deprecated"); }

    void SetMove(FMetaObject *object, T&& src) const {
        UNUSED(object);
        // still moves the value on this stack, will be freed on return
        const T value(std::move(src)); // <- maintains coherency with default properties
    }
    void SetCopy(FMetaObject *object, const T& src) const { UNUSED(object); UNUSED(src); }
};
//----------------------------------------------------------------------------
template <typename T, typename _Class>
TMetaDeprecatedAccessor<T, _Class> MakeDeprecatedAccessor() {
    return TMetaDeprecatedAccessor<T, _Class>();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

#include "Core.RTTI/MetaPropertyAccessor-inl.h"
