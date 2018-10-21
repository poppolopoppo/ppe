#pragma once

#include "RTTI/NativeTypes.h"

#include "Maths/Packing_fwd.h"

namespace PPE {
struct FGuid;
class PPE_CORE_API FTimestamp;
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TBaseScalarTraits<T>
//----------------------------------------------------------------------------
template <typename T>
class TBaseScalarTraits : public IScalarTraits {
public: // ITypeTraits
    virtual bool Equals(const void* lhs, const void* rhs) const override final;
    virtual hash_t HashValue(const void* data) const override final;

    virtual bool PromoteCopy(const void* , const FAtom& ) const override /*final*/ { return false; }
    virtual bool PromoteMove(void* , const FAtom& ) const override /*final*/ { return false; }
};
//----------------------------------------------------------------------------
template <typename T>
bool TBaseScalarTraits<T>::Equals(const void* lhs, const void* rhs) const {
    Assert(lhs);
    Assert(rhs);

    return (*reinterpret_cast<const T*>(lhs) == *reinterpret_cast<const T*>(rhs));
}
//----------------------------------------------------------------------------
template <typename T>
hash_t TBaseScalarTraits<T>::HashValue(const void* data) const {
    Assert(data);

    return hash_tuple(TypeId(), *reinterpret_cast<const T*>(data));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// ENatypeType traits
//----------------------------------------------------------------------------
#define DECL_RTTI_NATIVETYPE_TRAITS(_Name, T, _TypeId) \
    PPE_RTTI_API PTypeTraits Traits(Meta::TType<T>) NOEXCEPT; \
    inline constexpr FTypeId NativeTypeId(Meta::TType<T>) NOEXCEPT { return FTypeId(_TypeId); }
FOREACH_RTTI_NATIVETYPES(DECL_RTTI_NATIVETYPE_TRAITS)
#undef DECL_RTTI_NATIVETYPE_TRAITS
//----------------------------------------------------------------------------
enum class ENativeType : FTypeId {
    Invalid = 0,
#define DECL_RTTI_NATIVETYPE_ENUM(_Name, T, _TypeId) _Name = _TypeId,
FOREACH_RTTI_NATIVETYPES(DECL_RTTI_NATIVETYPE_ENUM)
#undef DECL_RTTI_NATIVETYPE_ENUM
    __Count
}; //!enum class ENativeType
//----------------------------------------------------------------------------
template <typename T>
constexpr FTypeId NativeTypeId() {
    return NativeTypeId(Meta::TType<T>{});
}
//----------------------------------------------------------------------------
PPE_RTTI_API PTypeTraits MakeTraits(ENativeType nativeType);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// RTTI support for packed data
//----------------------------------------------------------------------------
PPE_RTTI_API PTypeTraits Traits(Meta::TType<byten>);
PPE_RTTI_API PTypeTraits Traits(Meta::TType<shortn>);
PPE_RTTI_API PTypeTraits Traits(Meta::TType<wordn>);
PPE_RTTI_API PTypeTraits Traits(Meta::TType<ubyten>);
PPE_RTTI_API PTypeTraits Traits(Meta::TType<ushortn>);
PPE_RTTI_API PTypeTraits Traits(Meta::TType<uwordn>);
PPE_RTTI_API PTypeTraits Traits(Meta::TType<FHalfFloat>);
PPE_RTTI_API PTypeTraits Traits(Meta::TType<UX10Y10Z10W2N>);
PPE_RTTI_API PTypeTraits Traits(Meta::TType<FGuid>);
PPE_RTTI_API PTypeTraits Traits(Meta::TType<FTimestamp>);
PPE_RTTI_API PTypeTraits Traits(Meta::TType<u128>);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TEnumTraits<T>
//----------------------------------------------------------------------------
template <typename _Enum/*, class = Meta::TEnableIf<std::is_enum_v<_Enum>> */>
using TEnum_t = std::underlying_type_t<_Enum>;
//----------------------------------------------------------------------------
PPE_RTTI_API bool PromoteEnum(const IScalarTraits& self, i64 src, const FAtom& dst);
//----------------------------------------------------------------------------
template <typename T>
class TEnumTraits final : public TBaseTypeTraits<TEnum_t<T>, TBaseScalarTraits<TEnum_t<T>> > {
    using base_traits = TBaseTypeTraits<TEnum_t<T>, TBaseScalarTraits<TEnum_t<T>> >;

public: // ITypeTraits
    virtual const FMetaEnum* EnumClass() const override final { return RTTI::MetaEnum<T>(); }
    virtual const FMetaClass* ObjectClass() const override final { return nullptr; }

public: // ITypeTraits
    virtual FTypeId TypeId() const override final;
    virtual ETypeFlags TypeFlags() const override final;
    virtual FTypeInfos TypeInfos() const override final;

    virtual bool IsDefaultValue(const void* data) const override final;
    virtual void ResetToDefaultValue(void* data) const override final;

    virtual bool DeepEquals(const void* lhs, const void* rhs) const override final;
    virtual void DeepCopy(const void* src, void* dst) const override final;

    virtual bool PromoteCopy(const void* src, const FAtom& dst) const override final;
    virtual bool PromoteMove(void* src, const FAtom& dst) const override final;

    virtual void* Cast(void* data, const PTypeTraits& dst) const override final;

    virtual bool Accept(IAtomVisitor* visitor, void* data) const override final;
};
//----------------------------------------------------------------------------
template <typename _Enum>
PTypeTraits Traits(Meta::TType< _Enum >, Meta::TEnableIf< std::is_enum_v<_Enum> >* = nullptr) NOEXCEPT {
    return PTypeTraits::Make< TEnumTraits<_Enum> >();
}
//----------------------------------------------------------------------------
template <typename T>
FTypeId TEnumTraits<T>::TypeId() const {
    return NativeTypeId<TEnum_t<T>>();
}
//----------------------------------------------------------------------------
template <typename T>
ETypeFlags TEnumTraits<T>::TypeFlags() const {
    return (ETypeFlags::POD |
            ETypeFlags::Scalar |
            ETypeFlags::Enum |
            ETypeFlags::Native |
            ETypeFlags::TriviallyDestructible );
}
//----------------------------------------------------------------------------
template <typename T>
FTypeInfos TEnumTraits<T>::TypeInfos() const {
    return FTypeInfos(
        MetaEnumName(TEnumTraits<T>::EnumClass()),
        TEnumTraits<T>::TypeId(),
        TEnumTraits<T>::TypeFlags(),
        sizeof(T) );
}
//----------------------------------------------------------------------------
template <typename T>
bool TEnumTraits<T>::DeepEquals(const void* lhs, const void* rhs) const {
    return (*reinterpret_cast<const TEnum_t<T>*>(lhs) ==
            *reinterpret_cast<const TEnum_t<T>*>(rhs) );
}
//----------------------------------------------------------------------------
template <typename T>
void TEnumTraits<T>::DeepCopy(const void* src, void* dst) const {
    *reinterpret_cast<TEnum_t<T>*>(dst) =
        *reinterpret_cast<const TEnum_t<T>*>(src);
}
//----------------------------------------------------------------------------
template <typename T>
bool TEnumTraits<T>::PromoteCopy(const void* src, const FAtom& dst) const {
    return (not base_traits::PromoteCopy(src, dst)
        ? PromoteEnum(*this, i64(*reinterpret_cast<const TEnum_t<T>*>(src)), dst)
        : true);
}
//----------------------------------------------------------------------------
template <typename T>
bool TEnumTraits<T>::PromoteMove(void* src, const FAtom& dst) const {
    return (not base_traits::PromoteMove(src, dst)
        ? PromoteEnum(*this, i64(*reinterpret_cast<TEnum_t<T>*>(src)), dst)
        : true);
}
//----------------------------------------------------------------------------
template <typename T>
void* TEnumTraits<T>::Cast(void* data, const PTypeTraits& dst) const {
    return base_traits::Cast(data, dst);
}
//----------------------------------------------------------------------------
template <typename T>
bool TEnumTraits<T>::Accept(IAtomVisitor* visitor, void* data) const {
    Assert(visitor);
    Assert(data);

    return AtomVisit(*visitor,
        static_cast<const IScalarTraits*>(this),
        *reinterpret_cast<TEnum_t<T>*>(data) );
}
//----------------------------------------------------------------------------
template <typename T>
bool TEnumTraits<T>::IsDefaultValue(const void* data) const {
    Assert(data);

    const auto defaultValue= TEnum_t<T>( MetaEnumDefaultValue(TEnumTraits<T>::EnumClass()) );
    return (*reinterpret_cast<const TEnum_t<T>*>(data) == defaultValue);
}
//----------------------------------------------------------------------------
template <typename T>
void TEnumTraits<T>::ResetToDefaultValue(void* data) const {
    Assert(data);

    const auto defaultValue = TEnum_t<T>(MetaEnumDefaultValue(TEnumTraits<T>::EnumClass()));
    *reinterpret_cast<TEnum_t<T>*>(data) = defaultValue;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TObjectTraits<T>
//----------------------------------------------------------------------------
PPE_RTTI_API bool DeepEqualsObject(const PMetaObject& lhs, const PMetaObject& rhs);
PPE_RTTI_API void DeepCopyObject(const IScalarTraits& self, const PMetaObject& src, PMetaObject& dst);
PPE_RTTI_API bool PromoteCopyObject(const IScalarTraits& self, const PMetaObject& src, const FAtom& dst);
PPE_RTTI_API bool PromoteMoveObject(const IScalarTraits& self, PMetaObject& src, const FAtom& dst);
PPE_RTTI_API void* CastObject(const IScalarTraits& self, PMetaObject& data, const PTypeTraits& dst);
//----------------------------------------------------------------------------
template <typename T>
class TObjectTraits final : public TBaseTypeTraits<PMetaObject, TBaseScalarTraits<PMetaObject>> {
    using base_traits = TBaseTypeTraits<PMetaObject, TBaseScalarTraits<PMetaObject>>;

public: // ITypeTraits
    virtual const FMetaEnum* EnumClass() const override final { return nullptr; }
    virtual const FMetaClass* ObjectClass() const override final { return RTTI::MetaClass<T>(); }

public: // ITypeTraits
    virtual FTypeId TypeId() const override final;
    virtual ETypeFlags TypeFlags() const override final;
    virtual FTypeInfos TypeInfos() const override final;

    virtual bool IsDefaultValue(const void* data) const override final;
    virtual void ResetToDefaultValue(void* data) const override final;

    virtual bool DeepEquals(const void* lhs, const void* rhs) const override final;
    virtual void DeepCopy(const void* src, void* dst) const override final;

    virtual bool PromoteCopy(const void* src, const FAtom& dst) const override final;
    virtual bool PromoteMove(void* src, const FAtom& dst) const override final;

    virtual void* Cast(void* data, const PTypeTraits& dst) const override final;

    virtual bool Accept(IAtomVisitor* visitor, void* data) const override final;
};
//----------------------------------------------------------------------------
template <typename _Class, typename = Meta::TEnableIf< std::is_base_of_v<FMetaObject, _Class> > >
PTypeTraits Traits(Meta::TType< TRefPtr<_Class> >) NOEXCEPT {
    return PTypeTraits::Make< TObjectTraits<Meta::TDecay<_Class>> >();
}
//----------------------------------------------------------------------------
template <typename T>
FTypeId TObjectTraits<T>::TypeId() const {
    return FTypeId(ENativeType::MetaObject);
}
//----------------------------------------------------------------------------
template <typename T>
ETypeFlags TObjectTraits<T>::TypeFlags() const {
    return (ETypeFlags::Scalar | ETypeFlags::Native);
}
//----------------------------------------------------------------------------
template <typename T>
FTypeInfos TObjectTraits<T>::TypeInfos() const {
    return FTypeInfos(
        MetaClassName(TObjectTraits<T>::ObjectClass()),
        TObjectTraits<T>::TypeId(),
        TObjectTraits<T>::TypeFlags(),
        sizeof(PMetaObject) );
}
//----------------------------------------------------------------------------
template <typename T>
bool TObjectTraits<T>::DeepEquals(const void* lhs, const void* rhs) const {
    return DeepEqualsObject(
        *reinterpret_cast<const PMetaObject*>(lhs),
        *reinterpret_cast<const PMetaObject*>(rhs) );
}
//----------------------------------------------------------------------------
template <typename T>
void TObjectTraits<T>::DeepCopy(const void* src, void* dst) const {
    DeepCopyObject(*this,
        *reinterpret_cast<const PMetaObject*>(src),
        *reinterpret_cast<PMetaObject*>(dst) );
}
//----------------------------------------------------------------------------
template <typename T>
bool TObjectTraits<T>::PromoteCopy(const void* src, const FAtom& dst) const {
    return (not base_traits::PromoteCopy(src, dst)
        ? PromoteCopyObject(*this, *reinterpret_cast<const PMetaObject*>(src), dst)
        : true );
}
//----------------------------------------------------------------------------
template <typename T>
bool TObjectTraits<T>::PromoteMove(void* src, const FAtom& dst) const {
    return (not base_traits::PromoteMove(src, dst)
        ? PromoteMoveObject(*this, *reinterpret_cast<PMetaObject*>(src), dst)
        : true );
}
//----------------------------------------------------------------------------
template <typename T>
void* TObjectTraits<T>::Cast(void* data, const PTypeTraits& dst) const {
    return CastObject(*this, *reinterpret_cast<PMetaObject*>(data), dst);
}
//----------------------------------------------------------------------------
template <typename T>
bool TObjectTraits<T>::Accept(IAtomVisitor* visitor, void* data) const {
    Assert(visitor);
    Assert(data);

    return AtomVisit(*visitor,
        static_cast<const IScalarTraits*>(this),
        *reinterpret_cast<PMetaObject*>(data) );
}
//----------------------------------------------------------------------------
template <typename T>
bool TObjectTraits<T>::IsDefaultValue(const void* data) const {
    Assert(data);

    const PMetaObject& pobj = (*reinterpret_cast<const PMetaObject*>(data));
    return(nullptr == pobj);
}
//----------------------------------------------------------------------------
template <typename T>
void TObjectTraits<T>::ResetToDefaultValue(void* data) const {
    Assert(data);

    PMetaObject& pobj = (*reinterpret_cast<PMetaObject*>(data));
    pobj.reset();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
