#pragma once

#include "RTTI/NativeTypes.h"

#include "Container/Array.h"
#include "Container/Pair.h"
#include "Container/Tuple.h"
#include "Maths/ScalarMatrix_fwd.h"
#include "Maths/ScalarVector_fwd.h"

namespace PPE {
class FQuaternion;
class FTransform;
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TBaseTupleTraits<_Args...>
//----------------------------------------------------------------------------
PPE_RTTI_API PTypeTraits MakeAnyTuple(size_t arity);
//----------------------------------------------------------------------------
template <typename... _Args>
class TBaseTupleTraits : public ITupleTraits {
public: // ITypeTraits
    CONSTEXPR explicit TBaseTupleTraits(size_t sizeInBytes)
    :   ITupleTraits(
        MakeTupleTypeId({ MakeTraits<_Args>()... }),
        MakeTupleTypeFlags({ MakeTraits<_Args>()... }),
        sizeInBytes )
    {}

    virtual FStringView TypeName() const override final;

    virtual bool IsDefaultValue(const void* data) const override final;
    virtual void ResetToDefaultValue(void* data) const override final;

    virtual bool Equals(const void* lhs, const void* rhs) const override final;
    virtual hash_t HashValue(const void* data) const override final;

    virtual bool DeepEquals(const void* lhs, const void* dst) const override final;
    virtual void DeepCopy(const void* src, void* dst) const override final;

    virtual bool PromoteCopy(const void* src, const FAtom& dst) const override /*final*/;
    virtual bool PromoteMove(void* src, const FAtom& dst) const override /*final*/;

public: // ITupleTraits
    virtual size_t Arity() const override final { return (sizeof...(_Args)); }
    virtual TMemoryView<const PTypeTraits> TupleTraits() const override final;
};
//----------------------------------------------------------------------------
template <typename... _Args>
FStringView TBaseTupleTraits<_Args...>::TypeName() const {
    ONE_TIME_INITIALIZE(const FString, GCachedTypeName, MakeTupleTypeName({
        MakeTraits<_Args>()...
        }));
    return GCachedTypeName.MakeView();
}
//----------------------------------------------------------------------------
template <typename... _Args>
bool TBaseTupleTraits<_Args...>::IsDefaultValue(const void* data) const {
    forrange(i, 0, sizeof...(_Args))
        if (not At((void*)data, i).IsDefaultValue())
            return false;

    return true;
}
//----------------------------------------------------------------------------
template <typename... _Args>
void TBaseTupleTraits<_Args...>::ResetToDefaultValue(void* data) const {
    forrange(i, 0, sizeof...(_Args))
        At(data, i).ResetToDefaultValue();
}
//----------------------------------------------------------------------------
template <typename... _Args>
bool TBaseTupleTraits<_Args...>::Equals(const void* lhs, const void* rhs) const {
    Assert(lhs);
    Assert(rhs);

    forrange(i, 0, sizeof...(_Args)) {
        const FAtom lhsIt = At((void*)lhs, i);
        const FAtom rhsIt = At((void*)lhs, i);

        if (not lhsIt.Equals(rhsIt))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
template <typename... _Args>
hash_t TBaseTupleTraits<_Args...>::HashValue(const void* data) const {
    Assert(data);

    hash_t h{ TypeId() };

    ForEach((void*)data, [&h](const FAtom& elt) {
        hash_combine(h, elt.HashValue());
        return true;
    });

    return h;
}
//----------------------------------------------------------------------------
template <typename... _Args>
bool TBaseTupleTraits<_Args...>::PromoteCopy(const void* src, const FAtom& dst) const {
    Assert(src);
    Assert(dst);

    if (const ITupleTraits* const dstTuple = dst.Traits()->AsTuple()) {
        if (dstTuple->Arity() != sizeof...(_Args))
            return false;

        forrange(i, 0, sizeof...(_Args)) {
            const FAtom s = At((void*)src, i);
            const FAtom d = dstTuple->At(dst.Data(), i);

            if (not s.PromoteCopy(d))
                return false;
        }

        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
template <typename... _Args>
bool TBaseTupleTraits<_Args...>::PromoteMove(void* src, const FAtom& dst) const {
    Assert(src);
    Assert(dst);

    if (const ITupleTraits* const dstTuple = dst.Traits()->AsTuple()) {
        if (dstTuple->Arity() != sizeof...(_Args))
            return false;

        forrange(i, 0, sizeof...(_Args)) {
            const FAtom s = At(src, i);
            const FAtom d = dstTuple->At(dst.Data(), i);

            if (not s.PromoteMove(d))
                return false;
        }

        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
template <typename... _Args>
bool TBaseTupleTraits<_Args...>::DeepEquals(const void* lhs, const void* rhs) const {
    Assert(lhs);
    Assert(rhs);

    forrange(i, 0, sizeof...(_Args)) {
        const FAtom a = At((void*)lhs, i);
        const FAtom b = At((void*)rhs, i);

        if (not a.DeepEquals(b))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
template <typename... _Args>
void TBaseTupleTraits<_Args...>::DeepCopy(const void* src, void* dst) const {
    Assert(src);
    Assert(dst);

    forrange(i, 0, sizeof...(_Args)) {
        const FAtom s = At((void*)src, i);
        const FAtom d = At(dst, i);

        s.DeepCopy(d);
    }
}
//----------------------------------------------------------------------------
template <typename... _Args>
TMemoryView<const PTypeTraits> TBaseTupleTraits<_Args...>::TupleTraits() const {
#if 0
    typedef const PTypeTraits FTupleTraits[sizeof...(_Args)];
    ONE_TIME_INITIALIZE(FTupleTraits, GTupleTraits, {
        MakeTraits<_Args>()...
        });
#else
#   ifndef PPE_HAS_MAGIC_STATICS
#       error "C++11 magic statics are needed here (TODO ?)"
#   endif
    static const PTypeTraits GTupleTraits[sizeof...(_Args)] = {
        MakeTraits<_Args>()...
    };
#endif
    return MakeView(GTupleTraits);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TPairTraits<_First, _Second>
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
class TPairTraits final : public TBaseTypeTraits< TPair<_First, _Second>, TBaseTupleTraits<_First, _Second> > {
    using base_traits = TBaseTypeTraits< TPair<_First, _Second>, TBaseTupleTraits<_First, _Second> >;
    using typename base_traits::value_type;
    using typename base_traits::pointer;
    using typename base_traits::const_pointer;

public: // ITypeTraits
    CONSTEXPR TPairTraits() : base_traits(sizeof(TPair<_First, _Second>)) {}

public: // ITupleTraits
    using typename base_traits::foreach_fun;

    virtual FAtom At(void* data, size_t index) const override final;

    virtual bool ForEach(void* data, const foreach_fun& foreach) const override final;
};
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
PTypeTraits Traits(Meta::TType< TPair<_First, _Second> >) noexcept {
    return PTypeTraits::MakeDynamic< TPairTraits<_First, _Second> >();
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
FAtom TPairTraits<_First, _Second>::At(void* data, size_t index) const {
    Assert(data);
    Assert(index < 2);

    return (0 == index
        ? FAtom(&static_cast<value_type*>(data)->first, MakeTraits<_First>())
        : FAtom(&static_cast<value_type*>(data)->second, MakeTraits<_Second>()));
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
bool TPairTraits<_First, _Second>::ForEach(void* data, const foreach_fun& foreach) const {
    Assert(data);

    return (
        foreach(FAtom(&static_cast<value_type*>(data)->first, MakeTraits<_First>())) &&
        foreach(FAtom(&static_cast<value_type*>(data)->second, MakeTraits<_Second>())));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TStaticArrayTraits<T, _Dim>
//----------------------------------------------------------------------------
namespace details {
template <typename T, size_t _Dim>
struct TArrayToTupleTraits {
    template <size_t _Idx>
    using TId = T;
    template <size_t... _Idx>
    static TBaseTupleTraits<TId<_Idx>...> SequenceToTuple(std::index_sequence<_Idx...>);
    using type = decltype(SequenceToTuple(std::declval<std::make_index_sequence<_Dim>>()));
};
} //!details
template <typename T, size_t _Dim>
using TBaseStaticArrayTraits = TBaseTypeTraits<
    TArray<T, _Dim>,
    typename details::TArrayToTupleTraits<T, _Dim>::type
>;
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
class TStaticArrayTraits final : public TBaseStaticArrayTraits<T, _Dim> {
    using base_traits = TBaseStaticArrayTraits<T, _Dim>;
    using typename base_traits::value_type;
    using typename base_traits::pointer;
    using typename base_traits::const_pointer;

public: //ITypeTraits
    CONSTEXPR TStaticArrayTraits() : base_traits(sizeof(TArray<T, _Dim>)) {}

public: // ITupleTraits
    using typename base_traits::foreach_fun;

    virtual FAtom At(void* data, size_t index) const override final;

    virtual bool ForEach(void* data, const foreach_fun& foreach) const override final;
};
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
PTypeTraits Traits(Meta::TType< TArray<T, _Dim> >) noexcept {
    STATIC_ASSERT(sizeof(TArray<T, _Dim>) == sizeof(typename TStaticArrayTraits<T, _Dim>::value_type));
    return PTypeTraits::Make< TStaticArrayTraits<T, _Dim> >();
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FAtom TStaticArrayTraits<T, _Dim>::At(void* data, size_t index) const {
    Assert(data);

    return FAtom(&static_cast<pointer>(data)->at(index), MakeTraits<T>());
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool TStaticArrayTraits<T, _Dim>::ForEach(void* data, const foreach_fun& foreach) const {
    Assert(data);

    const PTypeTraits elt_traits = MakeTraits<T>();

    foreachitem(elt, *static_cast<pointer>(data)) {
        if (not foreach(FAtom(std::addressof(*elt), elt_traits)))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
// extern instantiation for most common types wrapped as a static array :
PPE_RTTI_API PTypeTraits Traits(Meta::TType<byte2>) NOEXCEPT;
PPE_RTTI_API PTypeTraits Traits(Meta::TType<byte4>) NOEXCEPT;
PPE_RTTI_API PTypeTraits Traits(Meta::TType<ubyte2>) NOEXCEPT;
PPE_RTTI_API PTypeTraits Traits(Meta::TType<ubyte4>) NOEXCEPT;
PPE_RTTI_API PTypeTraits Traits(Meta::TType<short2>) NOEXCEPT;
PPE_RTTI_API PTypeTraits Traits(Meta::TType<short4>) NOEXCEPT;
PPE_RTTI_API PTypeTraits Traits(Meta::TType<ushort2>) NOEXCEPT;
PPE_RTTI_API PTypeTraits Traits(Meta::TType<ushort4>) NOEXCEPT;
PPE_RTTI_API PTypeTraits Traits(Meta::TType<word2>) NOEXCEPT;
PPE_RTTI_API PTypeTraits Traits(Meta::TType<word3>) NOEXCEPT;
PPE_RTTI_API PTypeTraits Traits(Meta::TType<word4>) NOEXCEPT;
PPE_RTTI_API PTypeTraits Traits(Meta::TType<uword2>) NOEXCEPT;
PPE_RTTI_API PTypeTraits Traits(Meta::TType<uword3>) NOEXCEPT;
PPE_RTTI_API PTypeTraits Traits(Meta::TType<uword4>) NOEXCEPT;
PPE_RTTI_API PTypeTraits Traits(Meta::TType<float2>) NOEXCEPT;
PPE_RTTI_API PTypeTraits Traits(Meta::TType<float3>) NOEXCEPT;
PPE_RTTI_API PTypeTraits Traits(Meta::TType<float4>) NOEXCEPT;
PPE_RTTI_API PTypeTraits Traits(Meta::TType<float2x2>) NOEXCEPT;
PPE_RTTI_API PTypeTraits Traits(Meta::TType<float3x3>) NOEXCEPT;
PPE_RTTI_API PTypeTraits Traits(Meta::TType<float4x3>) NOEXCEPT;
PPE_RTTI_API PTypeTraits Traits(Meta::TType<float4x4>) NOEXCEPT;
PPE_RTTI_API PTypeTraits Traits(Meta::TType<FQuaternion>) NOEXCEPT;
PPE_RTTI_API PTypeTraits Traits(Meta::TType<FTransform>) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TTupleTraits<_Args...>
//----------------------------------------------------------------------------
template <typename... _Args>
class TTupleTraits final : public TBaseTypeTraits< TTuple<_Args...>, TBaseTupleTraits<_Args...> > {
    using base_traits = TBaseTypeTraits< TTuple<_Args...>, TBaseTupleTraits<_Args...> >;
    using typename base_traits::value_type;
    using typename base_traits::pointer;
    using typename base_traits::const_pointer;

public: //ITypeTraits
    CONSTEXPR TTupleTraits() : base_traits(sizeof(TTuple<_Args...>)) {}

public: // ITupleTraits
    using typename base_traits::foreach_fun;

    virtual FAtom At(void* data, size_t index) const override final;

    virtual bool ForEach(void* data, const foreach_fun& foreach) const override final;

private:
    template <size_t... _Idx>
    static FAtom GetNth_(value_type& tuple, size_t index, std::index_sequence<_Idx...>);
};
//----------------------------------------------------------------------------
template <typename _Arg0, typename... _Args>
PTypeTraits Traits(Meta::TType< TTuple<_Arg0, _Args...> >) noexcept {
    return PTypeTraits::MakeDynamic< TTupleTraits<_Arg0, _Args...> >();
}
//----------------------------------------------------------------------------
template <typename... _Args>
FAtom TTupleTraits<_Args...>::At(void* data, size_t index) const {
    Assert(data);

    return GetNth_(*static_cast<value_type*>(data), index, std::index_sequence_for<_Args...>{});
}
//----------------------------------------------------------------------------
template <typename... _Args>
bool TTupleTraits<_Args...>::ForEach(void* data, const foreach_fun& foreach) const {
    Assert(data);

    value_type& tuple = *static_cast<value_type*>(data);

    forrange(i, 0, sizeof...(_Args)) {
        const FAtom elt = GetNth_(tuple, i, std::index_sequence_for<_Args...>{});
        if (not foreach(elt))
            return false; // not sure if it could be done statically due to this return
    }

    return true;
}
//----------------------------------------------------------------------------
template <typename... _Args>
template <size_t... _Idx>
FAtom TTupleTraits<_Args...>::GetNth_(value_type& tuple, size_t index, std::index_sequence<_Idx...>) {
    Assert(index < sizeof...(_Idx));
    typedef FAtom(*getter_type)(value_type&);
    static const getter_type GGetNth[sizeof...(_Args)] = {
        [](value_type& t) { return MakeAtom(&std::get<_Idx>(t)); }...
    };
    return (GGetNth[index](tuple));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE