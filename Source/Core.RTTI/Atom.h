#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/TypeTraits.h"

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
        return _traits->Cast(*this, to);
    }

    template <typename T>
    T& TypedData() const {
        Assert(_data);
        void* cast = Cast(MakeTraits<T>());
        Assert(cast);
        return (*reinterpret_cast<T*>(cast));
    }

    template <typename T>
    const T& TypedConstData() const {
        return TypedData<T>();
    }

    FTypeId TypeId() const { return _traits->TypeId(); }
    FTypeInfos TypeInfos() const { return _traits->TypeInfos(); }

    bool IsDefaultValue() const { return _traits->IsDefaultValue(*this); }

    bool Equals(const FAtom& other) const {
        return _traits->Equals(*this, other);
    }

    void Copy(const FAtom& dst) const {
        _traits->Copy(*this, dst);
    }

    void Move(const FAtom& dst) {
        _traits->Move(*this, dst);
    }

    bool DeepEquals(const FAtom& other) const {
        return (_traits == other._traits && _traits->DeepEquals(*this, other));
    }

    bool DeepCopy(const FAtom& dst) const {
        if (_traits != dst._traits)
            return false;
        _traits->DeepCopy(*this, dst);
        return true;
    }

    bool PromoteCopy(const FAtom& dst) const { return _traits->PromoteCopy(*this, dst); }
    bool PromoteMove(const FAtom& dst) const { return _traits->PromoteMove(*this, dst); }

    hash_t HashValue() const { return _traits->HashValue(*this); }

    void Format(std::basic_ostream<char>& oss) const { _traits->Format(oss, *this); }
    void Format(std::basic_ostream<wchar_t>& oss) const { _traits->Format(oss, *this); }

    void Swap(FAtom& other) {
        std::swap(_data, other._data);
        std::swap(_traits, other._traits);
    }

    void SwapValue(const FAtom& other) const {
        _traits->Swap(*this, other);
    }

    bool Accept(IAtomVisitor* visitor) const {
        return _traits->Accept(visitor, *this);
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
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator << (std::basic_ostream<_Char, _Traits>& oss, const RTTI::FAtom& atom) {
    atom.Format(oss);
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
