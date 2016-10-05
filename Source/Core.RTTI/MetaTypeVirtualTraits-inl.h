#pragma once

#include "Core.RTTI/MetaTypeVirtualTraits.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
bool AssignMove(T *dst, FMetaAtom *src) {
    Assert(dst);
    Assert(src);

    typedef typename std::decay< T >::type dst_type;

    typename TMetaAtomWrapper< dst_type >::type tmp;
    if (!TMetaTypeTraits< dst_type >::VirtualTraits()->AssignMove(&tmp, src))
        return false;

    TMetaTypeTraits< dst_type >::UnwrapMove(*dst, std::move(tmp.Wrapper()));
    return true;
}
//----------------------------------------------------------------------------
template <typename T>
bool AssignCopy(T *dst, const FMetaAtom *src) {
    Assert(dst);
    Assert(src);

    typedef typename std::decay< T >::type dst_type;

    typename TMetaAtomWrapper< dst_type >::type tmp;
    if (!TMetaTypeTraits< dst_type >::VirtualTraits()->AssignCopy(&tmp, src))
        return false;

    TMetaTypeTraits< dst_type >::UnwrapMove(*dst, std::move(tmp.Wrapper()));
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
bool AssignMove(FMetaAtom *dst, T *src) {
    Assert(dst);
    Assert(src);

    typedef typename std::decay< T >::type src_type;

    typename TMetaAtomWrapper< src_type >::type tmp;
    TMetaTypeTraits< src_type >::WrapMove(tmp.Wrapper(), std::move(*src));

    if (dst->Traits()->AssignMove(dst, &tmp))
        return true;

    TMetaTypeTraits< src_type >::UnwrapMove(*src, std::move(tmp.Wrapper()));
    return false;
}
//----------------------------------------------------------------------------
template <typename T>
bool AssignCopy(FMetaAtom *dst, const T *src) {
    Assert(dst);
    Assert(src);

    typedef typename std::decay< T >::type src_type;

    typename TMetaAtomWrapper< src_type >::type tmp;
    TMetaTypeTraits< src_type >::WrapCopy(tmp.Wrapper(), *src);

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

    typename TMetaAtomWrapper< src_type >::type tmp_src;
    TMetaTypeTraits< src_type >::WrapMove(tmp_src.Wrapper(), std::move(*src));

    typename TMetaAtomWrapper< dst_type >::type tmp_dst;
    if (TMetaTypeTraits< dst_type >::VirtualTraits()->AssignMove(&tmp_dst, &tmp_src)) {
        TMetaTypeTraits< dst_type >::UnwrapMove(*dst, std::move(tmp_dst.Wrapper()));
        return true;
    }
    else {
        TMetaTypeTraits< src_type >::UnwrapMove(*src, std::move(tmp_src.Wrapper()));
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

    typename TMetaAtomWrapper< src_type >::type tmp_src;
    TMetaTypeTraits< src_type >::WrapCopy(tmp_src.Wrapper(), *src);

    typename TMetaAtomWrapper< dst_type >::type tmp_dst;
    if (!TMetaTypeTraits< dst_type >::VirtualTraits()->AssignMove(&tmp_dst, &tmp_src))
        return false;

    TMetaTypeTraits< dst_type >::UnwrapMove(*dst, std::move(tmp_dst.Wrapper()));
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
