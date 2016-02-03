#include "stdafx.h"

#include "MetaClass.h"

#include "MetaClassDatabase.h"

#include "MetaProperty.h"

#include "Core/IO/String.h"
#include "Core/Memory/SegregatedMemoryPool.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
MetaClass::MetaClass(const MetaClassName& name, Flags attributes, const MetaClass *parent)
:   _name(name)
,   _parent(parent)
,   _attributes(attributes) {}
//----------------------------------------------------------------------------
MetaClass::~MetaClass() {
    for (const auto& it : _properties)
        checked_delete(it.second);
}
//----------------------------------------------------------------------------
const MetaProperty *MetaClass::PropertyIFP(const char *name, size_t attributes /* = 0 */, bool inherited /* = true */) const {
    Assert(name);

    for (const auto& p : _properties)
        if ((p.second->Attributes() & attributes) == attributes &&
            (0 == CompareN(p.first.c_str(), name, p.first.size())) )
            return p.second;

    return (inherited && _parent)
        ? _parent->PropertyIFP(name, attributes, true)
        : nullptr;
}
//----------------------------------------------------------------------------
const MetaProperty *MetaClass::PropertyIFP(const MetaPropertyName& name, size_t attributes /* = 0 */, bool inherited /* = true */) const {
    Assert(name.size());

    const auto it = _properties.Find(name);
    if ((it != _properties.end()) &&
        (it->second->Attributes() & attributes) == attributes)
        return it->second;

    return (inherited && _parent)
        ? _parent->PropertyIFP(name, attributes, true)
        : nullptr;
}
//----------------------------------------------------------------------------
bool MetaClass::InheritsFrom(const MetaClass *parent) const {
    Assert(parent);

    if (parent == this)
        return true;
    else if (nullptr == _parent)
        return false;
    else
        return _parent->InheritsFrom(parent);
}
//----------------------------------------------------------------------------
bool MetaClass::IsAssignableFrom(const MetaClass *child) const {
    Assert(child);
    return child->InheritsFrom(this);
}
//----------------------------------------------------------------------------
void MetaClass::Register(MetaClassHashMap& database) const {
    database.Add(this);
}
//----------------------------------------------------------------------------
void MetaClass::Unregister(MetaClassHashMap& database) const {
    database.Remove(this);
}
//----------------------------------------------------------------------------
MetaObject* MetaClass::CreateInstance() const {
    Assert(!IsAbstract());
    return VirtualCreateInstance();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
