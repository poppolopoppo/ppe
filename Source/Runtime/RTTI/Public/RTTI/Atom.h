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
    :   _data((void*)data)
    ,   _traits(traits)
    {}

    PPE_FAKEBOOL_OPERATOR_DECL() { return _data; }

    void* Data() const { return _data; }
    const PTypeTraits& Traits() const { return _traits; }

    void* Cast(const PTypeTraits& to) const;

    FTypeId TypeId() const { return _traits->TypeId(); }
    ETypeFlags TypeFlags() const { return _traits->TypeFlags(); }
    FTypeInfos TypeInfos() const { return _traits->TypeInfos(); }
    FStringView TypeName() const { return _traits->TypeName(); }
    size_t SizeInBytes() const { return _traits->SizeInBytes(); }
    FNamedTypeInfos NamedTypeInfos() const { return _traits->NamedTypeInfos(); }

    bool IsScalar() const { return (_traits->TypeFlags() & ETypeFlags::Scalar); }
    bool IsTuple() const { return (_traits->TypeFlags() & ETypeFlags::Tuple); }
    bool IsList() const { return (_traits->TypeFlags() & ETypeFlags::List); }
    bool IsDico() const { return (_traits->TypeFlags() & ETypeFlags::Dico); }
    bool IsEnum() const { return (_traits->TypeFlags() & ETypeFlags::Enum); }
    bool IsObject() const { return (_traits->TypeFlags() & ETypeFlags::Object); }
    bool IsNative() const { return (_traits->TypeFlags() & ETypeFlags::Native); }
    bool IsPOD() const { return (_traits->TypeFlags() & ETypeFlags::POD); }
    bool IsTriviallyDestructible() const { return (_traits->TypeFlags() & ETypeFlags::TriviallyDestructible); }

    bool IsAny() const;

    bool IsDefaultValue() const { return _traits->IsDefaultValue(_data); }
    void ResetToDefaultValue() { _traits->ResetToDefaultValue(_data); }

    bool Equals(const FAtom& other) const;
    void Copy(const FAtom& dst) const;
    void Move(const FAtom& dst);

    bool DeepEquals(const FAtom& other) const;
    bool DeepCopy(const FAtom& dst) const;

    bool PromoteCopy(const FAtom& dst) const { return _traits->PromoteCopy(_data, dst); }
    bool PromoteMove(const FAtom& dst) const { return _traits->PromoteMove(_data, dst); }

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

    inline friend void swap(FAtom& lhs, FAtom& rhs) { lhs.Swap(rhs); }
    inline friend hash_t hash_value(const FAtom& value) { return value.HashValue(); }

    PPE_RTTI_API static FAtom FromObj(const PMetaObject& obj);

private:
    void* _data;
    PTypeTraits _traits;
};
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
PPE_ASSUME_TYPE_AS_POD(RTTI::FAtom);
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, const RTTI::FAtom& atom);
PPE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, const RTTI::FAtom& atom);
//----------------------------------------------------------------------------
template <typename T>
T& checked_cast(RTTI::FAtom& atom) noexcept {
    return atom.TypedData<T>();
}
//----------------------------------------------------------------------------
template <typename T>
const T& checked_cast(const RTTI::FAtom& atom) noexcept {
    return atom.TypedConstData<T>();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
