#pragma once

#include "RTTI/NativeTypes.h"

#include "Container/Array.h"
#include "Container/Pair.h"
#include "Container/Tuple.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_RTTI_API PTypeTraits MakeAnyTuple(size_t arity) NOEXCEPT;
//----------------------------------------------------------------------------
// TBaseTupleTraits<_Args...>
//----------------------------------------------------------------------------
template <typename... _Args>
class TBaseTupleTraits : public ITupleTraits {
public: // ITypeTraits
    using ITupleTraits::ITupleTraits;

    using tuple_type = TTuple<_Args...>;

    virtual FStringView TypeName() const override final;

    virtual bool IsDefaultValue(const void* data) const NOEXCEPT override final;
    virtual void ResetToDefaultValue(void* data) const override final;

    virtual bool Equals(const void* lhs, const void* rhs) const NOEXCEPT override final;
    virtual hash_t HashValue(const void* data) const NOEXCEPT override final;

    virtual bool DeepEquals(const void* lhs, const void* dst) const override final;
    virtual void DeepCopy(const void* src, void* dst) const override final;

public: // ITupleTraits
    virtual size_t Arity() const NOEXCEPT override final { return (sizeof...(_Args)); }
    virtual TMemoryView<const PTypeTraits> TupleTraits() const NOEXCEPT override final;
};
//----------------------------------------------------------------------------
template <typename... _Args>
FStringView TBaseTupleTraits<_Args...>::TypeName() const {
    ONE_TIME_INITIALIZE(const FString, GCachedTypeName, MakeTupleTypeName({ MakeTraits<_Args>()... }));
    return GCachedTypeName.MakeView();
}
//----------------------------------------------------------------------------
template <typename... _Args>
bool TBaseTupleTraits<_Args...>::IsDefaultValue(const void* data) const NOEXCEPT {
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
bool TBaseTupleTraits<_Args...>::Equals(const void* lhs, const void* rhs) const NOEXCEPT {
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
hash_t TBaseTupleTraits<_Args...>::HashValue(const void* data) const NOEXCEPT {
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
TMemoryView<const PTypeTraits> TBaseTupleTraits<_Args...>::TupleTraits() const NOEXCEPT {
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
// TTupleTrait<T, _Args...>
//----------------------------------------------------------------------------
template <typename T, typename... _Args>
class TTupleTraits : public TBaseTypeTraits< T, TBaseTupleTraits<_Args...> > {
protected:
    using base_traits = TBaseTypeTraits< T, TBaseTupleTraits<_Args...> >;

    using typename base_traits::value_type;
    using typename base_traits::pointer;
    using typename base_traits::const_pointer;

    using base_traits::base_traits;

    using base_traits::At;

public: // ITypeTraits
    virtual bool PromoteCopy(const void* src, const FAtom& dst) const override final;
    virtual bool PromoteMove(void* src, const FAtom& dst) const NOEXCEPT override final;

    virtual void* Cast(void* data, const PTypeTraits& dst) const override final {
        return base_traits::BaseCast(data, dst);
    }
};
//----------------------------------------------------------------------------
template <typename T, typename... _Args>
bool TTupleTraits<T, _Args...>::PromoteCopy(const void* src, const FAtom& dst) const {
    Assert(src);
    Assert(dst);

    if (base_traits::BasePromoteCopy(src, dst))
        return true;

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
template <typename T, typename... _Args>
bool TTupleTraits<T, _Args...>::PromoteMove(void* src, const FAtom& dst) const NOEXCEPT {
    Assert(src);
    Assert(dst);

    if (base_traits::BasePromoteMove(src, dst))
        return true;

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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Tuple, typename... _Args>
CONSTEXPR PTypeInfos TupleTypeInfos(TType< TTuple<_Args...> >) {
    return FTypeHelpers::Tuple< _Tuple, _Args... >;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TPairTraits<_First, _Second>
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
class TPairTraits final : public TTupleTraits< TPair<_First, _Second>, _First, _Second > {
    using base_traits = TTupleTraits< TPair<_First, _Second>, _First, _Second >;
    using typename base_traits::value_type;
    using typename base_traits::pointer;
    using typename base_traits::const_pointer;

public: // ITypeTraits
    using base_traits::base_traits;

public: // ITupleTraits
    using typename base_traits::foreach_fun;

    virtual FAtom At(void* data, size_t index) const NOEXCEPT override final;

    virtual bool ForEach(void* data, const foreach_fun& foreach) const NOEXCEPT override final;
};
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
CONSTEXPR PTypeInfos TypeInfos(TType< TPair<_First, _Second> >) {
    return FTypeHelpers::Tuple< TPair<_First, _Second>, _First, _Second >;
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
CONSTEXPR PTypeTraits Traits(TType< TPair<_First, _Second> >) {
    return MakeStaticType< TPairTraits<_First, _Second>, TPair<_First, _Second> >();
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
FAtom TPairTraits<_First, _Second>::At(void* data, size_t index) const NOEXCEPT {
    Assert(data);
    Assert(index < 2);

    return (0 == index
        ? FAtom(&static_cast<value_type*>(data)->first, MakeTraits<_First>())
        : FAtom(&static_cast<value_type*>(data)->second, MakeTraits<_Second>()));
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
bool TPairTraits<_First, _Second>::ForEach(void* data, const foreach_fun& foreach) const NOEXCEPT {
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
    using array_type = TArray<T, _Dim>;
    template <size_t _Idx>
    using id_t = T;
    template <size_t... _Idx>
    static TTupleTraits< array_type, id_t<_Idx>... > array_to_tuple(std::index_sequence<_Idx...>);
    using tuple_traits = decltype(array_to_tuple(std::declval<std::make_index_sequence<_Dim>>()));
    using tuple_type = typename tuple_traits::tuple_type;
};
} //!details
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
class TStaticArrayTraits final : public details::TArrayToTupleTraits<T, _Dim>::tuple_traits {
    using base_traits = typename details::TArrayToTupleTraits<T, _Dim>::tuple_traits;
    using typename base_traits::value_type;
    using typename base_traits::pointer;
    using typename base_traits::const_pointer;

public: //ITypeTraits
    using base_traits::base_traits;
    using typename base_traits::tuple_type;

public: // ITupleTraits
    using typename base_traits::foreach_fun;

    virtual FAtom At(void* data, size_t index) const NOEXCEPT override final;

    virtual bool ForEach(void* data, const foreach_fun& foreach) const NOEXCEPT override final;
};
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
CONSTEXPR PTypeInfos TypeInfos(TType< TArray<T, _Dim> >) {
    using tuple_type = typename details::TArrayToTupleTraits<T, _Dim>::tuple_type;
    return TupleTypeInfos< TArray<T, _Dim> >(Type< tuple_type >);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
CONSTEXPR PTypeTraits Traits(TType< TArray<T, _Dim> >) {
    STATIC_ASSERT(sizeof(TArray<T, _Dim>) == sizeof(typename details::TArrayToTupleTraits<T, _Dim>::tuple_type));
    return MakeStaticType< TStaticArrayTraits<T, _Dim>, TArray<T, _Dim> >();
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FAtom TStaticArrayTraits<T, _Dim>::At(void* data, size_t index) const NOEXCEPT {
    Assert(data);

    return FAtom(&static_cast<pointer>(data)->at(index), MakeTraits<T>());
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool TStaticArrayTraits<T, _Dim>::ForEach(void* data, const foreach_fun& foreach) const NOEXCEPT {
    Assert(data);

    const PTypeTraits elt_traits = MakeTraits<T>();

    foreachitem(elt, *static_cast<pointer>(data)) {
        if (not foreach(FAtom(std::addressof(*elt), elt_traits)))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TStdTupleTraits<_Args...>
//----------------------------------------------------------------------------
template <typename... _Args>
class TStdTupleTraits final : public TTupleTraits< TTuple<_Args...>, _Args... > {
    using base_traits = TTupleTraits< TTuple<_Args...>, _Args... >;
    using typename base_traits::value_type;
    using typename base_traits::pointer;
    using typename base_traits::const_pointer;

public: //ITypeTraits
    using base_traits::base_traits;

public: // ITupleTraits
    using typename base_traits::foreach_fun;

    virtual FAtom At(void* data, size_t index) const NOEXCEPT override final;

    virtual bool ForEach(void* data, const foreach_fun& foreach) const NOEXCEPT override final;

private:
    template <size_t... _Idx>
    static FAtom GetNth_(value_type& tuple, size_t index, std::index_sequence<_Idx...>) NOEXCEPT;
};
//----------------------------------------------------------------------------
template <typename _Arg0, typename... _Args>
CONSTEXPR PTypeInfos TypeInfos(TType< TTuple<_Arg0, _Args...> >) {
    return TupleTypeInfos< TTuple<_Arg0, _Args...> >(Type< TTuple<_Arg0, _Args...> >);
}
//----------------------------------------------------------------------------
template <typename _Arg0, typename... _Args>
CONSTEXPR PTypeTraits Traits(TType< TTuple<_Arg0, _Args...> >) {
    return MakeStaticType< TStdTupleTraits<_Arg0, _Args...>, TTuple<_Arg0, _Args...> >();
}
//----------------------------------------------------------------------------
template <typename... _Args>
FAtom TStdTupleTraits<_Args...>::At(void* data, size_t index) const NOEXCEPT {
    Assert(data);

    return GetNth_(*static_cast<value_type*>(data), index, std::index_sequence_for<_Args...>{});
}
//----------------------------------------------------------------------------
template <typename... _Args>
bool TStdTupleTraits<_Args...>::ForEach(void* data, const foreach_fun& foreach) const NOEXCEPT {
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
FAtom TStdTupleTraits<_Args...>::GetNth_(value_type& tuple, size_t index, std::index_sequence<_Idx...>) NOEXCEPT {
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