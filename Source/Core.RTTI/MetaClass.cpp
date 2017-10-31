#include "stdafx.h"

#include "MetaClass.h"

#include "MetaNamespace.h"

#include "Core/Diagnostic/Logger.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaClass::FMetaClass(FClassId id, const FName& name, EClassFlags flags, const FMetaNamespace* metaNamespace)
    : _id(id)
    , _flags(flags)
    , _name(name)
    , _namespace(metaNamespace) {
    Assert(_id.Value());
    Assert(not _name.empty());
    Assert(_flags != EClassFlags(0));
    Assert(_namespace);
}
//----------------------------------------------------------------------------
FMetaClass::~FMetaClass() {
    Assert(not IsRegistered());
    Assert(_propertiesAll.empty());
}
//----------------------------------------------------------------------------
// Cast / Inheritance
//----------------------------------------------------------------------------
bool FMetaClass::CastTo(const FMetaClass& other) const {
    return (InheritsFrom(other) || IsAssignableFrom(other));
}
//----------------------------------------------------------------------------
bool FMetaClass::InheritsFrom(const FMetaClass& parent) const {
    Assert(IsRegistered());

    return _id.Contains(parent._id);
}
//----------------------------------------------------------------------------
bool FMetaClass::IsAssignableFrom(const FMetaClass& child) const {
    Assert(IsRegistered());

    return child._id.Contains(_id);
}
//----------------------------------------------------------------------------
// Functions
//----------------------------------------------------------------------------
void FMetaClass::RegisterFunction(FMetaFunction&& function) {
    Assert(not IsRegistered());
    Assert(not function.Name().empty());
#ifdef WITH_CORE_ASSERT
    // check that no property with the same name was already registered
    for (const FMetaFunction& f : _functionsSelf)
        Assert(function.Name() != f.Name());
#endif

    _functionsSelf.emplace_back(std::move(function));
}
//----------------------------------------------------------------------------
const FMetaFunction& FMetaClass::Function(const FName& name, EFunctionFlags flags/* = EFunctionFlags(0) */, bool inherited/* = true */) const {
    const FMetaFunction* pfunction = FunctionIFP(name, flags, inherited);
    return (*(pfunction ? pfunction : OnMissingFunction(name, flags)));
}
//----------------------------------------------------------------------------
const FMetaFunction* FMetaClass::FunctionIFP(const FName& name, EFunctionFlags flags/* = EFunctionFlags(0) */, bool inherited/* = true */) const {
    Assert(IsRegistered());
    Assert(not name.empty());

    const FMetaFunction* pfunction;
    if (inherited) {
        const auto it = _functionsAll.find(name);
        if (_functionsAll.end() == it)
            return nullptr;
        pfunction = it->second;
    }
    else {
        const auto it = std::find_if(_functionsSelf.begin(), _functionsSelf.end(), [&name](const FMetaFunction& f) {
            return (f.Name() == name);
        });
        if (_functionsSelf.end() == it)
            return nullptr;
        pfunction = &*it;
    }
    Assert(pfunction);

    return (pfunction->Flags() & flags ? pfunction : nullptr);
}
//----------------------------------------------------------------------------
const FMetaFunction* FMetaClass::FunctionIFP(const FStringView& name, EFunctionFlags flags/* = EFunctionFlags(0) */, bool inherited/* = true */) const {
    Assert(IsRegistered());
    Assert(not name.empty());

    const FMetaFunction* pfunction;
    if (inherited) {
        const hash_t h = FName::HashValue(name);
        const auto it = _functionsAll.find_like(name, h);
        if (_functionsAll.end() == it)
            return nullptr;
        pfunction = it->second;
    }
    else {
        const auto it = std::find_if(_functionsSelf.begin(), _functionsSelf.end(), [&name](const FMetaFunction& f) {
            return (f.Name() == name);
        });
        if (_functionsSelf.end() == it)
            return nullptr;
        pfunction = &*it;
    }
    Assert(pfunction);

    return (pfunction->Flags() & flags ? pfunction : nullptr);
}
//----------------------------------------------------------------------------
const FMetaFunction* FMetaClass::OnMissingFunction(const FName& name, EFunctionFlags flags/* = EFunctionFlags(0) */) const {
    Assert(IsRegistered());
    Assert(not name.empty());
    UNUSED(name);
    UNUSED(flags);

    LOG(Error, L"[RTTI] Missing function {0}::{1} !", _name, name);

    AssertNotReached();
    return nullptr;
}
//----------------------------------------------------------------------------
// Properties
//----------------------------------------------------------------------------
void FMetaClass::RegisterProperty(FMetaProperty&& property) {
    Assert(not IsRegistered());
    Assert(not property.Name().empty());
#ifdef WITH_CORE_ASSERT
    // check that no property with the same name was already registered
    for (const FMetaProperty& p : _propertiesSelf)
        Assert(property.Name() != p.Name());
#endif

    _propertiesSelf.emplace_back(std::move(property));
}
//----------------------------------------------------------------------------
const FMetaProperty& FMetaClass::Property(const FName& name, EPropertyFlags flags/* = EPropertyFlags(0) */, bool inherited/* = true */) const {
    const FMetaProperty* pproperty = PropertyIFP(name, flags, inherited);
    return (*(pproperty ? pproperty : OnMissingProperty(name, flags)));
}
//----------------------------------------------------------------------------
const FMetaProperty* FMetaClass::PropertyIFP(const FName& name, EPropertyFlags flags/* = EPropertyFlags(0) */, bool inherited/* = true */) const {
    Assert(IsRegistered());
    Assert(not name.empty());

    const FMetaProperty* pproperty = nullptr;
    if (inherited) {
        const auto it = _propertiesAll.find(name);
        if (_propertiesAll.end() == it)
            return nullptr;
        pproperty = it->second;
    }
    else {
        const auto it = std::find_if(_propertiesSelf.begin(), _propertiesSelf.end(), [&name](const FMetaProperty& p) {
            return (p.Name() == name);
        });
        if (_propertiesSelf.end() == it)
            return nullptr;
        pproperty = &*it;
    }
    Assert(pproperty);

    return (pproperty->Flags() & flags ? pproperty : nullptr);
}
//----------------------------------------------------------------------------
const FMetaProperty* FMetaClass::PropertyIFP(const FStringView& name, EPropertyFlags flags/* = EPropertyFlags(0) */, bool inherited/* = true */) const {
    Assert(IsRegistered());
    Assert(not name.empty());

    const FMetaProperty* pproperty = nullptr;
    if (inherited) {
        const hash_t h = FName::HashValue(name);
        const auto it = _propertiesAll.find_like(name, h);
        if (_propertiesAll.end() == it)
            return nullptr;
        pproperty = it->second;
    }
    else {
        const auto it = std::find_if(_propertiesSelf.begin(), _propertiesSelf.end(), [&name](const FMetaProperty& p) {
            return (p.Name() == name);
        });
        if (_propertiesSelf.end() == it)
            return nullptr;
        pproperty = &*it;
    }
    Assert(pproperty);

    return (pproperty->Flags() & flags ? pproperty : nullptr);
}
//----------------------------------------------------------------------------
const FMetaProperty* FMetaClass::OnMissingProperty(const FName& name, EPropertyFlags flags/* = EPropertyFlags(0) */) const {
    Assert(IsRegistered());
    Assert(not name.empty());
    UNUSED(name);
    UNUSED(flags);

    LOG(Error, L"[RTTI] Missing property {0}::{1} !", _name, name);

    AssertNotReached();
    return nullptr;
}
//----------------------------------------------------------------------------
// Called by meta namespace
//----------------------------------------------------------------------------
void FMetaClass::CallOnRegister_IFN() {
    if (not (_flags & EClassFlags::Registered))
        OnRegister();
}
//----------------------------------------------------------------------------
void FMetaClass::OnRegister() {
    Assert(not IsRegistered());
    Assert(_propertiesAll.empty());

    _flags = _flags + EClassFlags::Registered;
    _propertiesSelf.shrink_to_fit();

    const FMetaClass* parent = Parent();
    if (parent) {
        if (parent->_namespace == _namespace)
            const_cast<FMetaClass*>(parent)->CallOnRegister_IFN();
        else
            Assert(parent->IsRegistered());

        _id = FClassId::Combine(_id, parent->_id);

        _propertiesAll.reserve(_propertiesSelf.size() + parent->_propertiesAll.size());
        _propertiesAll.append(parent->_propertiesAll);

        _functionsAll.reserve(_functionsSelf.size() + parent->_functionsAll.size());
        _functionsAll.append(parent->_functionsAll);
    }
    else {
        _propertiesAll.reserve(_propertiesSelf.size());
        _functionsAll.reserve(_functionsSelf.size());
    }

    for (const auto& p : _propertiesSelf)
        Insert_AssertUnique(_propertiesAll, p.Name(), &p);

    for (const auto& f : _functionsSelf)
        Insert_AssertUnique(_functionsAll, f.Name(), &f);

    Assert(IsRegistered());
}
//----------------------------------------------------------------------------
void FMetaClass::OnUnregister() {
    Assert(IsRegistered());

    _flags = _flags - EClassFlags::Registered;

    _propertiesAll.clear_ReleaseMemory();
    _functionsAll.clear_ReleaseMemory();

    Assert(not IsRegistered());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, RTTI::EClassFlags flags) {
    bool s = false;

    if (flags & RTTI::EClassFlags::Concrete)    { if (s) oss << '|'; else s = true; oss << "Concrete"; }
    if (flags & RTTI::EClassFlags::Abstract)    { if (s) oss << '|'; else s = true; oss << "Abstract"; }
    if (flags & RTTI::EClassFlags::Dynamic)     { if (s) oss << '|'; else s = true; oss << "Dynamic"; }
    if (flags & RTTI::EClassFlags::Public)      { if (s) oss << '|'; else s = true; oss << "Public"; } else {
                                                  if (s) oss << '|'; else s = true; oss << "Private"; }
    if (flags & RTTI::EClassFlags::Mergeable)   { if (s) oss << '|'; else s = true; oss << "Mergeable"; }
    if (flags & RTTI::EClassFlags::Deprecated)  { if (s) oss << '|'; else s = true; oss << "Deprecated"; }
    if (flags & RTTI::EClassFlags::Registered)  { if (s) oss << '|'; else s = true; oss << "Registered"; }

    return oss;
}
//----------------------------------------------------------------------------
std::basic_ostream<wchar_t>& operator <<(std::basic_ostream<wchar_t>& oss, RTTI::EClassFlags flags) {
    bool s = false;

    if (flags & RTTI::EClassFlags::Concrete)    { if (s) oss << L'|'; else s = true; oss << L"Concrete"; }
    if (flags & RTTI::EClassFlags::Abstract)    { if (s) oss << L'|'; else s = true; oss << L"Abstract"; }
    if (flags & RTTI::EClassFlags::Dynamic)     { if (s) oss << L'|'; else s = true; oss << L"Dynamic"; }
    if (flags & RTTI::EClassFlags::Public)      { if (s) oss << L'|'; else s = true; oss << L"Public"; } else {
                                                  if (s) oss << L'|'; else s = true; oss << L"Private"; }
    if (flags & RTTI::EClassFlags::Mergeable)   { if (s) oss << L'|'; else s = true; oss << L"Mergeable"; }
    if (flags & RTTI::EClassFlags::Deprecated)  { if (s) oss << L'|'; else s = true; oss << L"Deprecated"; }
    if (flags & RTTI::EClassFlags::Registered)  { if (s) oss << L'|'; else s = true; oss << L"Registered"; }

    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
