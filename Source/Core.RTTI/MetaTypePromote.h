#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/MetaType.h"

#include <type_traits>

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
class FMetaAtom;
class FMetaObject;
class FMetaProperty;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _From, typename _To>
struct TMetaTypePromote {
    typedef std::false_type enabled;
};
//----------------------------------------------------------------------------
template <typename T>
struct TMetaTypePromote<T, T> {
    typedef std::true_type enabled;
    bool operator ()(T* dst, T&& rvalue) const { *dst = std::move(rvalue); return true; }
    bool operator ()(T* dst, const T& value) const { *dst = value; return true; }
};
//----------------------------------------------------------------------------
template <typename _From, typename _To>
constexpr bool PromotionAvailable() {
    typedef typename TMetaTypePromote<_From, _To>::enabled enabled_type;
    return enabled_type::value;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool PromoteMove(FMetaAtom *dst, FMetaAtom *src);
bool PromoteCopy(FMetaAtom *dst, const FMetaAtom *src);
//----------------------------------------------------------------------------
bool PromoteMove(FMetaAtom *dst, void *src, FMetaTypeId srcTypeId);
bool PromoteCopy(FMetaAtom *dst, const void *src, FMetaTypeId srcTypeId);
//----------------------------------------------------------------------------
bool PromoteMove(FMetaTypeId dstTypeId, void *dst, FMetaAtom *src);
bool PromoteCopy(FMetaTypeId dstTypeId, void *dst, const FMetaAtom *src);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
bool PromoteMove(FMetaAtom *dst, T *src);
//----------------------------------------------------------------------------
template <typename T>
bool PromoteCopy(FMetaAtom *dst, const T *src);
//----------------------------------------------------------------------------
template <typename T>
bool PromoteMove(T *dst, FMetaAtom *src);
//----------------------------------------------------------------------------
template <typename T>
bool PromoteCopy(T *dst, const FMetaAtom *src);
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
    return TMetaTypePromote<_From, _To>()(&dst, std::move(src));
}
//----------------------------------------------------------------------------
template <typename _To, typename _From>
typename std::enable_if<
    PromotionAvailable<_From, _To>(),
    bool
>::type PromoteCopy(_To& dst, const _From& src) {
    return TMetaTypePromote<_From, _To>()(&dst, src);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

#include "Core.RTTI/MetaTypePromote-inl.h"
