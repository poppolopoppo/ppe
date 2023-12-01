#pragma once

#include "RTTI_fwd.h"

#include "RTTI/Atom.h"
#include "RTTI/NativeTypes.h"
#include "RTTI/TypeTraits.h"

#include "Allocator/AllocatorBlock.h"
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
        has_support_for_v<T> &&
        not std::is_same_v<Meta::TDecay<T>, FAny>
    >;

    FAny() NOEXCEPT {}
    ~FAny();

    FAny(const FAny& other);
    FAny& operator =(const FAny& other);

    FAny(FAny&& rvalue) NOEXCEPT;
    FAny& operator =(FAny&& rvalue) NOEXCEPT;

    explicit FAny(ENativeType type);
    explicit FAny(const PTypeTraits& type);

    template <typename T, class = TWrapable<T> >
    explicit FAny(T&& rvalue) NOEXCEPT : FAny() {
        AssignMove_AssumeNotInitialized_(&rvalue, *MakeTraits<T>());
    }

    template <typename T, class = TWrapable<T> >
    explicit FAny(const T& value) : FAny() {
        AssignCopy_AssumeNotInitialized_(&value, *MakeTraits<T>());
    }

    operator FAtom () const { return InnerAtom(); }

    bool Valid() const { return (!!_traitsWFlags.Get()); }
    PPE_FAKEBOOL_OPERATOR_DECL() { return Valid(); }

    void* Data() {
        if (not Valid())
            return this;
        if (Likely(IsFittingInSitu_()))
            return std::addressof(_inSitu);
        return _allocatorBlock.Data;
    }
    const void* Data() const { return const_cast<FAny*>(this)->Data(); }

    PTypeTraits Traits() const {
        const ITypeTraits* const pTraits = _traitsWFlags.Get();
        if (Likely(pTraits))
            return PTypeTraits{ _traitsWFlags.Get() };
        return MakeTraits<FAny>();
    }

    FAtom InnerAtom() const { return FAtom(Data(), Traits()); }

    void Reset() { if (Valid()) Reset_AssumeInitialized_(); }

    FAny& Reset(ENativeType type);
    FAny& Reset(const ITypeTraits& traits);
    FAny& Reset(const PTypeTraits& traits) { return Reset(*traits); }

    void AssignCopy(const FAtom& atom) { AssignCopy(atom.Data(), *atom.Traits()); }
    void AssignMove(const FAtom& atom) NOEXCEPT { AssignMove(atom.Data(), *atom.Traits()); }

    void AssignCopy(const void* src, const ITypeTraits& traits) { AssignCopy_(src, traits); }
    void AssignMove(void* src, const ITypeTraits& traits) NOEXCEPT { AssignMove_(src, traits); }

    template <typename T, class = TWrapable<T> >
    void Assign(T&& rvalue) NOEXCEPT {
        AssignMove(RTTI::MakeAtom(&rvalue));
    }

    template <typename T, class = TWrapable<T> >
    void Assign(const T& value) {
        AssignCopy(RTTI::MakeAtom(&value));
    }

    template <typename T, class = TWrapable<T> >
    T& MakeDefault_AssumeNotValid() {
        Assert_NoAssume(not Valid());
        Assign(Meta::DefaultValue<T>());
        return FlatData<T>();
    }

    template <typename T, class = TWrapable<T> >
    T& MakeDefault_IFP() {
        if (not Valid())
            Assign(Meta::DefaultValue<T>());
        return FlatData<T>();
    }

    NODISCARD bool IsDefaultValue() const { return InnerAtom().IsDefaultValue(); }

    NODISCARD bool PromoteCopy(const FAtom& dst) const { return InnerAtom().PromoteCopy(dst); }
    NODISCARD bool PromoteMove(const FAtom& dst) const NOEXCEPT { return InnerAtom().PromoteMove(dst); }

    template <typename T>
    T& FlatData() const { return InnerAtom().FlatData<T>(); }
    template <typename T>
    T& TypedData() const { return InnerAtom().TypedData<T>(); }
    template <typename T>
    const T& TypedConstData() const { return InnerAtom().TypedConstData<T>(); }
    template <typename T>
    T* TypedDataIFP() const { return InnerAtom().TypedDataIFP<T>(); }
    template <typename T>
    const T* TypedConstDataIFP() const { return InnerAtom().TypedConstDataIFP<T>(); }

    bool Equals(const FAny& other) const;
    inline friend bool operator ==(const FAny& lhs, const FAny& rhs) { return lhs.Equals(rhs); }
    inline friend bool operator !=(const FAny& lhs, const FAny& rhs) { return (not operator ==(lhs, rhs)); }

    hash_t HashValue() const;
    inline friend hash_t hash_value(const FAny& value) NOEXCEPT { return value.HashValue(); }

    void Swap(FAny& other);
    inline friend void swap(FAny& lhs, FAny& rhs) NOEXCEPT { lhs.Swap(rhs); }

    STATIC_CONST_INTEGRAL(size_t, GInSituSize, 3 * sizeof(intptr_t));
    STATIC_CONST_INTEGRAL(size_t, GInSituAlignment, sizeof(intptr_t));

private:
    typedef ALIGNED_STORAGE(GInSituSize, GInSituAlignment) insitu_type;

    STATIC_ASSERT(sizeof(FAllocatorBlock) <= GInSituSize);

    Meta::TPointerWFlags<const ITypeTraits> _traitsWFlags{};
    union {
        mutable insitu_type _inSitu;
        FAllocatorBlock _allocatorBlock;
    };

    bool IsFittingInSitu_() const { return _traitsWFlags.Flag0(); }
    void SetFittingInSitu_(bool value) { _traitsWFlags.SetFlag0(value); }

    void SetTraits_(const ITypeTraits& traits) NOEXCEPT;

    void AssignCopy_(const void* src, const ITypeTraits& traits);
    void AssignMove_(void* src, const ITypeTraits& traits) NOEXCEPT;
    void AssignMoveDestroy_(void* src, const ITypeTraits& traits) NOEXCEPT;

    void AssignCopy_AssumeNotInitialized_(const void* src, const ITypeTraits& traits);
    void AssignMove_AssumeNotInitialized_(void* src, const ITypeTraits& traits) NOEXCEPT;
    void AssignMoveDestroy_AssumeNotInitialized_(void* src, const ITypeTraits& traits) NOEXCEPT;

    void* Allocate_AssumeNotInitialized_(size_t sizeInBytes);
    void Reset_AssumeInitialized_();
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
PPE_RTTI_API void AssignMove(FAny* dst, void* src, const ITypeTraits& traits) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator << (TBasicTextWriter<_Char>& oss, const FAny& any) {
    return oss << any.InnerAtom();
}
//----------------------------------------------------------------------------
template <typename _Char>
bool operator >> (const TBasicStringConversion<_Char>& iss, FAny* any) {
    Assert(any);

    if (any->Valid()) {
        FAtom atom = any->InnerAtom();
        return (iss >> &atom);
    }

    TBasicString<_Char> str;
    if (iss >> &str) {
        any->Assign(std::move(str));
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
