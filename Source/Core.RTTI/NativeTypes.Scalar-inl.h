#pragma once

#include "Core.RTTI/NativeTypes.h"

#include "Core/Maths/Packing_fwd.h"

namespace Core {
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
    CORE_RTTI_API PTypeTraits Traits(Meta::TType<T>) noexcept; \
    inline constexpr FTypeId NativeTypeId(Meta::TType<T>) noexcept { return FTypeId(_TypeId); }
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
CORE_RTTI_API PTypeTraits MakeTraits(ENativeType nativeType);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// RTTI support for enums
//----------------------------------------------------------------------------
template <typename _Enum>
PTypeTraits Traits(Meta::TType< _Enum >, Meta::TEnableIf< std::is_enum_v<_Enum> >* = nullptr) noexcept {
    // !!! BEWARE OF THE DOG !!!
    // *ALWAYS* specify the size of your enums wrapped in RTTI !
    return MakeTraits<typename TIntegral<_Enum>::type>();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// RTTI support for packed data
//----------------------------------------------------------------------------
CORE_RTTI_API PTypeTraits Traits(Meta::TType<byten>);
CORE_RTTI_API PTypeTraits Traits(Meta::TType<shortn>);
CORE_RTTI_API PTypeTraits Traits(Meta::TType<wordn>);
CORE_RTTI_API PTypeTraits Traits(Meta::TType<ubyten>);
CORE_RTTI_API PTypeTraits Traits(Meta::TType<ushortn>);
CORE_RTTI_API PTypeTraits Traits(Meta::TType<uwordn>);
CORE_RTTI_API PTypeTraits Traits(Meta::TType<FHalfFloat>);
CORE_RTTI_API PTypeTraits Traits(Meta::TType<UX10Y10Z10W2N>);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TObjectTraits<T>
//----------------------------------------------------------------------------
CORE_RTTI_API bool DeepEqualsObject(const PMetaObject& lhs, const PMetaObject& rhs);
CORE_RTTI_API void DeepCopyObject(const IScalarTraits& self, const PMetaObject& src, PMetaObject& dst);
CORE_RTTI_API bool PromoteCopyObject(const IScalarTraits& self, const PMetaObject& src, const FAtom& dst);
CORE_RTTI_API bool PromoteMoveObject(const IScalarTraits& self, PMetaObject& src, const FAtom& dst);
CORE_RTTI_API void* CastObject(const IScalarTraits& self, PMetaObject& data, const PTypeTraits& dst);
//----------------------------------------------------------------------------
template <typename T>
class TObjectTraits final : public TBaseTypeTraits<PMetaObject, TBaseScalarTraits<PMetaObject>> {
    using base_traits = TBaseTypeTraits<PMetaObject, TBaseScalarTraits<PMetaObject>>;
public: // ITypeTraits
    virtual const FMetaClass* ObjectClass() const override final;

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
PTypeTraits Traits(Meta::TType< TRefPtr<_Class> >) noexcept {
    return PTypeTraits::Make< TObjectTraits<Meta::TDecay<_Class>> >();
}
//----------------------------------------------------------------------------
template <typename T>
const FMetaClass* TObjectTraits<T>::ObjectClass() const {
    return RTTI::MetaClass<T>();
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
        MetaClassName(ObjectClass()),
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
    Assert(data);

    return visitor->Visit(static_cast<const IScalarTraits*>(this), *reinterpret_cast<PMetaObject*>(data));
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
} //!namespace Core
