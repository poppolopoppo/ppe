#include "stdafx.h"

#include "MetaAtomDatabase.h"

#include "MetaAtom.h"
#include "MetaObject.h"
#include "Typedefs.h"

#include "Core/IO/Format.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaAtomHashMap::FMetaAtomHashMap() {
    _objects.reserve(128);
}
//----------------------------------------------------------------------------
FMetaAtomHashMap::~FMetaAtomHashMap() {
    Assert(_objects.empty());
}
//----------------------------------------------------------------------------
void FMetaAtomHashMap::Add(const FName& name, FMetaAtom *metaAtom, bool allowOverride) {
    UNUSED(allowOverride);
    Assert(!name.empty());
    Assert(metaAtom);

    WRITESCOPELOCK(_barrier);
    PMetaAtom& slot = _objects[name];

    if (slot)
        Assert(allowOverride);

    slot = metaAtom;
}
//----------------------------------------------------------------------------
void FMetaAtomHashMap::Remove(const FName& name, FMetaAtom *metaAtom) {
    UNUSED(metaAtom);
    Assert(!name.empty());
    Assert(metaAtom);

    WRITESCOPELOCK(_barrier);

    const auto it = _objects.find(name);
    Assert(_objects.end() != it);
    Assert(it->second == metaAtom);

    _objects.erase(it);
}
//----------------------------------------------------------------------------
void FMetaAtomHashMap::Add(FMetaObject *metaObject) {
    Assert(metaObject);
    Assert(metaObject->RTTI_IsExported());

    const FName& name = metaObject->RTTI_Name();
    Assert(!name.empty());

    WRITESCOPELOCK(_barrier);

    _objects[name] = MakeAtom(PMetaObject(metaObject));
}
//----------------------------------------------------------------------------
void FMetaAtomHashMap::Remove(FMetaObject *metaObject) {
    Assert(metaObject);
    Assert(metaObject->RTTI_IsExported());

    const FName& name = metaObject->RTTI_Name();
    Assert(!name.empty());

    WRITESCOPELOCK(_barrier);

    const auto it = _objects.find(name);
    Assert(_objects.end() != it);
    Assert(it->second->Cast<PMetaObject>()->Wrapper() == metaObject);

    _objects.erase(it);
}
//----------------------------------------------------------------------------
FMetaAtom *FMetaAtomHashMap::GetIFP(const FName& name) const {
    Assert(!name.empty());

    READSCOPELOCK(_barrier);

    const auto it = _objects.find(name);

    return (_objects.end() == it) ? nullptr : it->second.get();
}
//----------------------------------------------------------------------------
void FMetaAtomHashMap::Clear() {
    WRITESCOPELOCK(_barrier);

    _objects.clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
