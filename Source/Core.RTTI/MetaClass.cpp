#include "stdafx.h"

#include "MetaClass.h"

#include "MetaFunction.h"
#include "MEtaNamespace.h"
#include "MetaProperty.h"

#include "Core/IO/String.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static TPair<FName, const FMetaFunction*> MetaFunctionPair_(const UCMetaFunction& f) {
    Assert(f);
    return MakePair(f->Name(), f.get());
}
template <typename _It>
static auto MakeMetaFunctionPair_(_It it) {
    return MakeOutputIterator(it, &MetaFunctionPair_);
}
//----------------------------------------------------------------------------
static TPair<FName, const FMetaProperty*> MetaPropertyPair_(const UCMetaProperty& p) {
    Assert(p);
    return MakePair(p->Name(), p.get());
}
template <typename _It>
static auto MakeMetaPropertyPair_(_It it) {
    return MakeOutputIterator(it, &MetaPropertyPair_);
}
//----------------------------------------------------------------------------
template <typename T>
static decltype(std::declval<const T>().get()) FindMemberIFP_(const FName& name, size_t attributes, const VECTOR(RTTI, T)& items) {
    for (const T& pItem : items) {
        if ((pItem->Name() == name) &&
            (pItem->Attributes() & attributes) == attributes)
            return pItem.get();
    }
    return nullptr;
}
//----------------------------------------------------------------------------
template <typename T>
static const T* FindMemberIFP_(const FName& name, size_t attributes, const HASHMAP(RTTI, FName, const T*)& items) {
    const auto it = items.find(name);
    return (it != items.end())
        ? ((it->second->Attributes() & attributes) == attributes ? it->second : nullptr)
        : nullptr;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// FMetaClassGuid contains a linear combination of prime numbers
//----------------------------------------------------------------------------
bool FMetaClassGuid::InheritsFrom(FMetaClassGuid parent) const {
    Assert(_value != 0);
    Assert(parent._value != 0);
    return ((_value / parent._value) * parent._value == _value);
}
//----------------------------------------------------------------------------
FMetaClassGuid FMetaClassGuid::Combine(FMetaClassGuid other) const {
    const FMetaClassGuid combined(_value * other._value);
    // Check for overflows :
    Assert(combined.InheritsFrom(_value));
    Assert(combined.InheritsFrom(other._value));
    return combined;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaClass::FMetaClass(
    FMetaClassGuid guid,
    EFlags attributes,
    const FName& name,
    const FMetaNamespace* metaNamespace )
:   _guid(guid)
,   _attributes(attributes)
,   _name(name)
,   _namespace(metaNamespace) {
    Assert(not _name.empty());
    Assert(nullptr != _namespace);
}
//----------------------------------------------------------------------------
FMetaClass::~FMetaClass() {}
//----------------------------------------------------------------------------
const FMetaFunction* FMetaClass::Function(const FName& name, size_t attributes /* = 0 */, bool inherited /* = true */) const {
    const FMetaFunction* func = FunctionIFP(name, attributes, inherited);
    if (nullptr == func && IsDynamic())
        func = OnMissingFunction(name, attributes);
    AssertRelease(nullptr != func);
    return func;
}
//----------------------------------------------------------------------------
const FMetaFunction* FMetaClass::Function(const FStringView& name, size_t attributes /* = 0 */, bool inherited /* = true */) const {
    Assert(not name.empty());
    // TODO: better method ?
    return Function(FName(name), attributes, inherited);
}
//----------------------------------------------------------------------------
const FMetaFunction *FMetaClass::FunctionIFP(const FName& name, size_t attributes /* = 0 */, bool inherited /* = true */) const {
    Assert(not name.empty());

    if (inherited)
        return FindMemberIFP_(name, attributes, _functionsInherited);
    else
        return FindMemberIFP_(name, attributes, _functions);
}
//----------------------------------------------------------------------------
const FMetaFunction *FMetaClass::FunctionIFP(const FStringView& name, size_t attributes /* = 0 */, bool inherited /* = true */) const {
    Assert(not name.empty());
    // TODO: better method ?
    return FunctionIFP(FName(name), attributes, inherited);
}
//----------------------------------------------------------------------------
const FMetaProperty* FMetaClass::Property(const FName& name, size_t attributes /* = 0 */, bool inherited /* = true */) const {
    const FMetaProperty* prop = PropertyIFP(name, attributes, inherited);
    if (nullptr == prop && IsDynamic())
        prop = OnMissingProperty(name, attributes);
    AssertRelease(nullptr != prop);
    return prop;
}
//----------------------------------------------------------------------------
const FMetaProperty* FMetaClass::Property(const FStringView& name, size_t attributes /* = 0 */, bool inherited /* = true */) const {
    Assert(not name.empty());
    // TODO: better method ?
    return Property(FName(name), attributes, inherited);
}
//----------------------------------------------------------------------------
const FMetaProperty *FMetaClass::PropertyIFP(const FName& name, size_t attributes /* = 0 */, bool inherited /* = true */) const {
    Assert(not name.empty());

    if (inherited)
        return FindMemberIFP_(name, attributes, _propertiesInherited);
    else
        return FindMemberIFP_(name, attributes, _properties);
}
//----------------------------------------------------------------------------
const FMetaProperty *FMetaClass::PropertyIFP(const FStringView& name, size_t attributes /* = 0 */, bool inherited /* = true */) const {
    Assert(not name.empty());
    // TODO: better method ?
    return PropertyIFP(FName(name), attributes, inherited);
}
//----------------------------------------------------------------------------
void FMetaClass::Initialize() {
    Assert(_functionsInherited.empty());
    Assert(_propertiesInherited.empty());

    if (const FMetaClass* parent = Parent()) {
        _guid = _guid.Combine(parent->_guid);

        _functionsInherited.reserve(parent->_functionsInherited.size() + _functions.size());
        _functionsInherited.append(parent->_functionsInherited);

        _propertiesInherited.reserve(parent->_propertiesInherited.size() + _properties.size());
        _propertiesInherited.append(parent->_propertiesInherited);
    }
    else {
        _functionsInherited.reserve(_functions.size());
        _propertiesInherited.reserve(_properties.size());
    }

    _functionsInherited.insert_AssertUnique(
        MakeMetaFunctionPair_(_functions.begin()),
        MakeMetaFunctionPair_(_functions.end()) );

    _propertiesInherited.insert_AssertUnique(
        MakeMetaPropertyPair_(_properties.begin()),
        MakeMetaPropertyPair_(_properties.end()) );
}
//----------------------------------------------------------------------------
void FMetaClass::RegisterFunction(UCMetaFunction&& func) {
    Assert(func);
    Assert(nullptr == FunctionIFP(func->Name(), 0, false));

    _functions.push_back(std::move(func));
}
//----------------------------------------------------------------------------
void FMetaClass::RegisterProperty(UCMetaProperty&& prop) {
    Assert(prop);
    Assert(nullptr == PropertyIFP(prop->Name(), 0, false));

    _properties.push_back(std::move(prop));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
