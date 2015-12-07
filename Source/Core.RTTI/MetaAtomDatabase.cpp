#include "stdafx.h"

#include "MetaAtomDatabase.h"

#include "MetaAtom.h"
#include "MetaObject.h"

#include "Core/IO/Format.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
MetaAtomHashMap::MetaAtomHashMap() {
    _objects.reserve(128);
}
//----------------------------------------------------------------------------
MetaAtomHashMap::~MetaAtomHashMap() {
    Assert(_objects.empty());
}
//----------------------------------------------------------------------------
void MetaAtomHashMap::Add(const MetaObjectName& name, MetaAtom *metaAtom, bool allowOverride) {
    Assert(!name.empty());
    Assert(metaAtom);

    std::lock_guard<std::mutex> scopeLock(_barrier);
    PMetaAtom& slot = _objects[name];

    if (slot)
        Assert(allowOverride);

    slot = metaAtom;
}
//----------------------------------------------------------------------------
void MetaAtomHashMap::Remove(const MetaObjectName& name, MetaAtom *metaAtom) {
    Assert(!name.empty());
    Assert(metaAtom);

    std::lock_guard<std::mutex> scopeLock(_barrier);

    const auto it = _objects.find(name);
    Assert(_objects.end() != it);
    Assert(it->second == metaAtom);

    _objects.erase(it);
}
//----------------------------------------------------------------------------
void MetaAtomHashMap::Add(MetaObject *metaObject) {
    Assert(metaObject);

    const MetaObjectName& name = metaObject->RTTI_Name();
    Assert(!name.empty());

    std::lock_guard<std::mutex> scopeLock(_barrier);

    _objects[name] = MakeAtom(PMetaObject(metaObject));
}
//----------------------------------------------------------------------------
void MetaAtomHashMap::Remove(MetaObject *metaObject) {
    Assert(metaObject);

    const MetaObjectName& name = metaObject->RTTI_Name();
    Assert(!name.empty());

    std::lock_guard<std::mutex> scopeLock(_barrier);

    const auto it = _objects.find(name);
    Assert(_objects.end() != it);
    Assert(it->second->Cast<PMetaObject>()->Wrapper() == metaObject);

    _objects.erase(it);
}
//----------------------------------------------------------------------------
MetaAtom *MetaAtomHashMap::GetIFP(const MetaObjectName& name) const {
    Assert(!name.empty());

    std::lock_guard<std::mutex> scopeLock(_barrier);

    const auto it = _objects.find(name);

    return (_objects.end() == it) ? nullptr : it->second.get();
}
//----------------------------------------------------------------------------
void MetaAtomHashMap::Clear() {
    std::lock_guard<std::mutex> scopeLock(_barrier);

    _objects.clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
