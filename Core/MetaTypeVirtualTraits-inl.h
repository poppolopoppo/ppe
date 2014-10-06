#pragma once

#include "MetaTypeVirtualTraits.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
bool AssignMove(T *dst, MetaAtom *src) {
    Assert(dst);
    Assert(src);

    typedef typename std::decay< T >::type dst_type;

    MetaWrappedAtom< dst_type > tmp;
    if (!MetaTypeTraits< dst_type >::VirtualTraits()->AssignMove(&tmp, src))
        return false;

    MetaTypeTraits< dst_type >::UnwrapMove(*dst, std::move(tmp.Wrapper()));
    return true;
}
//----------------------------------------------------------------------------
template <typename T>
bool AssignCopy(T *dst, const MetaAtom *src) {
    Assert(dst);
    Assert(src);

    typedef typename std::decay< T >::type dst_type;

    MetaWrappedAtom< dst_type > tmp;
    if (!MetaTypeTraits< dst_type >::VirtualTraits()->AssignCopy(&tmp, src))
        return false;

    MetaTypeTraits< dst_type >::UnwrapMove(*dst, std::move(tmp.Wrapper()));
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
bool AssignMove(MetaAtom *dst, T *src) {
    Assert(dst);
    Assert(src);

    typedef typename std::decay< T >::type src_type;

    MetaWrappedAtom< src_type > tmp;
    MetaTypeTraits< src_type >::WrapMove(tmp.Wrapper(), std::move(*src));

    if (dst->Traits()->AssignMove(dst, &tmp))
        return true;

    MetaTypeTraits< src_type >::UnwrapMove(*src, std::move(tmp.Wrapper()));
    return false;
}
//----------------------------------------------------------------------------
template <typename T>
bool AssignCopy(MetaAtom *dst, const T *src) {
    Assert(dst);
    Assert(src);

    typedef typename std::decay< T >::type src_type;

    MetaWrappedAtom< src_type > tmp;
    MetaTypeTraits< src_type >::WrapCopy(tmp.Wrapper(), *src);

    return dst->Traits()->AssignMove(dst, &tmp);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
bool AssignMove(T *dst, T *src) {
    Assert(dst);
    Assert(src);

    *dst = std::move(*src); // duh, trivial case
    return true;
}
//----------------------------------------------------------------------------
template <typename T>
bool AssignCopy(T *dst, const T *src) {
    Assert(dst);
    Assert(src);

    *dst = *src; // duh, trivial case
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Dst, typename _Src>
bool AssignMove(_Dst *dst, _Src *src) {
    Assert(dst);
    Assert(src);

    typedef typename std::decay< _Dst >::type dst_type;
    typedef typename std::decay< _Src >::type src_type;

    MetaWrappedAtom< src_type > tmp_src;
    MetaTypeTraits< src_type >::WrapMove(tmp_src.Wrapper(), std::move(*src));

    MetaTypeTraits< dst_type > tmp_dst;
    if (MetaTypeTraits< dst_type >::VirtualTraits()->AssignMove(&tmp_dst, &tmp_src)) {
        MetaTypeTraits< dst_type >::UnwrapMove(*dst, std::move(tmp_dst.Wrapper()));
        return true;
    }
    else {
        MetaTypeTraits< src_type >::UnwrapMove(*src, std::move(tmp_src.Wrapper()));
        return false;
    }
}
//----------------------------------------------------------------------------
template <typename _Dst, typename _Src>
bool AssignCopy(_Dst *dst, const _Src *src) {
    Assert(dst);
    Assert(src);

    typedef typename std::decay< _Dst >::type dst_type;
    typedef typename std::decay< _Src >::type src_type;

    MetaWrappedAtom< src_type > tmp_src;
    MetaTypeTraits< src_type >::WrapCopy(tmp_src.Wrapper(), *src);

    MetaTypeTraits< dst_type > tmp_dst;
    if (!MetaTypeTraits< dst_type >::VirtualTraits()->AssignMove(&tmp_dst, &tmp_src))
        return false;

    MetaTypeTraits< dst_type >::UnwrapMove(*dst, std::move(tmp_dst.Wrapper()));
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
