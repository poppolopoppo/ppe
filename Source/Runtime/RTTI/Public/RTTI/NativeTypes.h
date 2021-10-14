#pragma once

#include "RTTI_fwd.h"

#include "RTTI/Atom.h"
#include "RTTI/AtomHelpers.h"
#include "RTTI/TypeInfos.h"
#include "RTTI/TypeTraits.h"
#include "RTTI/Typedefs.h"

#include "RTTI/NativeTypes.Definitions-inl.h"
#include "NativeTraits.h"

#include "IO/StringView.h"
#include "IO/TextWriter_fwd.h"
#include "Misc/Function.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Needed to manipulate FAny with fwd decl only :
//----------------------------------------------------------------------------
PPE_RTTI_API void AssignCopy(FAny* dst, const void* src, const ITypeTraits& traits);
PPE_RTTI_API void AssignMove(FAny* dst, void* src, const ITypeTraits& traits) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_RTTI_API bool AtomVisit(IAtomVisitor& visitor, const ITupleTraits* tuple, void* data);
PPE_RTTI_API bool AtomVisit(IAtomVisitor& visitor, const IListTraits* list, void* data);
PPE_RTTI_API bool AtomVisit(IAtomVisitor& visitor, const IDicoTraits* dico, void* data);
//----------------------------------------------------------------------------
#define DECL_ATOMVISIT_SCALAR(_Name, T, _TypeId) \
    PPE_RTTI_API bool AtomVisit(IAtomVisitor& visitor, const IScalarTraits* scalar, T& value);
FOREACH_RTTI_NATIVETYPES(DECL_ATOMVISIT_SCALAR)
#undef DECL_ATOMVISIT_SCALAR
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
    virtual PTypeTraits CommonType(const PTypeTraits& other) const override final;
    virtual bool PromoteCopy(const void* src, const FAtom& dst) const override final;
    virtual bool PromoteMove(void* src, const FAtom& dst) const NOEXCEPT override final;
*/

    void* BaseCast(void* data, const PTypeTraits& dst) const NOEXCEPT;

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
void* TBaseTypeTraits<T, _Parent>::BaseCast(void* data, const PTypeTraits& dst) const NOEXCEPT {
    Assert(data);
    Assert(dst);

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
    else if (dst.IsAny()) {
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
    else if (dst.IsAny()) {
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
// static type names are allocated with a dedicated heap:
PPE_RTTI_API void TypeNamesStart();
PPE_RTTI_API void TypeNamesShutdown();
// this is why these methods can return a FStringView instead of a FString:
PPE_RTTI_API FStringView MakeTupleTypeName(const TMemoryView<const PTypeTraits>& elements);
PPE_RTTI_API FStringView MakeListTypeName(const PTypeTraits& value);
PPE_RTTI_API FStringView MakeDicoTypeName(const PTypeTraits& key, const PTypeTraits& value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
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
template <typename T>
CONSTEXPR PTypeTraits MakeCommonType(const PTypeTraits& other) {
    Assert(other);

    if (other->TypeId() == MakeTypeInfos<T>().TypeId)
        return other;

    switch (other->TypeId()) {
#define DECL_NATIVETYPE_COMMONTYPE(_Name, _Other, _TypeId) \
    case _TypeId: \
        IF_CONSTEXPR( Meta::has_common_type_v<T, _Other> ) \
            return MakeTraits< typename std::common_type<T, _Other>::type >(); \
        break;
        FOREACH_RTTI_NATIVETYPES(DECL_NATIVETYPE_COMMONTYPE)
    #undef DECL_NATIVETYPE_COMMONTYPE
    }

    return PTypeTraits{};
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE

#include "RTTI/NativeTypes.Scalar-inl.h"
#include "RTTI/NativeTypes.Tuple-inl.h"
#include "RTTI/NativeTypes.List-inl.h"
#include "RTTI/NativeTypes.Dico-inl.h"
#include "RTTI/NativeTypes.Struct-inl.h"
#include "RTTI/NativeTypes.Aliasing-inl.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Should be the only method to acquire the PTypeTraits associated with T
//----------------------------------------------------------------------------
template <typename T>
PTypeTraits MakeTraits() NOEXCEPT {
    return RTTI_Traits(TypeTag< T >);
}
//----------------------------------------------------------------------------
// SFINAE to detect RTTI support (need to be defined at the end of this file for CLANG)
//----------------------------------------------------------------------------
namespace details {
template <typename T, typename = decltype( RTTI_TypeInfos(TypeTag< T >) ) >
std::true_type has_support_for_(int);
template <typename T>
std::false_type has_support_for_(...);
} //!details
template <typename T>
using has_support_for = decltype(details::has_support_for_<T>(0));
template <typename T>
CONSTEXPR bool has_support_for_v = has_support_for<T>::value;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
