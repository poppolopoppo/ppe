#include "stdafx.h"

#include "MetaClass.h"

#include "MetaClassDatabase.h"

#include "RTTI/Property/MetaProperty.h"

#include "IO/String.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
MetaClass::MetaClass(const MetaClassName& name, Flags attributes, const MetaClass *parent)
:   _name(name) {
    _parentWFlags.Reset(parent, static_cast<size_t>(attributes));
}
//----------------------------------------------------------------------------
MetaClass::~MetaClass() {
    for (const auto& it : _properties)
        delete(it.second);
}
//----------------------------------------------------------------------------
const MetaProperty *MetaClass::PropertyIFP(const char *name, size_t attributes /* = 0 */, bool inherited /* = true */) const {
    Assert(name);

    for (const auto& p : _properties)
        if ((p.second->Attributes() & attributes) == attributes &&
            (0 == CompareN(p.first.cstr(), name, p.first.size())) )
            return p.second;

    return (inherited && _parentWFlags.Get())
        ? _parentWFlags->PropertyIFP(name, attributes, true)
        : nullptr;
}
//----------------------------------------------------------------------------
const MetaProperty *MetaClass::PropertyIFP(const MetaPropertyName& name, size_t attributes /* = 0 */, bool inherited /* = true */) const {
    Assert(name.size());

    const auto it = _properties.Find(name);
    if ((it != _properties.end()) ||
        (it->second->Attributes() & attributes) == attributes)
        return it->second;

    return (inherited && _parentWFlags.Get())
        ? _parentWFlags->PropertyIFP(name, attributes, true)
        : nullptr;
}
//----------------------------------------------------------------------------
bool MetaClass::InheritsFrom(const MetaClass *parent) const {
    Assert(parent);

    if (parent == this)
        return true;
    else if (nullptr == _parentWFlags.Get())
        return false;
    else
        return _parentWFlags.Get()->InheritsFrom(parent);
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
