#include "stdafx.h"

#include "MetaClassDatabase.h"

#include "MetaClass.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
MetaClassHashMap::MetaClassHashMap() {
    _classes.reserve(128);
}
//----------------------------------------------------------------------------
MetaClassHashMap::~MetaClassHashMap() {}
//----------------------------------------------------------------------------
void MetaClassHashMap::Add(const MetaClassName& name, const MetaClass *metaClass, bool allowOverride) {
    Assert(metaClass);
    Assert(!metaClass->Name().empty());

    std::lock_guard<std::mutex> scopeLock(_barrier);
    const MetaClass *& slot = _classes[name];

    if (slot)
        Assert(allowOverride);

    slot = metaClass;
}
//----------------------------------------------------------------------------
void MetaClassHashMap::Remove(const MetaClassName& name, const MetaClass *metaClass) {
    Assert(metaClass);
    Assert(!metaClass->Name().empty());

    std::lock_guard<std::mutex> scopeLock(_barrier);
    const auto it = _classes.find(name);

    Assert(_classes.end() != it);
    Assert(it->second == metaClass);

    _classes.erase(it);
}
//----------------------------------------------------------------------------
void MetaClassHashMap::Add(const MetaClass *metaClass) {
    Assert(metaClass);

    Add(metaClass->Name(), metaClass, false);
}
//----------------------------------------------------------------------------
void MetaClassHashMap::Remove(const MetaClass *metaClass) {
    Assert(metaClass);

    Remove(metaClass->Name(), metaClass);
}
//----------------------------------------------------------------------------
const MetaClass *MetaClassHashMap::GetIFP(const MetaClassName& name) const {
    Assert(!name.empty());

    std::lock_guard<std::mutex> scopeLock(_barrier);
    const auto it = _classes.find(name);
    return (_classes.end() == it) ? nullptr : it->second;
}
//----------------------------------------------------------------------------
void MetaClassHashMap::Clear() {
    std::lock_guard<std::mutex> scopeLock(_barrier);
    _classes.clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
