#include "stdafx.h"

#include "MetaTypeVirtualTraits.h"

#include "MetaTypePromote.h"
#include "MetaAtom.h"
#include "MetaProperty.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FAbstractMetaTypeScalarTraits::AssignMove(FMetaAtom *dst, FMetaAtom *src) const {
    return PromoteMove(dst, src);
}
//----------------------------------------------------------------------------
bool FAbstractMetaTypeScalarTraits::AssignCopy(FMetaAtom *dst, const FMetaAtom *src) const {
    return PromoteCopy(dst, src);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FAbstractMetaTypePairTraits::AssignMove(FMetaAtom *dst, FMetaAtom *src) const {
    Assert(dst);
    Assert(src);

    IMetaAtomPair *const dst_pair = dst->AsPair();
    IMetaAtomPair *const src_pair = src->AsPair();
    Assert(dst_pair || src_pair);
    if (!(dst_pair && src_pair))
        return false;

    RTTI::TPair<PMetaAtom, PMetaAtom> tmp;
    src_pair->WrapMoveTo(tmp);

    if (dst_pair->UnwrapMoveFrom(tmp))
        return true;

    src_pair->UnwrapMoveFrom(tmp);

    return false;
}
//----------------------------------------------------------------------------
bool FAbstractMetaTypePairTraits::AssignCopy(FMetaAtom *dst, const FMetaAtom *src) const {
    Assert(dst);
    Assert(src);

    IMetaAtomPair *const dst_pair = dst->AsPair();
    const IMetaAtomPair *src_pair = src->AsPair();
    Assert(dst_pair || src_pair);
    if (!(dst_pair && src_pair))
        return false;

    RTTI::TPair<PMetaAtom, PMetaAtom> tmp;
    src_pair->WrapCopyTo(tmp);

    return dst_pair->UnwrapMoveFrom(tmp);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FAbstractMetaTypeVectorTraits::AssignMove(FMetaAtom *dst, FMetaAtom *src) const {
    Assert(dst);
    Assert(src);

    IMetaAtomVector *const dst_vector = dst->AsVector();
    IMetaAtomVector *const src_vector = src->AsVector();
    Assert(dst_vector || src_vector);
    if (!(dst_vector && src_vector))
        return false;

    RTTI::TVector<PMetaAtom> tmp;
    src_vector->WrapMoveTo(tmp);

    if (dst_vector->UnwrapMoveFrom(tmp))
        return true;

    src_vector->MoveFrom(tmp);

    return false;
}
//----------------------------------------------------------------------------
bool FAbstractMetaTypeVectorTraits::AssignCopy(FMetaAtom *dst, const FMetaAtom *src) const {
    Assert(dst);
    Assert(src);

    IMetaAtomVector *const dst_vector = dst->AsVector();
    const IMetaAtomVector *src_vector = src->AsVector();
    Assert(dst_vector || src_vector);
    if (!(dst_vector && src_vector))
        return false;

    RTTI::TVector<PMetaAtom> tmp;
    src_vector->WrapCopyTo(tmp);

    return dst_vector->UnwrapMoveFrom(tmp);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FAbstractMetaTypeDictionaryTraits::AssignMove(FMetaAtom *dst, FMetaAtom *src) const {
    Assert(dst);
    Assert(src);

    IMetaAtomDictionary *const dst_dict = dst->AsDictionary();
    IMetaAtomDictionary *const src_dict = src->AsDictionary();
    Assert(dst_dict || src_dict);
    if (!(dst_dict && src_dict))
        return false;

    RTTI::TDictionary<PMetaAtom, PMetaAtom> tmp;
    src_dict->WrapMoveTo(tmp);

    if (dst_dict->UnwrapMoveFrom(tmp))
        return true;

    src_dict->UnwrapMoveFrom(tmp);

    return false;
}
//----------------------------------------------------------------------------
bool FAbstractMetaTypeDictionaryTraits::AssignCopy(FMetaAtom *dst, const FMetaAtom *src) const {
    Assert(dst);
    Assert(src);

    IMetaAtomDictionary *const dst_dict = dst->AsDictionary();
    const IMetaAtomDictionary *src_dict = src->AsDictionary();
    Assert(dst_dict || src_dict);
    if (!(dst_dict && src_dict))
        return false;

    RTTI::TDictionary<PMetaAtom, PMetaAtom> tmp;
    src_dict->WrapCopyTo(tmp);

    return dst_dict->UnwrapMoveFrom(tmp);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool AssignMove(FMetaAtom *dst, FMetaAtom *src) {
    Assert(dst);
    Assert(dst->Traits());
    return dst->Traits()->AssignMove(dst, src);
}
//----------------------------------------------------------------------------
bool AssignCopy(FMetaAtom *dst, const FMetaAtom *src) {
    Assert(dst);
    Assert(dst->Traits());
    return dst->Traits()->AssignCopy(dst, src);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
