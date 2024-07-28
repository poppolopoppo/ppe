#pragma once

#include "Core.h"

#include "Container/Hash.h"
#include "IO/TextWriter_fwd.h"

#include <tuple>
#include <type_traits>
#include <utility>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T, u32 , class = void>
struct TCompressedPairElement {
    using reference = T&;
    using const_reference = const T&;

    template <typename _Dummy = T, class = Meta::TEnableIf<std::is_default_constructible_v<_Dummy>> >
    CONSTEXPR TCompressedPairElement() NOEXCEPT_IF(std::is_nothrow_default_constructible_v<T>)
    {}

    template <typename _Arg, class = Meta::TEnabledIf<!std::is_same_v<std::remove_cv_t<std::remove_reference_t<_Arg>>, TCompressedPairElement>> >
    CONSTEXPR TCompressedPairElement(_Arg&& arg) NOEXCEPT_IF(std::is_nothrow_constructible_v<T, _Arg>)
    :   _value{std::forward<_Arg>(arg)}
    {}

    template <typename... _Args, size_t... _Index>
    CONSTEXPR TCompressedPairElement(std::tuple<_Args...> args, std::index_sequence<_Index...>) NOEXCEPT_IF(std::is_nothrow_constructible_v<T, _Args...>)
    :   _value{std::forward<_Args>(std::get<_Index>(args))...}
    {}

    NODISCARD CONSTEXPR reference get() NOEXCEPT {
        return _value;
    }

    NODISCARD CONSTEXPR const_reference get() const NOEXCEPT {
        return _value;
    }

private:
    T _value{};
};
//----------------------------------------------------------------------------
template <typename T, u32 _Tag>
struct TCompressedPairElement<T, _Tag, Meta::TEnableIf<Meta::is_ebco_eligible_v<T>>> : T {
    using base_type = T;
    using reference = T&;
    using const_reference = const T&;

    template <typename _Dummy = T, class = Meta::TEnableIf<std::is_default_constructible_v<_Dummy>> >
    CONSTEXPR TCompressedPairElement() NOEXCEPT_IF(std::is_nothrow_default_constructible_v<base_type>)
    :   base_type{}
    {}

    template <typename _Arg, class = Meta::TEnabledIf<!std::is_same_v<std::remove_cv_t<std::remove_reference_t<_Arg>>, TCompressedPairElement>> >
    CONSTEXPR TCompressedPairElement(_Arg&& arg) NOEXCEPT_IF(std::is_nothrow_constructible_v<base_type, _Arg>)
    :   base_type{std::forward<_Arg>(arg)}
    {}

    template <typename... _Args, size_t... _Index>
    CONSTEXPR TCompressedPairElement(std::tuple<_Args...> args, std::index_sequence<_Index...>) NOEXCEPT_IF(std::is_nothrow_constructible_v<base_type, _Args...>)
    :   base_type{std::forward<_Args>(std::get<_Index>(args))...}
    {}

    NODISCARD CONSTEXPR reference get() NOEXCEPT {
        return (*this);
    }

    NODISCARD CONSTEXPR const_reference get() const NOEXCEPT {
        return (*this);
    }

private:
    T _value{};
};
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
class TCompressedPair final
    :   details::TCompressedPairElement<_First, 0u>
    ,   details::TCompressedPairElement<_Second, 1u> {
    using first_elt_t = details::TCompressedPairElement<_First, 0u>;
    using second_elt_t = details::TCompressedPairElement<_Second, 1u>;

public:
    using first_type = _First;
    using second_type = _Second;

    template <bool _Dummy = true, class = Meta::TEnableIf<_Dummy && std::is_default_constructible_v<first_type> && std::is_default_constructible_v<second_type>>>
    CONSTEXPR TCompressedPair() NOEXCEPT_IF(std::is_nothrow_default_constructible_v<first_elt_t> && std::is_nothrow_default_constructible_v<second_elt_t>)
    :   first_elt_t{}
    ,   second_elt_t{}
    {}

    CONSTEXPR TCompressedPair(const TCompressedPair& other) NOEXCEPT_IF(std::is_nothrow_copy_constructible_v<first_elt_t> && std::is_nothrow_copy_constructible_v<second_elt_t>) = default;
    CONSTEXPR TCompressedPair(TCompressedPair&& rvalue) NOEXCEPT_IF(std::is_nothrow_move_constructible_v<first_elt_t> && std::is_nothrow_move_constructible_v<second_elt_t>) = default;

    CONSTEXPR TCompressedPair& operator =(const TCompressedPair& other) NOEXCEPT_IF(std::is_nothrow_copy_assignable_v<first_elt_t> && std::is_nothrow_copy_assignable_v<second_elt_t>) = default;
    CONSTEXPR TCompressedPair& operator =(TCompressedPair&& rvalue) NOEXCEPT_IF(std::is_nothrow_move_assignable_v<first_elt_t> && std::is_nothrow_move_assignable_v<second_elt_t>) = default;

    template <typename _Arg, typename _Other>
    CONSTEXPR TCompressedPair(_Arg&& arg, _Other&& other) NOEXCEPT_IF(std::is_nothrow_constructible_v<first_elt_t, _Arg> && std::is_nothrow_constructible_v<second_elt_t, _Other>)
    :   first_elt_t{std::forward<_Arg>(arg)}
    ,   second_elt_t{std::forward<_Other>(other)}
    {}

    template <typename... _Args, typename... _Other>
    CONSTEXPR TCompressedPair(std::piecewise_construct_t,
        std::tuple<_Args...> args,
            std::tuple<_Other...> other)  NOEXCEPT_IF(std::is_nothrow_constructible_v<first_elt_t, _Args...> && std::is_nothrow_constructible_v<second_elt_t, _Other...>)
    :   first_elt_t{std::move(args), std::index_sequence_for<_Args...>{}}
    ,   second_elt_t{std::move(other), std::index_sequence_for<_Other...>{}}
    {}

    ~TCompressedPair() NOEXCEPT_IF(std::is_nothrow_destructible_v<first_type> && std::is_nothrow_destructible_v<second_type>) = default;

    NODISCARD CONSTEXPR first_type& first() NOEXCEPT {
        return static_cast<first_elt_t*>(this)->get();
    }
    NODISCARD CONSTEXPR const first_type& first() const NOEXCEPT {
        return static_cast<const first_elt_t*>(this)->get();
    }

    NODISCARD CONSTEXPR second_type& second() NOEXCEPT {
        return static_cast<second_elt_t*>(this)->get();
    }
    NODISCARD CONSTEXPR const second_type& second() const NOEXCEPT {
        return static_cast<const second_elt_t*>(this)->get();
    }

    template <u32 _Index>
    NODISCARD CONSTEXPR decltype(auto) get() NOEXCEPT {
        STATIC_ASSERT(_Index < 2);
        IF_CONSTEXPR(_Index == 0u)
            return first();
        else
            return second();
    }

    template <u32 _Index>
    NODISCARD CONSTEXPR decltype(auto) get() const NOEXCEPT {
        STATIC_ASSERT(_Index < 2);
        IF_CONSTEXPR(_Index == 0u)
            return first();
        else
            return second();
    }

    CONSTEXPR void swap(TCompressedPair& other) NOEXCEPT_IF(std::is_nothrow_swappable_v<first_type> && std::is_nothrow_swappable_v<second_type>) {
        using std::swap;
        swap(first(), other.first());
        swap(second(), other.second());
    }
};
//----------------------------------------------------------------------------
template <typename T, typename _Other>
TCompressedPair(T&& , _Other&& ) -> TCompressedPair<Meta::TDecay<T>, Meta::TDecay<_Other>>;
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
inline CONSTEXPR void swap(TCompressedPair<_First, _Second>& lhs, TCompressedPair<_First, _Second>& rhs) {
    lhs.swap(rhs);
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
NODISCARD FORCE_INLINE hash_t hash_value(const TCompressedPair<_First, _Second>& pair) {
    using namespace PPE;
    return hash_tuple(pair.first(), pair.second());
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
NODISCARD CONSTEXPR bool is_pod_type(TCompressedPair<_First, _Second>*) NOEXCEPT {
    using namespace PPE;
    return ( Meta::is_pod_v<_First> && Meta::is_pod_v<_Second> );
}
//----------------------------------------------------------------------------
namespace Meta {
template <typename _First, typename _Second>
TCompressedPair<_First, _Second> NoInitType(TType< TCompressedPair<_First, _Second> >) {
    return TCompressedPair{ MakeNoInit<_First>(), MakeNoInit<_Second>() };
}
template <typename _First, typename _Second>
TCompressedPair<_First, _Second> ForceInitType(TType< TCompressedPair<_First, _Second> >) {
    return TCompressedPair{ MakeForceInit<_First>(), MakeForceInit<_Second>() };
}
template <typename _First, typename _Second>
void Construct(TCompressedPair<_First, _Second>* p, FNoInit) {
    Construct(p,
        MakeNoInit<_First>(),
        MakeNoInit<_Second>() );
}
template <typename _First, typename _Second>
void Construct(TCompressedPair<_First, _Second>* p, FForceInit) {
    Construct(p,
        MakeForceInit<_First>(),
        MakeForceInit<_Second>() );
}
} //!Meta
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
using TPair = std::pair<_First, _Second>;
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
FORCE_INLINE TPair< Meta::TRemoveReference<_First>, Meta::TRemoveReference<_Second> > MakePair(_First&& first, _Second&& second) {
    typedef TPair< Meta::TRemoveReference<_First>, Meta::TRemoveReference<_Second> > pair_type;
    return pair_type(
        std::forward<_First>(first),
        std::forward<_Second>(second) );
}
//----------------------------------------------------------------------------
namespace Meta {
template <typename _First, typename _Second>
TPair<_First, _Second> NoInitType(TType< TPair<_First, _Second> >) {
    return MakePair(MakeNoInit<_First>(), MakeNoInit<_Second>());
}
template <typename _First, typename _Second>
TPair<_First, _Second> ForceInitType(TType< TPair<_First, _Second> >) {
    return MakePair(MakeForceInit<_First>(), MakeForceInit<_Second>());
}
template <typename _First, typename _Second>
void Construct(TPair<_First, _Second>* p, FNoInit) {
    Construct(p,
        MakeNoInit<_First>(),
        MakeNoInit<_Second>() );
}
template <typename _First, typename _Second>
void Construct(TPair<_First, _Second>* p, FForceInit) {
    Construct(p,
        MakeForceInit<_First>(),
        MakeForceInit<_Second>() );
}
} //!Meta
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _First, typename _Second>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const TPair<_First, _Second>& pair) {
    return oss << STRING_LITERAL(_Char, "( ") << pair.first << STRING_LITERAL(_Char, ", ") << pair.second << STRING_LITERAL(_Char, " )");
}
//----------------------------------------------------------------------------
template <typename _Char, typename _First, typename _Second>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const TCompressedPair<_First, _Second>& pair) {
    return oss << STRING_LITERAL(_Char, "( ") << pair.first() << STRING_LITERAL(_Char, ", ") << pair.second() << STRING_LITERAL(_Char, " )");
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

namespace std {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
struct tuple_size<PPE::TCompressedPair<_First, _Second>> : integral_constant<size_t, 2u>
{};
//----------------------------------------------------------------------------
template <size_t _Index, typename _First, typename _Second>
struct tuple_element<_Index, PPE::TCompressedPair<_First, _Second>>
:   conditional<_Index == 0u, _First, _Second> {
    static_assert(_Index < 2u, "Index out of bounds");
};
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
NODISCARD FORCE_INLINE PPE::hash_t hash_value(const pair<_First, _Second>& pair) {
    using namespace PPE;
    return hash_tuple(pair.first, pair.second);
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
NODISCARD CONSTEXPR bool is_pod_type(pair<_First, _Second>*) NOEXCEPT {
    using namespace PPE;
    return ( Meta::is_pod_v<_First> && Meta::is_pod_v<_Second> );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
}
