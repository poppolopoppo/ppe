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
template <typename _First, typename _Second>
using TPair = std::pair<_First, _Second>;
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
FORCE_INLINE TPair< Meta::TRemoveReference<_First>, Meta::TRemoveReference<_Second> > MakePair(_First&& first, _Second&& second) {
    typedef TPair< Meta::TRemoveReference<_First>, Meta::TRemoveReference<_Second> > pair_type;
    return pair_type(
        std::forward<_First>(first),
        std::forward<_Second>(second));
}
//----------------------------------------------------------------------------
template <typename _Char, typename _First, typename _Second>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const TPair<_First, _Second>& pair) {
    return oss << STRING_LITERAL(_Char, "( ") << pair.first << STRING_LITERAL(_Char, ", ") << pair.second << STRING_LITERAL(_Char, " )");
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
        MakeNoInit<_Second>());
}
template <typename _First, typename _Second>
void Construct(TPair<_First, _Second>* p, FForceInit) {
    Construct(p,
        MakeForceInit<_First>(),
        MakeForceInit<_Second>());
}
} //!Meta
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

namespace std {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
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
    return (Meta::is_pod_v<_First> && Meta::is_pod_v<_Second>);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace std
