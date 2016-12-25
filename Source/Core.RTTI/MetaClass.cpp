#include "stdafx.h"

#include "MetaClass.h"

#include "MetaClassDatabase.h"
#include "MetaFunction.h"
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
bool FMetaClass::CastTo(const FMetaClass *other) const {
    Assert(other);
    return (IsAssignableFrom(other) || other->IsAssignableFrom(this));
}
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
TMemoryView<const UCMetaFunction> FMetaClass::Functions() const {
    return VirtualFunctions();
}
//----------------------------------------------------------------------------
TMemoryView<const UCMetaProperty> FMetaClass::Properties() const {
    return VirtualProperties();
}
//----------------------------------------------------------------------------
const FMetaFunction *FMetaClass::FunctionIFP(const FStringView& name, size_t attributes /* = 0 */, bool inherited /* = true */) const {
    Assert(not name.empty());

    const FMetaFunction* result = VirtualFunctionIFP(name, attributes);
    if (result)
        return result;

    return (inherited && Parent())
        ? Parent()->FunctionIFP(name, attributes, true)
        : nullptr;
}
//----------------------------------------------------------------------------
const FMetaFunction *FMetaClass::FunctionIFP(const FName& name, size_t attributes /* = 0 */, bool inherited /* = true */) const {
    Assert(not name.empty());

    const FMetaFunction* result = VirtualFunctionIFP(name, attributes);
    if (result)
        return result;

    return (inherited && Parent())
        ? Parent()->FunctionIFP(name, attributes, true)
        : nullptr;
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
FInScopeMetaClass::FInScopeMetaClass(const FName& name, EFlags attributes)
:   FMetaClass(name, attributes) {}
//----------------------------------------------------------------------------
FInScopeMetaClass::~FInScopeMetaClass() {}
//----------------------------------------------------------------------------
TMemoryView<const UCMetaFunction> FInScopeMetaClass::VirtualFunctions() const {
    return MakeView(_functions);
}
//----------------------------------------------------------------------------
TMemoryView<const UCMetaProperty> FInScopeMetaClass::VirtualProperties() const {
    return MakeView(_properties);
}
//----------------------------------------------------------------------------
const FMetaFunction *FInScopeMetaClass::VirtualFunctionIFP(const FStringView& name, size_t attributes) const {
    for (const UCMetaFunction& f : _functions)
        if ((f->Attributes() & attributes) == attributes &&
            (0 == Compare(f->Name().MakeView(), name)))
            return f.get();

    return nullptr;
}
//----------------------------------------------------------------------------
const FMetaFunction *FInScopeMetaClass::VirtualFunctionIFP(const FName& name, size_t attributes) const {
    for (const UCMetaFunction& f : _functions)
        if ((f->Attributes() & attributes) == attributes &&
            (f->Name() == name))
            return f.get();

    return nullptr;
}
//----------------------------------------------------------------------------
const FMetaProperty *FInScopeMetaClass::VirtualPropertyIFP(const FStringView& name, size_t attributes) const {
    for (const UCMetaProperty& p : _properties)
        if ((p->Attributes() & attributes) == attributes &&
            (0 == Compare(p->Name().MakeView(), name)) )
            return p.get();

    return nullptr;
}
//----------------------------------------------------------------------------
const FMetaProperty *FInScopeMetaClass::VirtualPropertyIFP(const FName& name, size_t attributes) const {
    for (const UCMetaProperty& p : _properties)
        if ((p->Attributes() & attributes) == attributes &&
            (p->Name() == name) )
            return p.get();

    return nullptr;
}
//----------------------------------------------------------------------------
void FInScopeMetaClass::RegisterFunction(UCMetaFunction&& func) {
    Assert(func);
    Assert(not func->Name().empty());

#ifdef WITH_CORE_ASSERT
    {
        const FName name = func->Name();
        for (const UCMetaFunction& f : _functions)
            Assert(f->Name() != name);
    }
#endif

    _functions.emplace_back(std::move(func));
}
//----------------------------------------------------------------------------
void FInScopeMetaClass::RegisterProperty(UCMetaProperty&& prop) {
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
