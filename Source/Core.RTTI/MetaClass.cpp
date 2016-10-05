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
FMetaClass::FMetaClass(const FName& name, EFlags attributes)
:   _name(name)
,   _attributes(attributes) {}
//----------------------------------------------------------------------------
FMetaClass::~FMetaClass() {}
//----------------------------------------------------------------------------
bool FMetaClass::InheritsFrom(const FMetaClass *parent) const {
    Assert(parent);

    if (parent == this)
        return true;

    const FMetaClass* _parent = Parent();
    return (nullptr != _parent)
        ? _parent->InheritsFrom(parent)
        : false;
}
//----------------------------------------------------------------------------
bool FMetaClass::IsAssignableFrom(const FMetaClass *child) const {
    Assert(child);
    return child->InheritsFrom(this);
}
//----------------------------------------------------------------------------
void FMetaClass::Register(FMetaClassHashMap& database) const {
    database.Add(this);
}
//----------------------------------------------------------------------------
void FMetaClass::Unregister(FMetaClassHashMap& database) const {
    database.Remove(this);
}
//----------------------------------------------------------------------------
const FMetaClass* FMetaClass::Parent() const {
    return VirtualParent();
}
//----------------------------------------------------------------------------
TMemoryView<const UCMetaProperty> FMetaClass::Properties() const {
    return VirtualProperties();
}
//----------------------------------------------------------------------------
const FMetaProperty *FMetaClass::PropertyIFP(const FStringView& name, size_t attributes /* = 0 */, bool inherited /* = true */) const {
    Assert(not name.empty());

    const FMetaProperty* result = VirtualPropertyIFP(name, attributes);
    if (result)
        return result;

    return (inherited && Parent())
        ? Parent()->PropertyIFP(name, attributes, true)
        : nullptr;
}
//----------------------------------------------------------------------------
const FMetaProperty *FMetaClass::PropertyIFP(const FName& name, size_t attributes /* = 0 */, bool inherited /* = true */) const {
    Assert(name.size());

    const FMetaProperty* result = VirtualPropertyIFP(name, attributes);
    if (result)
        return result;

    return (inherited && Parent())
        ? Parent()->PropertyIFP(name, attributes, true)
        : nullptr;
}
//----------------------------------------------------------------------------
FMetaObject* FMetaClass::CreateInstance() const {
    Assert(not IsAbstract());
    return VirtualCreateInstance();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
InScopeMetaClass::InScopeMetaClass(const FName& name, EFlags attributes)
:   FMetaClass(name, attributes) {}
//----------------------------------------------------------------------------
InScopeMetaClass::~InScopeMetaClass() {}
//----------------------------------------------------------------------------
TMemoryView<const UCMetaProperty> InScopeMetaClass::VirtualProperties() const {
    return MakeView(_properties);
}
//----------------------------------------------------------------------------
const FMetaProperty *InScopeMetaClass::VirtualPropertyIFP(const FStringView& name, size_t attributes) const {
    for (const UCMetaProperty& p : _properties)
        if ((p->Attributes() & attributes) == attributes &&
            (0 == Compare(p->Name().MakeView(), name)) )
            return p.get();

    return nullptr;
}
//----------------------------------------------------------------------------
const FMetaProperty *InScopeMetaClass::VirtualPropertyIFP(const FName& name, size_t attributes) const {
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
        const FName name = prop->Name();
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
