#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/MetaType.h"

#include <type_traits>

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
class MetaAtom;
class MetaObject;
class MetaProperty;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _From, typename _To>
struct MetaTypePromote {
    typedef std::false_type enabled;
};
//----------------------------------------------------------------------------
template <typename T>
struct MetaTypePromote<T, T> {
    typedef std::true_type enabled;
    bool operator ()(T* dst, T&& rvalue) const { *dst = std::move(rvalue); return true; }
    bool operator ()(T* dst, const T& value) const { *dst = value; return true; }
};
//----------------------------------------------------------------------------
template <typename _From, typename _To>
constexpr bool PromotionAvailable() {
    typedef typename MetaTypePromote<_From, _To>::enabled enabled_type;
    return enabled_type::value;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool PromoteMove(MetaAtom *dst, MetaAtom *src);
bool PromoteCopy(MetaAtom *dst, const MetaAtom *src);
//----------------------------------------------------------------------------
bool PromoteMove(MetaAtom *dst, void *src, MetaTypeId srcTypeId);
bool PromoteCopy(MetaAtom *dst, const void *src, MetaTypeId srcTypeId);
//----------------------------------------------------------------------------
bool PromoteMove(MetaTypeId dstTypeId, void *dst, MetaAtom *src);
bool PromoteCopy(MetaTypeId dstTypeId, void *dst, const MetaAtom *src);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
bool PromoteMove(MetaAtom *dst, T *src);
//----------------------------------------------------------------------------
template <typename T>
bool PromoteCopy(MetaAtom *dst, const T *src);
//----------------------------------------------------------------------------
template <typename T>
bool PromoteMove(T *dst, MetaAtom *src);
//----------------------------------------------------------------------------
template <typename T>
bool PromoteCopy(T *dst, const MetaAtom *src);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

#include "Core.RTTI/MetaTypePromote-inl.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _To, typename _From>
typename std::enable_if<
    PromotionAvailable<_From, _To>(),
    bool
>::type PromoteMove(_To& dst, _From&& src) {
    return MetaTypePromote<_From, _To>()(&dst, std::move(src));
}
//----------------------------------------------------------------------------
template <typename _To, typename _From>
typename std::enable_if<
    PromotionAvailable<_From, _To>(),
    bool
>::type PromoteCopy(_To& dst, const _From& src) {
    return MetaTypePromote<_From, _To>()(&dst, src);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
