#pragma once

#include "RTTI/NativeTypes.h"
#include "RTTI/NativeTraits.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TBaseScalarTraits<T>
//----------------------------------------------------------------------------
template <typename T>
class TBaseScalarTraits : public IScalarTraits {
protected:
    using IScalarTraits::IScalarTraits;

public: // ITypeTraits
    virtual bool Equals(const void* lhs, const void* rhs) const NOEXCEPT override final;

    virtual PTypeTraits CommonType(const PTypeTraits& other) const NOEXCEPT override final;

    virtual hash_t HashValue(const void* data) const NOEXCEPT override final;
};
//----------------------------------------------------------------------------
template <typename T>
bool TBaseScalarTraits<T>::Equals(const void* lhs, const void* rhs) const NOEXCEPT {
    Assert(lhs);
    Assert(rhs);

    return (*static_cast<const T*>(lhs) == *static_cast<const T*>(rhs));
}
//----------------------------------------------------------------------------
template <typename T>
PTypeTraits TBaseScalarTraits<T>::CommonType(const PTypeTraits& other) const NOEXCEPT {
    return MakeCommonType<T>(other);
}
//----------------------------------------------------------------------------
template <typename T>
hash_t TBaseScalarTraits<T>::HashValue(const void* data) const NOEXCEPT {
    Assert(data);

    return hash_tuple(TypeId(), *static_cast<const T*>(data));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TEnumTraits<T>
//----------------------------------------------------------------------------
template <typename _Enum/*, class = Meta::TEnableIf<std::is_enum_v<_Enum>> */>
using TEnumOrd = std::underlying_type_t<_Enum>;
//----------------------------------------------------------------------------
PPE_RTTI_API bool PromoteEnum(const IScalarTraits& self, FMetaEnumOrd src, const FAtom& dst);
//----------------------------------------------------------------------------
template <typename T>
class TEnumTraits final : public TBaseTypeTraits<TEnumOrd<T>, TBaseScalarTraits<TEnumOrd<T>> > {
    using base_traits = TBaseTypeTraits<TEnumOrd<T>, TBaseScalarTraits<TEnumOrd<T>> >;

    public: // ITypeTraits
    using base_traits::base_traits;

    virtual const FMetaEnum* EnumClass() const NOEXCEPT override final { return RTTI::MetaEnum<T>(); }
    virtual const FMetaClass* ObjectClass() const NOEXCEPT override final { return nullptr; }

    public: // ITypeTraits
    virtual FStringView TypeName() const override final;

    virtual bool IsDefaultValue(const void* data) const NOEXCEPT override final;
    virtual void ResetToDefaultValue(void* data) const override final;

    virtual bool DeepEquals(const void* lhs, const void* rhs) const override final;
    virtual void DeepCopy(const void* src, void* dst) const override final;

    virtual bool PromoteCopy(const void* src, const FAtom& dst) const override final;
    virtual bool PromoteMove(void* src, const FAtom& dst) const NOEXCEPT override final;

    virtual void* Cast(void* data, const PTypeTraits& dst) const override final;

    virtual bool Accept(IAtomVisitor* visitor, void* data) const override final;
};
//----------------------------------------------------------------------------
template <typename _Enum, class = Meta::TEnableIf< std::is_enum_v<_Enum> > >
CONSTEXPR PTypeInfos RTTI_TypeInfos(TTypeTag< _Enum >) {
    return MakeEnumTypeInfos< TEnumOrd<_Enum>, NativeTypeId< TEnumOrd<_Enum> >() >;
}
//----------------------------------------------------------------------------
template <typename _Enum>
CONSTEXPR PTypeTraits RTTI_Traits(TTypeTag< _Enum >, Meta::TEnableIf< std::is_enum_v<_Enum> >* = nullptr) {
    return MakeStaticType< TEnumTraits<_Enum>, _Enum >();
}
//----------------------------------------------------------------------------
template <typename T>
FStringView TEnumTraits<T>::TypeName() const {
    return MetaEnumName(TEnumTraits<T>::EnumClass());
}
//----------------------------------------------------------------------------
template <typename T>
bool TEnumTraits<T>::DeepEquals(const void* lhs, const void* rhs) const {
    return (*static_cast<const TEnumOrd<T>*>(lhs) ==
            *static_cast<const TEnumOrd<T>*>(rhs) );
}
//----------------------------------------------------------------------------
template <typename T>
void TEnumTraits<T>::DeepCopy(const void* src, void* dst) const {
    *static_cast<TEnumOrd<T>*>(dst) =
        *static_cast<const TEnumOrd<T>*>(src);
}
//----------------------------------------------------------------------------
template <typename T>
bool TEnumTraits<T>::PromoteCopy(const void* src, const FAtom& dst) const {
    return (not base_traits::BasePromoteCopy(src, dst)
        ? PromoteEnum(*this, i64(*static_cast<const TEnumOrd<T>*>(src)), dst)
        : true);
}
//----------------------------------------------------------------------------
template <typename T>
bool TEnumTraits<T>::PromoteMove(void* src, const FAtom& dst) const NOEXCEPT {
    return (not base_traits::BasePromoteMove(src, dst)
        ? PromoteEnum(*this, i64(*static_cast<TEnumOrd<T>*>(src)), dst)
        : true);
}
//----------------------------------------------------------------------------
template <typename T>
void* TEnumTraits<T>::Cast(void* data, const PTypeTraits& dst) const {
    return base_traits::BaseCast(data, dst);
}
//----------------------------------------------------------------------------
template <typename T>
bool TEnumTraits<T>::Accept(IAtomVisitor* visitor, void* data) const {
    Assert(visitor);
    Assert(data);

    return AtomVisit(*visitor,
        static_cast<const IScalarTraits*>(this),
        *static_cast<TEnumOrd<T>*>(data) );
}
//----------------------------------------------------------------------------
template <typename T>
bool TEnumTraits<T>::IsDefaultValue(const void* data) const NOEXCEPT {
    Assert(data);

    const auto defaultValue = TEnumOrd<T>( MetaEnumDefaultValue(TEnumTraits<T>::EnumClass()) );
    return (*static_cast<const TEnumOrd<T>*>(data) == defaultValue);
}
//----------------------------------------------------------------------------
template <typename T>
void TEnumTraits<T>::ResetToDefaultValue(void* data) const {
    Assert(data);

    const auto defaultValue = TEnumOrd<T>( MetaEnumDefaultValue(TEnumTraits<T>::EnumClass()) );
    *static_cast<TEnumOrd<T>*>(data) = defaultValue;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TObjectTraits<T>
//----------------------------------------------------------------------------
PPE_RTTI_API bool DeepEqualsObject(const PMetaObject& lhs, const PMetaObject& rhs);
PPE_RTTI_API void DeepCopyObject(const IScalarTraits& self, const PMetaObject& src, PMetaObject& dst);
PPE_RTTI_API bool PromoteCopyObject(const IScalarTraits& self, const PMetaObject& src, const FAtom& dst);
PPE_RTTI_API bool PromoteMoveObject(const IScalarTraits& self, PMetaObject& src, const FAtom& dst) NOEXCEPT;
PPE_RTTI_API void* CastObject(const IScalarTraits& self, PMetaObject& data, const PTypeTraits& dst) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T>
class TObjectTraits final : public TBaseTypeTraits<PMetaObject, TBaseScalarTraits<PMetaObject>> {
    using base_traits = TBaseTypeTraits<PMetaObject, TBaseScalarTraits<PMetaObject>>;

    public: // IScalarTraits
    virtual const FMetaEnum* EnumClass() const NOEXCEPT override final { return nullptr; }
    virtual const FMetaClass* ObjectClass() const NOEXCEPT override final { return RTTI::MetaClass<T>(); }

    public: // ITypeTraits
    using base_traits::base_traits;

    virtual FStringView TypeName() const override final;

    virtual bool IsDefaultValue(const void* data) const NOEXCEPT override final;
    virtual void ResetToDefaultValue(void* data) const override final;

    virtual bool DeepEquals(const void* lhs, const void* rhs) const override final;
    virtual void DeepCopy(const void* src, void* dst) const override final;

    virtual bool PromoteCopy(const void* src, const FAtom& dst) const override final;
    virtual bool PromoteMove(void* src, const FAtom& dst) const NOEXCEPT override final;

    virtual void* Cast(void* data, const PTypeTraits& dst) const override final;

    virtual bool Accept(IAtomVisitor* visitor, void* data) const override final;
};
//----------------------------------------------------------------------------
template <typename _Class, typename = Meta::TEnableIf< std::is_base_of_v<FMetaObject, _Class> > >
CONSTEXPR PTypeInfos RTTI_TypeInfos(TTypeTag< TRefPtr<_Class> >) {
    return MakeObjectTypeInfos< TRefPtr<_Class>, FTypeId(ENativeType::MetaObject) >;
}
//----------------------------------------------------------------------------
template <typename _Class, typename = Meta::TEnableIf< std::is_base_of_v<FMetaObject, _Class> > >
CONSTEXPR PTypeTraits RTTI_Traits(TTypeTag< TRefPtr<_Class> >) {
    return MakeStaticType< TObjectTraits<Meta::TDecay<_Class>>, TRefPtr<_Class> >();
}
//----------------------------------------------------------------------------
template <typename T>
FStringView TObjectTraits<T>::TypeName() const {
    return MetaClassName(TObjectTraits<T>::ObjectClass());
}
//----------------------------------------------------------------------------
template <typename T>
bool TObjectTraits<T>::DeepEquals(const void* lhs, const void* rhs) const {
    return DeepEqualsObject(
        *static_cast<const PMetaObject*>(lhs),
        *static_cast<const PMetaObject*>(rhs) );
}
//----------------------------------------------------------------------------
template <typename T>
void TObjectTraits<T>::DeepCopy(const void* src, void* dst) const {
    DeepCopyObject(*this,
        *static_cast<const PMetaObject*>(src),
        *static_cast<PMetaObject*>(dst) );
}
//----------------------------------------------------------------------------
template <typename T>
bool TObjectTraits<T>::PromoteCopy(const void* src, const FAtom& dst) const {
    return (not BasePromoteCopy(src, dst)
        ? PromoteCopyObject(*this, *static_cast<const PMetaObject*>(src), dst)
        : true );
}
//----------------------------------------------------------------------------
template <typename T>
bool TObjectTraits<T>::PromoteMove(void* src, const FAtom& dst) const NOEXCEPT {
    return (not BasePromoteMove(src, dst)
        ? PromoteMoveObject(*this, *static_cast<PMetaObject*>(src), dst)
        : true );
}
//----------------------------------------------------------------------------
template <typename T>
void* TObjectTraits<T>::Cast(void* data, const PTypeTraits& dst) const {
    void* p = BaseCast(data, dst);
    if (not p)
        p = CastObject(*this, *static_cast<PMetaObject*>(data), dst);

    return p;
}
//----------------------------------------------------------------------------
template <typename T>
bool TObjectTraits<T>::Accept(IAtomVisitor* visitor, void* data) const {
    Assert(visitor);
    Assert(data);

    return AtomVisit(*visitor,
        static_cast<const IScalarTraits*>(this),
        *static_cast<PMetaObject*>(data) );
}
//----------------------------------------------------------------------------
template <typename T>
bool TObjectTraits<T>::IsDefaultValue(const void* data) const NOEXCEPT {
    Assert(data);

    const PMetaObject& pobj = (*static_cast<const PMetaObject*>(data));
    return(nullptr == pobj);
}
//----------------------------------------------------------------------------
template <typename T>
void TObjectTraits<T>::ResetToDefaultValue(void* data) const {
    Assert(data);

    PMetaObject& pobj = (*static_cast<PMetaObject*>(data));
    pobj.reset();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
