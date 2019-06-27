#pragma once

#include "RTTI.h"

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

    template <typename... _Args>
    CONSTEXPR explicit TBaseTypeTraits(_Args&&... args)
        : _Parent(std::forward<_Args>(args)...)
    {}

public: // ITypeTraits
    virtual void Construct(void* data) const override final;
    virtual void ConstructCopy(void* data, const void* other) const override final;
    virtual void ConstructMove(void* data, void* rvalue) const override final;
    virtual void ConstructMoveDestroy(void* data, void* rvalue) const override final;
    virtual void ConstructSwap(void* data, void* other) const override final;
    virtual void Destroy(void* data) const override final;

    using _Parent::IsDefaultValue;//virtual bool IsDefaultValue(const void* data) const override final;
    using _Parent::ResetToDefaultValue;//virtual void ResetToDefaultValue(void* data) const override final;

    using _Parent::Equals;//virtual bool Equals(const void* lhs, const void* rhs) const override final;
    virtual void Copy(const void* src, void* dst) const override final;
    virtual void Move(void* src, void* dst) const override final;

    virtual void Swap(void* lhs, void* rhs) const override final;

    using _Parent::DeepEquals;//virtual bool DeepEquals(const void* lhs, const void* rhs) const override final;
    using _Parent::DeepCopy;//virtual void DeepCopy(const void* src, void* dst) const override final;

    virtual bool PromoteCopy(const void* src, const FAtom& dst) const override /*final*/;
    virtual bool PromoteMove(void* src, const FAtom& dst) const override /*final*/;

    virtual void* Cast(void* data, const PTypeTraits& dst) const override /*final*/;

    using _Parent::HashValue;//virtual hash_t HashValue(const void* data) const override final;
};
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::Construct(void* data) const {
    Assert(data);

    Meta::Construct(reinterpret_cast<pointer>(data), Meta::ForceInit);
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::ConstructCopy(void* data, const void* other) const {
    Assert(data);
    Assert(other);

    Meta::Construct(reinterpret_cast<pointer>(data), *reinterpret_cast<const_pointer>(other));
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::ConstructMove(void* data, void* rvalue) const {
    Assert(data);
    Assert(rvalue);

    Meta::Construct(reinterpret_cast<pointer>(data), std::move(*reinterpret_cast<pointer>(rvalue)));
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::ConstructMoveDestroy(void* data, void* rvalue) const {
    Assert(data);
    Assert(rvalue);

    Meta::Construct(reinterpret_cast<pointer>(data), std::move(*reinterpret_cast<pointer>(rvalue)));
    Meta::Destroy(reinterpret_cast<pointer>(rvalue));
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::ConstructSwap(void* data, void* other) const {
    Assert(data);
    Assert(other);

    using std::swap;

    Meta::Construct(reinterpret_cast<pointer>(data), Meta::ForceInit);
    swap(*reinterpret_cast<pointer>(data), *reinterpret_cast<pointer>(other));
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::Destroy(void* data) const {
    Assert(data);

    Meta::Destroy(reinterpret_cast<pointer>(data));
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::Copy(const void* src, void* dst) const {
    Assert(src);
    Assert(dst);

    *reinterpret_cast<pointer>(dst) = *reinterpret_cast<const_pointer>(src);
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::Move(void* src, void* dst) const {
    Assert(src);
    Assert(dst);

    *reinterpret_cast<pointer>(dst) = std::move(*reinterpret_cast<pointer>(src));
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void TBaseTypeTraits<T, _Parent>::Swap(void* lhs, void* rhs) const {
    Assert(lhs);
    Assert(rhs);

    using std::swap;
    swap(*reinterpret_cast<pointer>(lhs), *reinterpret_cast<pointer>(rhs));
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
bool TBaseTypeTraits<T, _Parent>::PromoteCopy(const void* src, const FAtom& dst) const {
    Assert(src);
    Assert(dst);

    if (*dst.Traits() == *this) {
        Copy(src, dst.Data());
        return true;
    }
    else if (dst.Traits()->TypeId() == FTypeId(ENativeType::Any)) {
        AssignCopy(reinterpret_cast<FAny*>(dst.Data()), src, *this);
        return true;
    }
    else {
        return _Parent::PromoteCopy(src, dst);
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
bool TBaseTypeTraits<T, _Parent>::PromoteMove(void* src, const FAtom& dst) const {
    Assert(src);
    Assert(dst);

    if (*dst.Traits() == *this) {
        Move(src, dst.Data());
        return true;
    }
    else if (dst.Traits()->TypeId() == FTypeId(ENativeType::Any)) {
        AssignMove(reinterpret_cast<FAny*>(dst.Data()), src, *this);
        return true;
    }
    else {
        return _Parent::PromoteMove(src, dst);
    }
}
//----------------------------------------------------------------------------
template <typename T, typename _Parent>
void* TBaseTypeTraits<T, _Parent>::Cast(void* data, const PTypeTraits& dst) const {
    Assert(data);

    return (*dst == *this ? data : nullptr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_RTTI_API FString MakeTupleTypeName(const TMemoryView<const PTypeTraits>& elements);
PPE_RTTI_API FString MakeListTypeName(const PTypeTraits& value);
PPE_RTTI_API FString MakeDicoTypeName(const PTypeTraits& key, const PTypeTraits& value);
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

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// SFINAE to detect RTTI support (need to be defined at the end of this file for CLANG)
//----------------------------------------------------------------------------
namespace details {
template <typename T, typename = decltype(Traits(std::declval<Meta::TType<T>>())) >
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
    return Traits(Meta::TType< Meta::TDecay<T> >{});
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
