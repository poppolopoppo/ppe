#include "stdafx.h"

#include "MetaTypeVirtualTraits.h"

#include "MetaTypePromote.h"
#include "Atom/MetaAtom.h"
#include "Property/MetaProperty.h"

#include "Core/Meta/OneTimeInitialize.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const MetaTypeScalarTraits *MetaTypeScalarTraits::Instance() {
    ONE_TIME_DEFAULT_INITIALIZE(const MetaTypeScalarTraits, gInstance);
    return &gInstance;
}
//----------------------------------------------------------------------------
bool MetaTypeScalarTraits::AssignMove(MetaAtom *dst, MetaAtom *src) const {
    return PromoteMove(dst, src);
}
//----------------------------------------------------------------------------
bool MetaTypeScalarTraits::AssignCopy(MetaAtom *dst, const MetaAtom *src) const {
    return PromoteCopy(dst, src);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const MetaTypePairTraits *MetaTypePairTraits::Instance() {
    ONE_TIME_DEFAULT_INITIALIZE(const MetaTypePairTraits, gInstance);
    return &gInstance;
}
//----------------------------------------------------------------------------
bool MetaTypePairTraits::AssignMove(MetaAtom *dst, MetaAtom *src) const {
    Assert(dst);
    Assert(src);

    IMetaAtomPair *const dst_pair = dst->AsPair();
    IMetaAtomPair *const src_pair = src->AsPair();
    Assert(dst_pair || src_pair);
    if (!(dst_pair && src_pair))
        return false;

    RTTI::Pair<PMetaAtom, PMetaAtom> tmp;
    src_pair->WrapMoveTo(tmp);

    if (dst_pair->UnwrapMoveFrom(tmp))
        return true;

    src_pair->UnwrapMoveFrom(tmp);

    return false;
}
//----------------------------------------------------------------------------
bool MetaTypePairTraits::AssignCopy(MetaAtom *dst, const MetaAtom *src) const {
    Assert(dst);
    Assert(src);

    IMetaAtomPair *const dst_pair = dst->AsPair();
    const IMetaAtomPair *src_pair = src->AsPair();
    Assert(dst_pair || src_pair);
    if (!(dst_pair && src_pair))
        return false;

    RTTI::Pair<PMetaAtom, PMetaAtom> tmp;
    src_pair->WrapCopyTo(tmp);

    return dst_pair->UnwrapMoveFrom(tmp);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const MetaTypeVectorTraits *MetaTypeVectorTraits::Instance() {
    ONE_TIME_DEFAULT_INITIALIZE(const MetaTypeVectorTraits, gInstance);
    return &gInstance;
}
//----------------------------------------------------------------------------
bool MetaTypeVectorTraits::AssignMove(MetaAtom *dst, MetaAtom *src) const {
    Assert(dst);
    Assert(src);

    IMetaAtomVector *const dst_vector = dst->AsVector();
    IMetaAtomVector *const src_vector = src->AsVector();
    Assert(dst_vector || src_vector);
    if (!(dst_vector && src_vector))
        return false;

    RTTI::Vector<PMetaAtom> tmp;
    src_vector->WrapMoveTo(tmp);

    if (dst_vector->UnwrapMoveFrom(tmp))
        return true;

    src_vector->MoveFrom(tmp);

    return false;
}
//----------------------------------------------------------------------------
bool MetaTypeVectorTraits::AssignCopy(MetaAtom *dst, const MetaAtom *src) const {
    Assert(dst);
    Assert(src);

    IMetaAtomVector *const dst_vector = dst->AsVector();
    const IMetaAtomVector *src_vector = src->AsVector();
    Assert(dst_vector || src_vector);
    if (!(dst_vector && src_vector))
        return false;

    RTTI::Vector<PMetaAtom> tmp;
    src_vector->WrapCopyTo(tmp);

    return dst_vector->UnwrapMoveFrom(tmp);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const MetaTypeDictionaryTraits *MetaTypeDictionaryTraits::Instance() {
    ONE_TIME_DEFAULT_INITIALIZE(const MetaTypeDictionaryTraits, gInstance);
    return &gInstance;
}
//----------------------------------------------------------------------------
bool MetaTypeDictionaryTraits::AssignMove(MetaAtom *dst, MetaAtom *src) const {
    Assert(dst);
    Assert(src);

    IMetaAtomDictionary *const dst_dict = dst->AsDictionary();
    IMetaAtomDictionary *const src_dict = src->AsDictionary();
    Assert(dst_dict || src_dict);
    if (!(dst_dict && src_dict))
        return false;

    RTTI::Dictionary<PMetaAtom, PMetaAtom> tmp;
    src_dict->WrapMoveTo(tmp);

    if (dst_dict->UnwrapMoveFrom(tmp))
        return true;

    src_dict->UnwrapMoveFrom(tmp);

    return false;
}
//----------------------------------------------------------------------------
bool MetaTypeDictionaryTraits::AssignCopy(MetaAtom *dst, const MetaAtom *src) const {
    Assert(dst);
    Assert(src);

    IMetaAtomDictionary *const dst_dict = dst->AsDictionary();
    const IMetaAtomDictionary *src_dict = src->AsDictionary();
    Assert(dst_dict || src_dict);
    if (!(dst_dict && src_dict))
        return false;

    RTTI::Dictionary<PMetaAtom, PMetaAtom> tmp;
    src_dict->WrapCopyTo(tmp);

    return dst_dict->UnwrapMoveFrom(tmp);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool AssignMove(MetaAtom *dst, MetaAtom *src) {
    Assert(dst);
    Assert(dst->Traits());
    return dst->Traits()->AssignMove(dst, src);
}
//----------------------------------------------------------------------------
bool AssignCopy(MetaAtom *dst, const MetaAtom *src) {
    Assert(dst);
    Assert(dst->Traits());
    return dst->Traits()->AssignCopy(dst, src);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
