#pragma once

#include "Core.h"

#include "MetaType.h"

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
struct MetaTypePromote<T, T> : public std::unary_function<T&&, T>{
    typedef std::true_type enabled;
    T&& operator ()(T&& rvalue) const { return std::move(rvalue); }
    const T& operator ()(const T& value) const { return value; }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _From, typename _To>
void PromoteMove(_To& dst, _From&& src) {
    typedef MetaTypePromote<_From, _To> promote_type;
    static_assert(promote_type::enabled::value, "invalid meta promotion");
    dst = std::move(promote_type()(std::move(src)));
}
//----------------------------------------------------------------------------
template <typename _From, typename _To>
void PromoteCopy(_To& dst, const _From& src) {
    typedef MetaTypePromote<_From, _To> promote_type;
    static_assert(promote_type::enabled::value, "invalid meta promotion");
    dst = promote_type()(src);
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
bool PromoteMove(MetaAtom *dst, T *src) {
    static_assert(MetaType<T>::TypeId, "T is not a valid rtti type");
    return PromoteMove(dst, src, MetaType<T>::TypeId);
}
//----------------------------------------------------------------------------
template <typename T>
bool PromoteCopy(MetaAtom *dst, const T *src) {
    static_assert(MetaType<T>::TypeId, "T is not a valid rtti type");
    return PromoteCopy(dst, src, MetaType<T>::TypeId);
}
//----------------------------------------------------------------------------
template <typename T>
bool PromoteCopy(T *dst, MetaAtom *src) {
    static_assert(MetaType<T>::TypeId, "T is not a valid rtti type");
    return PromoteCopy(MetaType<T>::TypeId, dst, src);
}
//----------------------------------------------------------------------------
template <typename T>
bool PromoteCopy(T *dst, const MetaAtom *src) {
    static_assert(MetaType<T>::TypeId, "T is not a valid rtti type");
    return PromoteCopy(MetaType<T>::TypeId, dst, src);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

#include "MetaTypePromote-inl.h"
