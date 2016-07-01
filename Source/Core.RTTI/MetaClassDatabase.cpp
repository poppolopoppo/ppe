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
MetaClassHashMap::~MetaClassHashMap() {
    Clear();
}
//----------------------------------------------------------------------------
void MetaClassHashMap::Add(const MetaClass *metaClass) {
    Assert(metaClass);
    Assert(!metaClass->Name().empty());

    WRITESCOPELOCK(_barrier);
    const MetaClass *& slot = _classes[metaClass->Name()];

    AssertRelease(nullptr == slot);
    slot = metaClass;
}
//----------------------------------------------------------------------------
void MetaClassHashMap::Remove(const MetaClass *metaClass) {
    Assert(metaClass);
    Assert(!metaClass->Name().empty());

    WRITESCOPELOCK(_barrier);
    const auto it = _classes.find(metaClass->Name());

    AssertRelease(_classes.end() != it);
    Assert(it->second == metaClass);

    _classes.erase(it);
}
//----------------------------------------------------------------------------
const MetaClass *MetaClassHashMap::GetIFP(const RTTI::Name& name) const {
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
