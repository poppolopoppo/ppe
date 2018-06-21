#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/TypeTraits.h"

#include "Core/IO/TextWriter_fwd.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FAtom {
public:
    FAtom() : _data(nullptr) {}
    FAtom(const void* data, const PTypeTraits& traits)
        : _data((void*)data)
        , _traits(traits) {
    }

    CORE_FAKEBOOL_OPERATOR_DECL() { return _data; }

    void* Data() const { return _data; }
    const PTypeTraits& Traits() const { return _traits; }

    void* Cast(const PTypeTraits& to) const {
        Assert(_data);
        return _traits->Cast(_data, to);
    }

    template <typename T>
    T& FlatData() const {
        Assert(*MakeTraits<T>() == *_traits);
        return (*reinterpret_cast<T*>(_data));
    }

    template <typename T>
    T& TypedData() const {
        const PTypeTraits dst = MakeTraits<T>();
        void* const casted = ((*dst == *_traits) ? _data : Cast(dst));
        Assert(casted);
        return (*reinterpret_cast<T*>(casted));
    }

    template <typename T>
    const T& TypedConstData() const {
        return TypedData<T>();
    }

    template <typename T>
    T* TypedDataIFP() const {
        const PTypeTraits dst = MakeTraits<T>();
        void* const casted = ((*dst == *_traits) ? _data : Cast(dst));
        return (casted ? reinterpret_cast<T*>(casted) : nullptr);
    }

    template <typename T>
    const T* TypedConstDataIFP() const {
        return TypedDataIFP<T>();
    }

    FTypeId TypeId() const { return _traits->TypeId(); }
    ETypeFlags TypeFlags() const { return _traits->TypeFlags(); }
    FTypeInfos TypeInfos() const { return _traits->TypeInfos(); }
    size_t SizeInBytes() const { return _traits->SizeInBytes(); }

    bool IsDefaultValue() const { return _traits->IsDefaultValue(_data); }
    void ResetToDefaultValue() { _traits->ResetToDefaultValue(_data); }

    bool Equals(const FAtom& other) const {
        return _traits->Equals(*this, other);
    }

    void Copy(const FAtom& dst) const {
        Assert(dst && _traits);
        Assert(*dst.Traits() == *_traits);
        _traits->Copy(_data, dst.Data());
    }

    void Move(const FAtom& dst) {
        Assert(dst && _traits);
        Assert(*dst.Traits() == *_traits);
        _traits->Move(_data, dst.Data());
    }

    bool DeepEquals(const FAtom& other) const {
        return (_traits == other._traits && _traits->DeepEquals(_data, other._data));
    }

    bool DeepCopy(const FAtom& dst) const {
        if (_traits == dst._traits) {
            _traits->DeepCopy(_data, dst.Data());
            return true;
        }
        return true;
    }

    bool PromoteCopy(const FAtom& dst) const { return _traits->PromoteCopy(_data, dst); }
    bool PromoteMove(const FAtom& dst) const { return _traits->PromoteMove(_data, dst); }

    // /!\ slow, obviously
    FString ToString() const;
    FWString ToWString() const;

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
    return (reinterpret_cast<T*>(atom.Traits()
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
CORE_ASSUME_TYPE_AS_POD(RTTI::FAtom);
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, const RTTI::FAtom& atom);
CORE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, const RTTI::FAtom& atom);
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
} //!namespace Core
