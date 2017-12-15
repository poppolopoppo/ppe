#pragma once

#include "Core/Core.h"

#include <iosfwd>
#include <utility>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
using TPair = std::pair<_First, _Second>;
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
FORCE_INLINE TPair< Meta::TRemoveReference<_First>, Meta::TRemoveReference<_Second> > MakePair(_First&& first, _Second&& second) {
    typedef TPair< Meta::TRemoveReference<_First>, Meta::TRemoveReference<_Second> > pair_type;
    return pair_type(std::forward<_First>(first), std::forward<_Second>(second));
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
FORCE_INLINE hash_t hash_value(const TPair<_First, _Second>& pair) {
    using Core::hash_value;
    return hash_tuple(hash_value(pair.first), hash_value(pair.second));
}
//----------------------------------------------------------------------------
namespace Meta {
template <typename _First, typename _Second>
struct TIsPod< TPair<_First, _Second> >
    : public std::integral_constant<bool,
        Meta::TIsPod<_First>::value &&
        Meta::TIsPod<_Second>::value >
{};
template <typename _First, typename _Second>
TPair<_First, _Second> ForceInitType(TType< TPair<_First, _Second> >) {
    return MakePair(ForceInit<_First>(), ForceInit<_Second>());
}
template <typename _First, typename _Second>
void Construct(TPair<_First, _Second>* p, FForceInit) {
    Construct(p, ForceInit<_First>(), ForceInit<_Second>());
}
} //!Meta
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Char,  typename _Traits,
    typename _First, typename _Second
>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const TPair<_First, _Second>& pair) {
    return oss << "( " << pair.first << ", " << pair.second << " )";
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
