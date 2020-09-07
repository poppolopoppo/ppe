#include "stdafx.h"

#include "RTTI/OpaqueData.h"

#include "RTTI/Atom.h"
#include "RTTI/AtomVisitor.h"
#include "RTTI/NativeTypes.h"
#include "RTTI/Typedefs.h"
#include "MetaClass.h"
#include "MetaObject.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(sizeof(FAny) == details::_sizeof_FAny);
//----------------------------------------------------------------------------
FAny& GetOpaqueData(FOpaqueData* opaque, const FName& name) NOEXCEPT {
    Assert(opaque);
    Assert(not name.empty());

    return opaque->Get(name);
}
//----------------------------------------------------------------------------
const FAny* GetOpaqueDataIFP(const FOpaqueData* opaque, const FName& name) NOEXCEPT {
    Assert(opaque);
    Assert(not name.empty());

    return opaque->GetIFP(name);
}
//----------------------------------------------------------------------------
bool TryGetOpaqueData(const FOpaqueData* opaque, const FName& name, const FAtom& dst) NOEXCEPT {
    Assert(opaque);
    Assert(not name.empty());

    if (const FAny* const pData = opaque->GetIFP(name)) {
        if (pData->InnerAtom().PromoteCopy(dst))
            return true;
    }

    return false;
}
//----------------------------------------------------------------------------
void SetOpaqueDataCopy(FOpaqueData* opaque, const FName& name, const FAtom& dst) {
    Assert(opaque);
    Assert(not name.empty());
    Assert(dst);

    opaque->Add(name).AssignCopy(dst);
}
//----------------------------------------------------------------------------
void SetOpaqueDataMove(FOpaqueData* opaque, const FName& name, const FAtom& dst) {
    Assert(opaque);
    Assert(not name.empty());
    Assert(dst);

    opaque->Add(name).AssignMove(dst);
}
//----------------------------------------------------------------------------
void UnsetOpaqueData(FOpaqueData* opaque, const FName& name) {
    Assert(opaque);
    Assert(not name.empty());

    opaque->Remove_AssertExists(name);
}
//----------------------------------------------------------------------------
void MergeOpaqueData(FOpaqueData* dst, const FOpaqueData& src, bool allowOverride/* = false */) {
    Assert(dst);

    // #TODO: merge list and dictionaries ?

    for (const TPair<FName, FAny>& it : src) {
        Assert(not it.first.empty());
        Assert(it.second);

        bool added = false;
        FAny& data = dst->FindOrAdd(it.first, &added)->second;

        if (allowOverride | added)
            data.AssignCopy(it.second.InnerAtom());
    }
}
//----------------------------------------------------------------------------
#define PPE_RTTI_OPAQUEDATA_NATIVETYPE_DEF(_Name, _Type, _Uid) \
    bool TryGetOpaqueData(const FOpaqueData* opaque, const FName& name, _Type* dst) NOEXCEPT { \
        return TryGetOpaqueData(opaque, name, MakeAtom(dst)); \
    } \
    void SetOpaqueData(FOpaqueData* opaque, const FName& name, const _Type& value) { \
        return SetOpaqueDataCopy(opaque, name, MakeAtom(const_cast<_Type*>(&value))); \
    } \
    void SetOpaqueData(FOpaqueData* opaque, const FName& name, _Type&& value) { \
        return SetOpaqueDataMove(opaque, name, MakeAtom(const_cast<_Type*>(&value))); \
    }
FOREACH_RTTI_NATIVETYPES(PPE_RTTI_OPAQUEDATA_NATIVETYPE_DEF)
#undef PPE_RTTI_OPAQUEDATA_NATIVETYPE_DEF
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
