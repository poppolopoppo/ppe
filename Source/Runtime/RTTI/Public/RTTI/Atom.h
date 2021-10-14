#pragma once

#include "RTTI_fwd.h"

#include "RTTI/TypeTraits.h"

#include "IO/TextWriter_fwd.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FAtom {
public:
    CONSTEXPR FAtom() NOEXCEPT : _data(nullptr) {}
    CONSTEXPR FAtom(const void* data, const PTypeTraits& traits) NOEXCEPT
    :   _data(const_cast<void*>(data))
    ,   _traits(traits)
    {}

    bool Valid() const { return (!!_data); }
    PPE_FAKEBOOL_OPERATOR_DECL() { return Valid(); }

    void* Data() const { return _data; }
    const PTypeTraits& Traits() const { return _traits; }

    PPE_RTTI_API void* Cast(const PTypeTraits& to) const;

    FTypeId TypeId() const { return _traits->TypeId(); }
    ETypeFlags TypeFlags() const { return _traits->TypeFlags(); }
    FTypeInfos TypeInfos() const { return _traits->TypeInfos(); }
    FStringView TypeName() const { return _traits->TypeName(); }
    size_t SizeInBytes() const { return _traits->SizeInBytes(); }
    FNamedTypeInfos NamedTypeInfos() const { return _traits->NamedTypeInfos(); }

    bool IsScalar() const { return _traits->TypeInfos().IsScalar(); }
    bool IsTuple() const { return _traits->TypeInfos().IsTuple(); }
    bool IsList() const { return _traits->TypeInfos().IsList(); }
    bool IsDico() const { return _traits->TypeInfos().IsDico(); }

    bool IsAlias() const { return _traits->TypeInfos().IsAlias(); }
    bool IsArithmetic() const { return _traits->TypeInfos().IsArithmetic(); }
    bool IsBoolean() const { return _traits->TypeInfos().IsBoolean(); }
    bool IsEnum() const { return _traits->TypeInfos().IsEnum(); }
    bool IsFloatingPoint() const { return _traits->TypeInfos().IsFloatingPoint(); }
    bool IsNative() const { return _traits->TypeInfos().IsNative(); }
    bool IsObject() const { return _traits->TypeInfos().IsObject(); }
    bool IsString() const { return _traits->TypeInfos().IsString(); }
    bool IsIntegral() const { return _traits->TypeInfos().IsIntegral(); }
    bool IsSignedIntegral() const { return _traits->TypeInfos().IsSignedIntegral(); }
    bool IsUnsignedIntegral() const { return _traits->TypeInfos().IsUnsignedIntegral(); }
    bool IsStructured() const { return _traits->TypeInfos().IsStructured(); }

    bool IsPOD() const { return _traits->TypeInfos().IsPOD(); }
    bool IsTriviallyDestructible() const { return _traits->TypeInfos().IsTriviallyDestructible(); }

    PPE_RTTI_API bool IsAny() const;

    bool IsDefaultValue() const { return _traits->IsDefaultValue(_data); }
    void ResetToDefaultValue() const { _traits->ResetToDefaultValue(_data); }

    PPE_RTTI_API bool Equals(const FAtom& other) const;
    PPE_RTTI_API void Copy(const FAtom& dst) const;
    PPE_RTTI_API void Move(const FAtom& dst);

    NODISCARD PPE_RTTI_API bool DeepEquals(const FAtom& other) const;
    NODISCARD PPE_RTTI_API bool DeepCopy(const FAtom& dst) const;

    NODISCARD bool PromoteCopy(const FAtom& dst) const { return _traits->PromoteCopy(_data, dst); }
    NODISCARD bool PromoteMove(const FAtom& dst) const { return _traits->PromoteMove(_data, dst); }

    template <typename T>
    T& FlatData() const {
        Assert_NoAssume(MakeTraits<T>()->TypeId() == _traits->TypeId());
        return (*static_cast<T*>(_data));
    }

    template <typename T>
    T& TypedData() const {
        const PTypeTraits dst = MakeTraits<T>();
        void* const casted = ((*dst == *_traits) ? _data : Cast(dst));
        Assert(casted);
        return (*static_cast<T*>(casted));
    }

    template <typename T>
    const T& TypedConstData() const {
        return TypedData<T>();
    }

    template <typename T>
    T* TypedDataIFP() const {
        const PTypeTraits dst = MakeTraits<T>();
        void* const casted = ((*dst == *_traits) ? _data : Cast(dst));
        return (casted ? static_cast<T*>(casted) : nullptr);
    }

    template <typename T>
    const T* TypedConstDataIFP() const {
        return TypedDataIFP<T>();
    }

    // /!\ slow, obviously
    PPE_RTTI_API FString ToString() const;
    PPE_RTTI_API FWString ToWString() const;

    hash_t HashValue() const { return _traits->HashValue(_data); }

    void Swap(FAtom& other) {
        std::swap(_data, other._data);
        std::swap(_traits, other._traits);
    }

    void SwapValue(const FAtom& other) const {
        _traits->Swap(_data, other.Data());
    }

    bool Accept(IAtomVisitor* visitor) const {
        return _traits->Accept(visitor, _data);
    }

    inline friend void swap(FAtom& lhs, FAtom& rhs) NOEXCEPT { lhs.Swap(rhs); }
    inline friend hash_t hash_value(const FAtom& value) NOEXCEPT { return value.HashValue(); }

    PPE_RTTI_API static FAtom FromObj(const PMetaObject& obj) NOEXCEPT;
    PPE_RTTI_API bool FromString(const FStringConversion& conv) const NOEXCEPT;

private:
    void* _data;
    PTypeTraits _traits;
};
//----------------------------------------------------------------------------
PPE_ASSUME_TYPE_AS_POD(FAtom);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
FAtom MakeAtom(T* data) {
    return FAtom(data, MakeTraits<T>());
}
//----------------------------------------------------------------------------
template <typename T>
FAtom InplaceAtom(const T& inplace) {
    return FAtom(&inplace, MakeTraits<T>());
}
//----------------------------------------------------------------------------
template <typename T>
T* Cast(const FAtom& atom) {
    return (static_cast<T*>(atom.Traits()
        ? atom.Cast(MakeTraits<T>())
        : nullptr ));
}
//----------------------------------------------------------------------------
template <typename T>
T* CastChecked(const FAtom& atom) {
    return (&atom.TypedData<T>());
}
//----------------------------------------------------------------------------
inline bool is_scalar_v(const FAtom& atom) { return (atom.IsScalar()); }
inline bool is_tuple_v(const FAtom& atom) { return (atom.IsTuple()); }
inline bool is_list_v(const FAtom& atom) { return (atom.IsList()); }
inline bool is_dico_v(const FAtom& atom) { return (atom.IsDico()); }
inline bool is_alias_v(const FAtom& atom) { return (atom.IsAlias()); }
inline bool is_arithmetic_v(const FAtom& atom) { return (atom.IsArithmetic()); }
inline bool is_boolean_v(const FAtom& atom) { return (atom.IsBoolean()); }
inline bool is_enum_v(const FAtom& atom) { return (atom.IsEnum()); }
inline bool is_floating_point_v(const FAtom& atom) { return (atom.IsFloatingPoint()); }
inline bool is_native_v(const FAtom& atom) { return (atom.IsNative()); }
inline bool is_object_v(const FAtom& atom) { return (atom.IsObject()); }
inline bool is_string_v(const FAtom& atom) { return (atom.IsString()); }
inline bool is_integral_v(const FAtom& atom) { return (atom.IsIntegral()); }
inline bool is_signed_integral_v(const FAtom& atom) { return (atom.IsSignedIntegral()); }
inline bool is_unsigned_integral_v(const FAtom& atom) { return (atom.IsUnsignedIntegral()); }
inline bool is_pod_v(const FAtom& atom) { return (atom.IsPOD()); }
inline bool is_trivially_destructible_v(const FAtom& atom) { return (atom.IsTriviallyDestructible()); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, const FAtom& atom);
PPE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, const FAtom& atom);
//----------------------------------------------------------------------------
PPE_RTTI_API bool operator >>(const FStringConversion& iss, FAtom* atom);
PPE_RTTI_API bool operator >>(const FWStringConversion& iss, FAtom* atom);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
T& checked_cast(RTTI::FAtom& atom) NOEXCEPT {
    return atom.TypedData<T>();
}
//----------------------------------------------------------------------------
template <typename T>
const T& checked_cast(const RTTI::FAtom& atom) NOEXCEPT {
    return atom.TypedConstData<T>();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
