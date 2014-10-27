#pragma once

#include "Core/Core.h"

#include "Core/RTTI/Property/MetaPropertyAccessor.h"
#include "Core/RTTI/Property/MetaPropertyName.h"

#include "Core/RTTI/Atom/MetaAtom.h"
#include "Core/RTTI/Type/MetaType.h"
#include "Core/RTTI/Type/MetaTypeTraits.h"
#include "Core/RTTI/Type/MetaTypeVirtualTraits.h"

#include "Core/Allocator/PoolAllocator.h"

#include <type_traits>

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MetaObject;
template <typename T>
class MetaTypedProperty;
//----------------------------------------------------------------------------
class MetaProperty {
public:
    enum Flags {
        Public       = 0,
        Protected    = 1,
        Private      = 2,
        ReadOnly     = 4,
    };

    MetaProperty(const MetaPropertyName& name, Flags attributes);
    virtual ~MetaProperty();

    MetaProperty(const MetaProperty&) = delete;
    MetaProperty& operator =(const MetaProperty&) = delete;

    const MetaPropertyName& Name() const { return _name; }
    Flags Attributes() const { return _attributes; }

    virtual MetaTypeInfo TypeInfo() const = 0;
    virtual const IMetaTypeVirtualTraits *Traits() const = 0;

    virtual MetaAtom *WrapMove(MetaObject *src) const = 0;
    virtual MetaAtom *WrapCopy(const MetaObject *src) const = 0;

    virtual bool UnwrapMove(MetaObject *dst, MetaAtom *src) const = 0;
    virtual bool UnwrapCopy(MetaObject *dst, const MetaAtom *src) const = 0;

    virtual void MoveTo(MetaObject *object, MetaAtom *atom) const = 0;
    virtual void CopyTo(const MetaObject *object, MetaAtom *atom) const = 0;

    virtual void MoveFrom(MetaObject *object, MetaAtom *atom) const = 0;
    virtual void CopyFrom(MetaObject *object, const MetaAtom *atom) const = 0;

    virtual void Move(MetaObject *dst, MetaObject *src) const = 0;
    virtual void Copy(MetaObject *dst, const MetaObject *src) const = 0;

    virtual bool Equals(const MetaObject *lhs, const MetaObject *rhs) const = 0;

    virtual void *RawPtr(MetaObject *obj) const = 0;
    virtual const void *RawPtr(const MetaObject *obj) const = 0;

    virtual size_t HashValue(const MetaObject *object) const = 0;

    template <typename T>
    MetaTypedProperty< typename MetaTypeTraits<T>::wrapper_type > *Cast() {
        return checked_cast<MetaTypedProperty< typename MetaTypeTraits<T>::wrapper_type > *>(this);
    }

    template <typename T>
    const MetaTypedProperty< typename MetaTypeTraits<T>::wrapper_type > *Cast() const {
        return checked_cast<const MetaTypedProperty< typename MetaTypeTraits<T>::wrapper_type > *>(this);
    }

    template <typename T>
    MetaTypedProperty< typename MetaTypeTraits<T>::wrapper_type > *As() {
        return dynamic_cast<MetaTypedProperty< typename MetaTypeTraits<T>::wrapper_type > *>(this);
    }

    template <typename T>
    const MetaTypedProperty< typename MetaTypeTraits<T>::wrapper_type > *As() const {
        return dynamic_cast<const MetaTypedProperty< typename MetaTypeTraits<T>::wrapper_type > *>(this);
    }

protected:
    MetaPropertyName _name;
    Flags _attributes;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class MetaTypedProperty : public MetaProperty {
public:
    typedef MetaType<T> meta_type;
    static_assert(meta_type::TypeId, "T is not supported by RTTI");

    MetaTypedProperty(const MetaPropertyName& name, Flags attributes);
    virtual ~MetaTypedProperty();

    virtual MetaTypeInfo TypeInfo() const override;

    virtual void GetCopy(const MetaObject *object, T& dst) const = 0;
    virtual void GetMove(MetaObject *object, T& dst) const = 0;

    virtual void SetMove(MetaObject *object, T&& src) const = 0;
    virtual void SetCopy(MetaObject *object, const T& src) const = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Accessor = MetaPropertyAccessor<T> >
class MetaWrappedProperty : public MetaTypedProperty< typename MetaTypeTraits<T>::wrapper_type > {
public:
    static_assert(  std::is_base_of<MetaPropertyAccessor<T>, _Accessor >::value,
                    "_Accessor must be derived from MetaPropertyAccessor<T>");

    typedef MetaTypeTraits< T > meta_type_traits;

    typedef typename meta_type_traits::meta_type meta_type;
    typedef typename meta_type_traits::wrapped_type wrapped_type;
    typedef typename meta_type_traits::wrapper_type wrapper_type;

    typedef MetaTypedAtom< wrapper_type > typed_atom_type;
    typedef MetaTypedProperty< wrapper_type > typed_property_type;

    typedef _Accessor accessor_type;

    MetaWrappedProperty(const MetaPropertyName& name, Flags attributes, accessor_type&& accessor);
    virtual ~MetaWrappedProperty();

    virtual const IMetaTypeVirtualTraits *Traits() const override { return meta_type_traits::VirtualTraits(); }

    virtual void GetCopy(const MetaObject *object, wrapper_type& dst) const override;
    virtual void GetMove(MetaObject *object, wrapper_type& dst) const override;

    virtual void SetMove(MetaObject *object, wrapper_type&& src) const override;
    virtual void SetCopy(MetaObject *object, const wrapper_type& src) const override;

    virtual MetaAtom *WrapMove(MetaObject *src) const override;
    virtual MetaAtom *WrapCopy(const MetaObject *src) const override;

    virtual bool UnwrapMove(MetaObject *dst, MetaAtom *src) const override;
    virtual bool UnwrapCopy(MetaObject *dst, const MetaAtom *src) const override;

    virtual void MoveTo(MetaObject *object, MetaAtom *atom) const override;
    virtual void CopyTo(const MetaObject *object, MetaAtom *atom) const override;

    virtual void MoveFrom(MetaObject *object, MetaAtom *atom) const override;
    virtual void CopyFrom(MetaObject *object, const MetaAtom *atom) const override;

    virtual void Move(MetaObject *dst, MetaObject *src) const override;
    virtual void Copy(MetaObject *dst, const MetaObject *src) const override;

    virtual bool Equals(const MetaObject *lhs, const MetaObject *rhs) const override;

    virtual void *RawPtr(MetaObject *obj) const override;
    virtual const void *RawPtr(const MetaObject *obj) const override;

    virtual size_t HashValue(const MetaObject *object) const override;

    SINGLETON_POOL_ALLOCATED_DECL(MetaWrappedProperty)

private:
    accessor_type _accessor;
};
//----------------------------------------------------------------------------
template <typename _Class, typename T>
MetaWrappedProperty<T, MetaFieldAccessor<T> > *MakeProperty(
    const MetaPropertyName& name, MetaProperty::Flags attributes, T _Class::* field) {
    return new MetaWrappedProperty<T, MetaFieldAccessor<T> >(
        name, attributes,
        MakeFieldAccessor(field) );
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
MetaWrappedProperty<T, MetaFunctionAccessor<T, _Class> > *MakeProperty(
    const MetaPropertyName& name, MetaProperty::Flags attributes,
    std::function<T&(_Class *)>&& getter,
    std::function<void(_Class *, T&&)>&& mover,
    std::function<void(_Class *, const T&)>&& setter) {
    return new MetaWrappedProperty<T, MetaFunctionAccessor<T, _Class> >(
        name, attributes,
        MakeFunctionAccessor(std::move(getter), std::move(mover), std::move(setter)) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

#include "Core/RTTI/Property/MetaProperty-inl.h"
