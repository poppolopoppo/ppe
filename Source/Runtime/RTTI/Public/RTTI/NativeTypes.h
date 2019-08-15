#pragma once

#include "RTTI_fwd.h"

#include "RTTI/Atom.h"
#include "RTTI/AtomHelpers.h"
#include "RTTI/TypeInfos.h"
#include "RTTI/TypeTraits.h"
#include "RTTI/Typedefs.h"

#include "RTTI/NativeTypes.Definitions-inl.h"

#include "IO/String.h"
#include "IO/TextWriter_fwd.h"
#include "Misc/Function.h"

namespace PPE {
namespace RTTI {
// Needed to manipulate FAny with fwd decl only :
PPE_RTTI_API void AssignCopy(FAny* dst, const void* src, const ITypeTraits& traits);
PPE_RTTI_API void AssignMove(FAny* dst, void* src, const ITypeTraits& traits);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TBaseTypeTraits<T, _Parent>
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
class TBaseTypeTraits : public _Parent {
protected:
    typedef T value_type;
    typedef Meta::TAddPointer<T> pointer;
    typedef Meta::TAddPointer<const T> const_pointer;

    using _Parent::_Parent;

public: // ITypeTraits
    virtual void Construct(void* data) const override final;
    virtual void ConstructCopy(void* data, const void* other) const override final;
    virtual void ConstructMove(void* data, void* rvalue) const NOEXCEPT override final;
    virtual void ConstructMoveDestroy(void* data, void* rvalue) const NOEXCEPT override final;
    virtual void ConstructSwap(void* data, void* other) const NOEXCEPT override final;
    virtual void Destroy(void* data) const NOEXCEPT override final;

    virtual void Copy(const void* src, void* dst) const override final;
    virtual void Move(void* src, void* dst) const NOEXCEPT override final;
    virtual void Swap(void* lhs, void* rhs) const NOEXCEPT override final;

protected:
    // Needs to be defined inside derived classes :
/*
    virtual bool IsDefaultValue(const void* data) const override final;
    virtual void ResetToDefaultValue(void* data) const override final;
    virtual bool Equals(const void* lhs, const void* rhs) const override final;
    virtual bool DeepEquals(const void* lhs, const void* rhs) const override final;
    virtual void DeepCopy(const void* src, void* dst) const override final;
    virtual hash_t HashValue(const void* data) const override final;
    virtual void* Cast(void* data, const PTypeTraits& dst) const override final;
    virtual bool PromoteCopy(const void* src, const FAtom& dst) const override final;
    virtual bool PromoteMove(void* src, const FAtom& dst) const NOEXCEPT override final;
*/

    void* BaseCast(void* data, const PTypeTraits& dst) const;
    bool BasePromoteCopy(const void* src, const FAtom& dst) const;
    bool BasePromoteMove(void* src, const FAtom& dst) const NOEXCEPT;
};
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::Construct(void* data) const {
    Assert(data);

    Meta::Construct(static_cast<pointer>(data), Meta::ForceInit);
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::ConstructCopy(void* data, const void* other) const {
    Assert(data);
    Assert(other);

    Meta::Construct(static_cast<pointer>(data), *static_cast<const_pointer>(other));
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::ConstructMove(void* data, void* rvalue) const NOEXCEPT {
    Assert(data);
    Assert(rvalue);

    Meta::Construct(static_cast<pointer>(data), std::move(*static_cast<pointer>(rvalue)));
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::ConstructMoveDestroy(void* data, void* rvalue) const NOEXCEPT {
    Assert(data);
    Assert(rvalue);

    Meta::Construct(static_cast<pointer>(data), std::move(*static_cast<pointer>(rvalue)));
    Meta::Destroy(static_cast<pointer>(rvalue));
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::ConstructSwap(void* data, void* other) const NOEXCEPT {
    Assert(data);
    Assert(other);

    using std::swap;

    Meta::Construct(static_cast<pointer>(data), Meta::ForceInit);
    swap(*static_cast<pointer>(data), *static_cast<pointer>(other));
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::Destroy(void* data) const NOEXCEPT {
    Assert(data);

    Meta::Destroy(static_cast<pointer>(data));
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::Copy(const void* src, void* dst) const {
    Assert(src);
    Assert(dst);

    *static_cast<pointer>(dst) = *static_cast<const_pointer>(src);
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::Move(void* src, void* dst) const NOEXCEPT {
    Assert(src);
    Assert(dst);

    *static_cast<pointer>(dst) = std::move(*static_cast<pointer>(src));
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::Swap(void* lhs, void* rhs) const NOEXCEPT {
    Assert(lhs);
    Assert(rhs);

    using std::swap;
    swap(*static_cast<pointer>(lhs), *static_cast<pointer>(rhs));
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void* TBaseTypeTraits<T, _Parent>::BaseCast(void* data, const PTypeTraits& dst) const {
    Assert(data);

    return (*dst == *this ? data : nullptr);
}

//----------------------------------------------------------------------------
template <typename T, typename _Parent>
bool TBaseTypeTraits<T, _Parent>::BasePromoteCopy(const void* src, const FAtom& dst) const {
    Assert(src);
    Assert(dst);

    if (*dst.Traits() == *this) {
        Copy(src, dst.Data());
        return true;
    }
    else if (dst.Traits()->TypeId() == FTypeId(ENativeType::Any)) {
        AssignCopy(static_cast<FAny*>(dst.Data()), src, *this);
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
bool TBaseTypeTraits<T, _Parent>::BasePromoteMove(void* src, const FAtom& dst) const NOEXCEPT {
    Assert(src);
    Assert(dst);

    if (*dst.Traits() == *this) {
        Move(src, dst.Data());
        return true;
    }
    else if (dst.Traits()->TypeId() == FTypeId(ENativeType::Any)) {
        AssignMove(static_cast<FAny*>(dst.Data()), src, *this);
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_RTTI_API FString MakeTupleTypeName(const TMemoryView<const PTypeTraits>& elements);
PPE_RTTI_API FString MakeListTypeName(const PTypeTraits& value);
PPE_RTTI_API FString MakeDicoTypeName(const PTypeTraits& key, const PTypeTraits& value);
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_PUSH() // #TODO : find where overflow really is when VS2019 get fixed
PRAGMA_MSVC_WARNING_DISABLE(4307) // '*': integral constant overflow
namespace details {
template <typename _Traits, typename T>
CONSTEXPR const _Traits TypeTraits{
    MakeTypeInfos<T>()
};
} //!details
template <typename _Traits, typename T>
CONSTEXPR PTypeTraits MakeStaticType() {
    return PTypeTraits{ &details::TypeTraits< _Traits, Meta::TDecay<T> > };
}
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE

#include "NativeTypes.Scalar-inl.h"
#include "NativeTypes.Tuple-inl.h"
#include "NativeTypes.List-inl.h"
#include "NativeTypes.Dico-inl.h"
#include "NativeTypes.Struct-inl.h"
#include "NativeTypes.Aliasing-inl.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// SFINAE to detect RTTI support (need to be defined at the end of this file for CLANG)
//----------------------------------------------------------------------------
namespace details {
template <typename T, typename = decltype(Traits(std::declval<TType<T>>())) >
std::true_type IsSupportedType_(int);
template <typename T>
std::false_type IsSupportedType_(...);
} //!details
template <typename T>
struct TIsSupportedType {
    using is_supported = decltype(details::IsSupportedType_<T>(0));
    STATIC_CONST_INTEGRAL(bool, value, not std::is_same<void, T>::value && is_supported::value);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Helpers traits for natives types
//----------------------------------------------------------------------------
CONSTEXPR bool is_arithmethic(FTypeId typeId);
CONSTEXPR bool is_integral(FTypeId typeId);
CONSTEXPR bool is_boolean(FTypeId typeId);
CONSTEXPR bool is_non_bool_integral(FTypeId typeId);
CONSTEXPR bool is_signed_integral(FTypeId typeId);
CONSTEXPR bool is_unsigned_integral(FTypeId typeId);
CONSTEXPR bool is_floating_point(FTypeId typeId);
CONSTEXPR bool is_string(FTypeId typeId);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Should be the only method to acquire the PTypeTraits associated with T
//----------------------------------------------------------------------------
template <typename T>
PTypeTraits MakeTraits() NOEXCEPT {
    return Traits(Type< Meta::TDecay<T> >);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
