#pragma once

#include "RTTI_fwd.h"

#include "RTTI/Atom.h"
#include "RTTI/NativeTypes.h"
#include "RTTI/TypeTraits.h"

#include "IO/TextWriter_fwd.h"
#include "Meta/AlignedStorage.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// RTTI Runtime polymorphisms with lifetime management
//----------------------------------------------------------------------------
class PPE_RTTI_API FAny {
public:
    template <typename T>
    using TWrapable = Meta::TEnableIf<
        TIsSupportedType<T>::value &&
        not std::is_same_v<FAny, T> // forbid to wrap FAny in another FAny
    >;

    FAny() NOEXCEPT;
    ~FAny();

    FAny(const FAny& other);
    FAny& operator =(const FAny& other);

    FAny(FAny&& rvalue) NOEXCEPT;
    FAny& operator =(FAny&& rvalue) NOEXCEPT;

    explicit FAny(ENativeType type);
    explicit FAny(const PTypeTraits& type);

    template <typename T, class = TWrapable<T> >
    explicit FAny(T&& rvalue) NOEXCEPT : _traits(Meta::NoInit) {
        const PTypeTraits traits = MakeTraits<T>();
        AssignMove_AssumeNotInitialized_(&rvalue, *traits, traits->SizeInBytes());
    }

    template <typename T, class = TWrapable<T> >
    explicit FAny(const T& value) : _traits(Meta::NoInit) {
        const PTypeTraits traits = MakeTraits<T>();
        AssignCopy_AssumeNotInitialized_(&value, *traits, traits->SizeInBytes());
    }

    operator FAtom () const { return InnerAtom(); }

    bool Valid() const { return _traits.Valid(); }
    PPE_FAKEBOOL_OPERATOR_DECL() { return (_traits); }

    void* Data() { return (_traits ? Data_(_traits->SizeInBytes()) : nullptr); }
    const void* Data() const { return const_cast<FAny*>(this)->Data(); }

    const PTypeTraits& Traits() const { return _traits; }
    FAtom InnerAtom() const { return FAtom(Data(), _traits); }

    void Reset() { if (_traits) Reset_AssumeInitialized_(_traits->SizeInBytes()); }
    FAny& Reset(const PTypeTraits& traits);

    void AssignCopy(const FAtom& atom) { AssignCopy(atom.Data(), *atom.Traits()); }
    void AssignMove(const FAtom& atom) { AssignMove(atom.Data(), *atom.Traits()); }

    void AssignCopy(const void* src, const ITypeTraits& traits) { AssignCopy_(src, traits, traits.SizeInBytes()); }
    void AssignMove(void* src, const ITypeTraits& traits) { AssignMove_(src, traits, traits.SizeInBytes()); }

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

    void Swap(FAny& other);
    inline friend void swap(FAny& lhs, FAny& rhs) { lhs.Swap(rhs); }

    STATIC_CONST_INTEGRAL(size_t, GInSituSize, 3 * sizeof(intptr_t));
    STATIC_CONST_INTEGRAL(size_t, GInSituAlignment, sizeof(intptr_t));

private:
    typedef ALIGNED_STORAGE(GInSituSize, GInSituAlignment) insitu_type;

    struct FExternalData {
        void* Ptr;
        size_t SizeInBytes;
    };
    STATIC_ASSERT(sizeof(FExternalData) <= GInSituSize);

    PTypeTraits _traits;
    union {
        insitu_type _inSitu;
        FExternalData _externalBlock;
    };

    void* Data_(const size_t sizeInBytes) const;

    void AssignCopy_(const void* src, const ITypeTraits& traits, const size_t sizeInBytes);
    void AssignMove_(void* src, const ITypeTraits& traits, const size_t sizeInBytes);
    void AssignMoveDestroy_(void* src, const ITypeTraits& traits, const size_t sizeInBytes);

    void AssignCopy_AssumeNotInitialized_(const void* src, const ITypeTraits& traits, const size_t sizeInBytes);
    void AssignMove_AssumeNotInitialized_(void* src, const ITypeTraits& traits, const size_t sizeInBytes);
    void AssignMoveDestroy_AssumeNotInitialized_(void* src, const ITypeTraits& traits, const size_t sizeInBytes);

    void* Allocate_AssumeNotInitialized_(const size_t sizeInBytes);
    void Reset_AssumeInitialized_(const size_t sizeInBytes);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline FAny& MakeAny(FAny& any) {
    return any;
}
////----------------------------------------------------------------------------
template <typename T, class = FAny::TWrapable<T> >
FAny MakeAny(T&& rvalue) {
    return FAny(std::move(rvalue));
}
//----------------------------------------------------------------------------
template <typename T, class = FAny::TWrapable<T> >
FAny MakeAny(const T& value) {
    return FAny(value);
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
// For fwd declarations
PPE_RTTI_API void AssignCopy(FAny* dst, const void* src, const ITypeTraits& traits);
PPE_RTTI_API void AssignMove(FAny* dst, void* src, const ITypeTraits& traits);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator << (TBasicTextWriter<_Char>& oss, const RTTI::FAny& any) {
    return oss << any.InnerAtom();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
