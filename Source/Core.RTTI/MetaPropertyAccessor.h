#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/Exceptions.h"

#include "Core/Meta/Delegate.h"

namespace Core {
namespace RTTI {
class MetaObject;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class MetaFieldAccessor {
public:
    MetaFieldAccessor(ptrdiff_t fieldOffset, size_t fieldSize);

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
class MetaMemberAccessor {
public:
    typedef const T& (_Class::* getter_type)() const;
    typedef void (_Class::* setter_type)(const T& );

    MetaMemberAccessor(getter_type getter, setter_type setter);

    T& GetReference(MetaObject *object) const;
    const T& GetReference(const MetaObject *object) const;

    void GetCopy(const MetaObject *object, T& dst) const;
    void GetMove(MetaObject *object, T& dst) const;

    void SetMove(MetaObject *object, T&& src) const;
    void SetCopy(MetaObject *object, const T& src) const;

private:
    getter_type _getter;
    setter_type _setter;
};
//----------------------------------------------------------------------------
template <typename T, typename _Class>
MetaMemberAccessor<T, _Class> MakeMemberAccessor(
    const T& (_Class::* getter)() const,
    void (_Class::* setter)(const T& ) ) {
    static_assert(  std::is_base_of<MetaObject, _Class>::value,
                    "_Class must be derived from RTTI::MetaObject");
    return MetaMemberAccessor<T, _Class>(getter, setter);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Class>
class MetaDelegateAccessor {
public:
    static_assert(  std::is_base_of<MetaObject, _Class>::value,
                    "_Class must be derived from RTTI::MetaObject");

    typedef Delegate<T&   (*)(_Class* )> getter_type;
    typedef Delegate<void (*)(_Class*, T&& )> mover_type;
    typedef Delegate<void (*)(_Class*, const T& )> setter_type;

    MetaDelegateAccessor(getter_type&& getter, mover_type&& mover, setter_type&& setter);

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
MetaDelegateAccessor<T, _Class> MakeDelegateAccessor(
    Delegate<T&   (*)(_Class* )>&& getter,
    Delegate<void (*)(_Class*, T&& )>&& mover,
    Delegate<void (*)(_Class*, const T& )>&& setter) {
    return MetaDelegateAccessor<T, _Class>(std::move(getter), std::move(mover), std::move(setter));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Class>
class MetaDeprecatedAccessor {
public:
    static_assert(  std::is_base_of<MetaObject, _Class>::value,
                    "_Class must be derived from RTTI::MetaObject");

    MetaDeprecatedAccessor() {}

    T& GetReference(MetaObject *object) const { throw PropertyException("deprecated"); }
    const T& GetReference(const MetaObject *object) const { throw PropertyException("deprecated"); }

    void GetCopy(const MetaObject *object, T& dst) const { throw PropertyException("deprecated"); }
    void GetMove(MetaObject *object, T& dst) const { throw PropertyException("deprecated"); }

    void SetMove(MetaObject *object, T&& src) const {
        UNUSED(object);
        // still moves the value on this stack, will be freed on return
        const T value(std::move(src)); // <- maintains coherency with default properties
    }
    void SetCopy(MetaObject *object, const T& src) const { UNUSED(object); UNUSED(src); }
};
//----------------------------------------------------------------------------
template <typename T, typename _Class>
MetaDeprecatedAccessor<T, _Class> MakeDeprecatedAccessor() {
    return MetaDeprecatedAccessor<T, _Class>();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

#include "Core.RTTI/MetaPropertyAccessor-inl.h"
