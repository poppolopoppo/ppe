#include "stdafx.h"

#include "MetaClassDatabase.h"

#include "MetaClass.h"
#include "MetaObject.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaClassHashMap::FMetaClassHashMap() {
    _classes.reserve(128);
}
//----------------------------------------------------------------------------
FMetaClassHashMap::~FMetaClassHashMap() {
    Clear();
}
//----------------------------------------------------------------------------
void FMetaClassHashMap::Add(const FMetaClass *metaClass) {
    Assert(metaClass);
    Assert(!metaClass->Name().empty());

    WRITESCOPELOCK(_barrier);
    const FMetaClass *& slot = _classes[metaClass->Name()];

    AssertRelease(nullptr == slot);
    slot = metaClass;
}
//----------------------------------------------------------------------------
void FMetaClassHashMap::Remove(const FMetaClass *metaClass) {
    Assert(metaClass);
    Assert(!metaClass->Name().empty());

    WRITESCOPELOCK(_barrier);
    const auto it = _classes.find(metaClass->Name());

    AssertRelease(_classes.end() != it);
    Assert(it->second == metaClass);

    _classes.erase(it);
}
//----------------------------------------------------------------------------
const FMetaClass *FMetaClassHashMap::GetIFP(const FName& name) const {
    Assert(!name.empty());

    READSCOPELOCK(_barrier);

    const auto it = _classes.find(name);
    return (_classes.end() == it) ? nullptr : it->second;
}
//----------------------------------------------------------------------------
void FMetaClassHashMap::Clear() {
    WRITESCOPELOCK(_barrier);

    _classes.clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
