#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/MetaPropertyAccessor.h"
#include "Core.RTTI/MetaType.h"

#include "Core.RTTI/MetaAtom.h"
#include "Core.RTTI/MetaType.h"
#include "Core.RTTI/MetaTypeTraits.h"
#include "Core.RTTI/MetaTypeVirtualTraits.h"

#include "Core/Allocator/PoolAllocator.h"

#include <type_traits>

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMetaObject;
template <typename T>
class TMetaTypedProperty;
//----------------------------------------------------------------------------
FWD_UNIQUEPTR(MetaProperty);
class FMetaProperty {
public:
    enum EFlags {
        Public       = 1<<0,
        Protected    = 1<<1,
        Private      = 1<<2,
        ReadOnly     = 1<<3,
        Deprecated   = 1<<4,
        Dynamic      = 1<<5,
    };

    FMetaProperty(const FName& name, EFlags attributes);
    virtual ~FMetaProperty();

    FMetaProperty(const FMetaProperty&) = delete;
    FMetaProperty& operator =(const FMetaProperty&) = delete;

    const FName& Name() const { return _name; }
    EFlags Attributes() const { return _attributes; }

    bool IsPublic()     const { return Meta::HasFlag(_attributes, Public); }
    bool IsProtected()  const { return Meta::HasFlag(_attributes, Protected); }
    bool IsPrivate()    const { return Meta::HasFlag(_attributes, Private); }
    bool IsReadOnly()   const { return Meta::HasFlag(_attributes, ReadOnly); }
    bool IsDeprecated() const { return Meta::HasFlag(_attributes, Deprecated); }
    bool IsDynamic()    const { return Meta::HasFlag(_attributes, Dynamic); }
    bool IsWritable()   const { return false == (IsReadOnly() || IsDeprecated()); }

    virtual FMetaTypeInfo TypeInfo() const = 0;
    virtual const IMetaTypeVirtualTraits *Traits() const = 0;

    virtual bool IsDefaultValue(const FMetaObject *object) const = 0;

    virtual FMetaAtom *WrapMove(FMetaObject *src) const = 0;
    virtual FMetaAtom *WrapCopy(const FMetaObject *src) const = 0;

    virtual bool UnwrapMove(FMetaObject *dst, FMetaAtom *src) const = 0;
    virtual bool UnwrapCopy(FMetaObject *dst, const FMetaAtom *src) const = 0;

    virtual void MoveTo(FMetaObject *object, FMetaAtom *atom) const = 0;
    virtual void CopyTo(const FMetaObject *object, FMetaAtom *atom) const = 0;

    virtual void MoveFrom(FMetaObject *object, FMetaAtom *atom) const = 0;
    virtual void CopyFrom(FMetaObject *object, const FMetaAtom *atom) const = 0;

    virtual void Move(FMetaObject *dst, FMetaObject *src) const = 0;
    virtual void Copy(FMetaObject *dst, const FMetaObject *src) const = 0;

    virtual void Swap(FMetaObject *lhs, FMetaObject *rhs) const = 0;

    virtual bool Equals(const FMetaObject *lhs, const FMetaObject *rhs) const = 0;
    virtual bool DeepEquals(const FMetaObject *lhs, const FMetaObject *rhs) const = 0;

    virtual void *RawPtr(FMetaObject *obj) const = 0;
    virtual const void *RawPtr(const FMetaObject *obj) const = 0;

    virtual size_t HashValue(const FMetaObject *object) const = 0;

    template <typename T>
    TMetaTypedProperty< typename TMetaTypeTraits<T>::wrapper_type > *Cast() {
        return checked_cast<TMetaTypedProperty< typename TMetaTypeTraits<T>::wrapper_type > *>(this);
    }

    template <typename T>
    const TMetaTypedProperty< typename TMetaTypeTraits<T>::wrapper_type > *Cast() const {
        return checked_cast<const TMetaTypedProperty< typename TMetaTypeTraits<T>::wrapper_type > *>(this);
    }

    template <typename T>
    TMetaTypedProperty< typename TMetaTypeTraits<T>::wrapper_type > *As() {
        return dynamic_cast<TMetaTypedProperty< typename TMetaTypeTraits<T>::wrapper_type > *>(this);
    }

    template <typename T>
    const TMetaTypedProperty< typename TMetaTypeTraits<T>::wrapper_type > *As() const {
        return dynamic_cast<const TMetaTypedProperty< typename TMetaTypeTraits<T>::wrapper_type > *>(this);
    }

protected:
    FName _name;
    EFlags _attributes;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TMetaTypedProperty : public FMetaProperty {
public:
    typedef TMetaType<T> meta_type;
    static_assert(meta_type::TypeId, "T is not supported by RTTI");

    TMetaTypedProperty(const FName& name, EFlags attributes);
    virtual ~TMetaTypedProperty();

    virtual FMetaTypeInfo TypeInfo() const override;

    virtual void GetCopy(const FMetaObject *object, T& dst) const = 0;
    virtual void GetMove(FMetaObject *object, T& dst) const = 0;

    virtual void SetMove(FMetaObject *object, T&& src) const = 0;
    virtual void SetCopy(FMetaObject *object, const T& src) const = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Accessor >
class TMetaWrappedProperty :
    public TMetaTypedProperty< typename TMetaTypeTraits<T>::wrapper_type >
,   private _Accessor {
public:
    typedef TMetaTypeTraits< T > meta_type_traits;

    typedef typename meta_type_traits::meta_type meta_type;
    typedef typename meta_type_traits::wrapped_type wrapped_type;
    typedef typename meta_type_traits::wrapper_type wrapper_type;

    typedef TMetaTypedAtom< wrapper_type > typed_atom_type;
    typedef TMetaTypedProperty< wrapper_type > typed_property_type;

    typedef _Accessor accessor_type;

    using FMetaProperty::EFlags;

    TMetaWrappedProperty(const FName& name, EFlags attributes, accessor_type&& accessor);
    virtual ~TMetaWrappedProperty();

    virtual const IMetaTypeVirtualTraits *Traits() const override { return meta_type_traits::VirtualTraits(); }

    virtual bool IsDefaultValue(const FMetaObject *object) const override;

    virtual void GetCopy(const FMetaObject *object, wrapper_type& dst) const override;
    virtual void GetMove(FMetaObject *object, wrapper_type& dst) const override;

    virtual void SetMove(FMetaObject *object, wrapper_type&& src) const override;
    virtual void SetCopy(FMetaObject *object, const wrapper_type& src) const override;

    virtual FMetaAtom *WrapMove(FMetaObject *src) const override;
    virtual FMetaAtom *WrapCopy(const FMetaObject *src) const override;

    virtual bool UnwrapMove(FMetaObject *dst, FMetaAtom *src) const override;
    virtual bool UnwrapCopy(FMetaObject *dst, const FMetaAtom *src) const override;

    virtual void MoveTo(FMetaObject *object, FMetaAtom *atom) const override;
    virtual void CopyTo(const FMetaObject *object, FMetaAtom *atom) const override;

    virtual void MoveFrom(FMetaObject *object, FMetaAtom *atom) const override;
    virtual void CopyFrom(FMetaObject *object, const FMetaAtom *atom) const override;

    virtual void Move(FMetaObject *dst, FMetaObject *src) const override;
    virtual void Copy(FMetaObject *dst, const FMetaObject *src) const override;

    virtual void Swap(FMetaObject *lhs, FMetaObject *rhs) const override;

    virtual bool Equals(const FMetaObject *lhs, const FMetaObject *rhs) const override;
    virtual bool DeepEquals(const FMetaObject *lhs, const FMetaObject *rhs) const override;

    virtual void *RawPtr(FMetaObject *obj) const override;
    virtual const void *RawPtr(const FMetaObject *obj) const override;

    virtual size_t HashValue(const FMetaObject *object) const override;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    using FMetaProperty::_name;
    using FMetaProperty::_attributes;
};
//----------------------------------------------------------------------------
template <typename T, typename _Class>
TMetaWrappedProperty<T, TMetaFieldAccessor<T> > *MakeProperty(
    const FName& name, FMetaProperty::EFlags attributes, T _Class::* field) {
    return new TMetaWrappedProperty<T, TMetaFieldAccessor<T> >(
        name, attributes,
        MakeFieldAccessor(field) );
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
TMetaWrappedProperty<T, TMetaMemberAccessor<T, _Class> > *MakeProperty(
    const FName& name, FMetaProperty::EFlags attributes,
    const T& (_Class::* getter)() const,
    void (_Class::* setter)(const T& ) ) {
    return new TMetaWrappedProperty<T, TMetaMemberAccessor<T, _Class> >(
        name, attributes,
        MakeMemberAccessor(getter, setter) );
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
TMetaWrappedProperty<T, TMetaDelegateAccessor<T, _Class> > *MakeProperty(
    const FName& name, FMetaProperty::EFlags attributes,
    TDelegate<T&   (*)(_Class* )>&& getter,
    TDelegate<void (*)(_Class*, T&& )>&& mover,
    TDelegate<void (*)(_Class*, const T& )>&& setter ) {
    return new TMetaWrappedProperty<T, TMetaDelegateAccessor<T, _Class> >(
        name, attributes,
        MakeDelegateAccessor(std::move(getter), std::move(mover), std::move(setter)) );
}
//----------------------------------------------------------------------------
template <typename T, typename _Class>
TMetaWrappedProperty<T, TMetaDeprecatedAccessor<T, _Class> > *MakeDeprecatedProperty(
    const FName& name, FMetaProperty::EFlags attributes ) {
    return new TMetaWrappedProperty<T, TMetaDeprecatedAccessor<T, _Class> >(
        name, FMetaProperty::EFlags(attributes | FMetaProperty::Deprecated | FMetaProperty::ReadOnly),
        MakeDeprecatedAccessor<T, _Class>() );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

#include "Core.RTTI/MetaProperty-inl.h"
