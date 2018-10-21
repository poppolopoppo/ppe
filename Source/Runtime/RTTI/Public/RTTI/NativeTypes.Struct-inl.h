#pragma once

#include "RTTI/NativeTypes.h"

#include "RTTI/NativeTypes.Tuple-inl.h" // TBaseTupleTraits<...>

#include "Container/Tuple.h"

// need C++17 structured bindings
#define USE_PPE_RTTI_STRUCT_AS_TUPLE _HAS_CXX17

#if USE_PPE_RTTI_STRUCT_AS_TUPLE

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TStructAsTupleTraits<T, TTuple<_Args>>
//----------------------------------------------------------------------------
template <typename T, typename _Tuple>
class TStructAsTupleTraits;
template <typename T, typename... _Args>
class TStructAsTupleTraits<T, TTuple<_Args&...>> final : public TBaseTypeTraits<T, TBaseTupleTraits<_Args...>> {
    using base_traits = TBaseTypeTraits<T, TBaseTupleTraits<_Args...>>;
    using typename base_traits::value_type;
    using typename base_traits::pointer;
    using typename base_traits::const_pointer;

public: // ITupleTraits
    using typename base_traits::foreach_fun;

    virtual FAtom At(void* data, size_t index) const override final;

    virtual bool ForEach(void* data, const foreach_fun& foreach) const override final;

private:
    template <size_t... _Idx>
    static FAtom GetNth_(TTuple<_Args&...>& refs, size_t index, std::index_sequence<_Idx...>);
};
//----------------------------------------------------------------------------
template <typename T, typename _Tuple = decltype(tie_as_tuple(std::declval<T&>())) >
PTypeTraits Traits(Meta::TType< T >) noexcept {
    return PTypeTraits::Make< TStructAsTupleTraits<T, _Tuple> >();
}
//----------------------------------------------------------------------------
template <typename T, typename... _Args>
FAtom TStructAsTupleTraits<T, TTuple<_Args&...> >::At(void* data, size_t index) const {
    Assert(data);

    TTuple<_Args&...> refs = tie_as_tuple(*reinterpret_cast<value_type*>(data));

    return GetNth_(refs, index, std::index_sequence_for<_Args...>{});
}
//----------------------------------------------------------------------------
template <typename T, typename... _Args>
bool TStructAsTupleTraits<T, TTuple<_Args&...> >::ForEach(void* data, const foreach_fun& foreach) const {
    Assert(data);

    TTuple<_Args&...> refs = tie_as_tuple(*reinterpret_cast<value_type*>(data));

    forrange(index, 0, sizeof...(_Args)) {
        const FAtom elt = GetNth_(refs, index, std::index_sequence_for<_Args...>{});
        if (not foreach(elt))
            return false; // not sure if it could be done statically due to this return
    }

    return true;
}
//----------------------------------------------------------------------------
template <typename T, typename... _Args>
template <size_t... _Idx>
FAtom TStructAsTupleTraits<T, TTuple<_Args&...> >::GetNth_(TTuple<_Args&...>& refs, size_t index, std::index_sequence<_Idx...>) {
    typedef FAtom(*getter_type)(TTuple<_Args&...>&);
    static const getter_type GGetNth[sizeof...(_Args)] = {
        [](TTuple<_Args&...>& val) { return MakeAtom(&std::get<_Idx>(val)); }...
    };

    Assert(index < sizeof...(_Idx));
    return (GGetNth[index](refs));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE

#endif //!USE_PPE_RTTI_STRUCT_AS_TUPLE
