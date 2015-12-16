#include "stdafx.h"

#include "MetaClassDatabase.h"

#include "MetaClass.h"
#include "MetaObject.h"

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
    Assert(MetaObject::MetaClass::Instance()->IsAssignableFrom(metaClass));

    WRITESCOPELOCK(_barrier);
    const MetaClass *& slot = _classes[name];

    if (slot)
        Assert(allowOverride);

    slot = metaClass;
}
//----------------------------------------------------------------------------
void MetaClassHashMap::Remove(const MetaClassName& name, const MetaClass *metaClass) {
    Assert(metaClass);
    Assert(!metaClass->Name().empty());

    WRITESCOPELOCK(_barrier);
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

    READSCOPELOCK(_barrier);

    const auto it = _classes.find(name);
    return (_classes.end() == it) ? nullptr : it->second;
}
//----------------------------------------------------------------------------
void MetaClassHashMap::Clear() {
    WRITESCOPELOCK(_barrier);

    _classes.clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
