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
MetaClass::~MetaClass() {}
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
MemoryView<const UCMetaProperty> MetaClass::Properties() const {
    return VirtualProperties();
}
//----------------------------------------------------------------------------
const MetaProperty *MetaClass::PropertyIFP(const StringSlice& name, size_t attributes /* = 0 */, bool inherited /* = true */) const {
    Assert(not name.empty());

    const MetaProperty* result = VirtualPropertyIFP(name, attributes);
    if (result)
        return result;

    return (inherited && Parent())
        ? Parent()->PropertyIFP(name, attributes, true)
        : nullptr;
}
//----------------------------------------------------------------------------
const MetaProperty *MetaClass::PropertyIFP(const MetaPropertyName& name, size_t attributes /* = 0 */, bool inherited /* = true */) const {
    Assert(name.size());

    const MetaProperty* result = VirtualPropertyIFP(name, attributes);
    if (result)
        return result;

    return (inherited && Parent())
        ? Parent()->PropertyIFP(name, attributes, true)
        : nullptr;
}
//----------------------------------------------------------------------------
MetaObject* MetaClass::CreateInstance() const {
    Assert(not IsAbstract());
    return VirtualCreateInstance();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
InScopeMetaClass::InScopeMetaClass(const MetaClassName& name, Flags attributes, const MetaClass *parent)
:   MetaClass(name, attributes, parent) {}
//----------------------------------------------------------------------------
InScopeMetaClass::~InScopeMetaClass() {}
//----------------------------------------------------------------------------
MemoryView<const UCMetaProperty> InScopeMetaClass::VirtualProperties() const {
    return MakeView(_properties);
}
//----------------------------------------------------------------------------
const MetaProperty *InScopeMetaClass::VirtualPropertyIFP(const StringSlice& name, size_t attributes) const {
    for (const UCMetaProperty& p : _properties)
        if ((p->Attributes() & attributes) == attributes &&
            (0 == Compare(p->Name().MakeView(), name)) )
            return p.get();

    return nullptr;
}
//----------------------------------------------------------------------------
const MetaProperty *InScopeMetaClass::VirtualPropertyIFP(const MetaPropertyName& name, size_t attributes) const {
    for (const UCMetaProperty& p : _properties)
        if ((p->Attributes() & attributes) == attributes &&
            (p->Name() == name) )
            return p.get();

    return nullptr;
}
//----------------------------------------------------------------------------
void InScopeMetaClass::RegisterProperty(UCMetaProperty&& prop) {
    Assert(prop);
    Assert(not prop->Name().empty());

#ifdef WITH_CORE_ASSERT
    {
        const MetaPropertyName name = prop->Name();
        for (const UCMetaProperty& p : _properties)
            Assert(p->Name() != name);
    }
#endif

    _properties.emplace_back(std::move(prop));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
