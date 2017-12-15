#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/Atom.h"
#include "Core.RTTI/NativeTypes.h"

#include "Core/Meta/AlignedStorage.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// RTTI Runtime polymorphism
//----------------------------------------------------------------------------
class CORE_RTTI_API FAny {
public:
    template <typename T>
    using TWrapable = Meta::TEnableIf<
        TIsSupportedType<T>::value &&
        not std::is_same<FAny, T>::value // forbid to wrap FAny in another FAny
    >;

    FAny() NOEXCEPT;
    ~FAny() { Reset(); }

    explicit FAny(ENativeType type) NOEXCEPT : FAny(MakeTraits(type)) {}
    explicit FAny(const PTypeTraits& type) NOEXCEPT;

    FAny(const FAny& other) { CopyFrom_(other); }
    FAny& operator =(const FAny& other) {
        CopyFrom_(other);
        return (*this);
    }

    FAny(FAny&& rvalue) : FAny() { Swap(rvalue); }
    FAny& operator =(FAny&& rvalue) {
        Reset();
        Swap(rvalue);
        return (*this);
    }

    operator FAtom () const { return InnerAtom(); }

    bool Valid() const { return _traits.Valid(); }
    CORE_FAKEBOOL_OPERATOR_DECL() { return (_traits); }

    void* Data();
    const void* Data() const { return const_cast<FAny*>(this)->Data(); }

    const PTypeTraits& Traits() const { return _traits; }
    FAtom InnerAtom() const { return FAtom(Data(), _traits); }

    void Reset();
    FAtom Reset(const PTypeTraits& traits);

    void AssignCopy(const FAtom& atom);
    void AssignMove(const FAtom& atom);

    template <typename T, class = TWrapable<T> >
    void Assign(T&& rvalue) {
        AssignMove(RTTI::MakeAtom(&rvalue));
    }

    template <typename T, class = TWrapable<T> >
    void Assign(const T& value) {
        AssignCopy(RTTI::MakeAtom(&value));
    }

    bool Equals(const FAny& other) const;
    inline friend bool operator ==(const FAny& lhs, const FAny& rhs) { return lhs.Equals(rhs); }
    inline friend bool operator !=(const FAny& lhs, const FAny& rhs) { return (not operator ==(lhs, rhs)); }

    hash_t HashValue() const;
    inline friend hash_t hash_value(FAny& value) { return value.HashValue(); }

    void Swap(FAny& other) {
        std::swap(_traits, other._traits);
        std::swap(_smallBuffer, other._smallBuffer);
    }
    inline friend void swap(FAny& lhs, FAny& rhs) { lhs.Swap(rhs); }

    STATIC_CONST_INTEGRAL(size_t, GSmallBufferSize, 16);
    STATIC_CONST_INTEGRAL(size_t, GSmallBufferAlignment, sizeof(intptr_t));

private:
    using smallbuffer_type = ALIGNED_STORAGE(GSmallBufferSize, GSmallBufferAlignment);

    union {
        void* _externalStorage;
        smallbuffer_type _smallBuffer;
    };
    PTypeTraits _traits;

    FAtom Allocate_(PTypeTraits&& traits);
    void CopyFrom_(const FAny& other);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, class = FAny::TWrapable<T> >
FAny MakeAny(T&& rvalue) {
    FAny any;
    any.Assign(std::move(rvalue));
    return any;
}
//----------------------------------------------------------------------------
template <typename T, class = FAny::TWrapable<T> >
FAny MakeAny(const T& value) {
    FAny any;
    any.Assign(value);
    return any;
}
//----------------------------------------------------------------------------
template <typename T>
T* Cast(const FAny& any) {
    return Cast<T>(any.InnerAtom());
}
//----------------------------------------------------------------------------
template <typename T>
T& CastChecked(const FAny& any) {
    return *CastChecked<T>(any.InnerAtom());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator << (std::basic_ostream<_Char, _Traits>& oss, const RTTI::FAny& any) {
    return oss << any.InnerAtom();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
