﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "MetaClass.h"

#include "MetaModule.h"

#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"
#include "IO/TextWriter.h"

namespace PPE {
namespace RTTI {
EXTERN_LOG_CATEGORY(PPE_RTTI_API, RTTI)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMetaClass::FMetaClass(FClassId id, const FName& name, EClassFlags flags, const FMetaModule* module) NOEXCEPT
:   _id(id)
,   _flags(flags)
,   _name(name)
,   _module(module)
#if USE_PPE_MEMORYDOMAINS
,   _trackingData(name.data(), _module
        ? &_module->TrackingData()
        : &MEMORYDOMAIN_TRACKING_DATA(MetaObject) )
{
    RegisterTrackingData(&_trackingData);
#else
{
#endif
    Assert_NoAssume(_id.Value());
    Assert_NoAssume(not _name.empty());
    Assert_NoAssume(_flags != EClassFlags(0));
    Assert(_module);
}
//----------------------------------------------------------------------------
FMetaClass::~FMetaClass() {
    Assert_NoAssume(not IsRegistered());
    Assert_NoAssume(_propertiesAll.empty());
#if USE_PPE_MEMORYDOMAINS
    Assert_NoAssume(_trackingData.empty()); // must be empty upon destruction
    UnregisterTrackingData(&_trackingData);
#endif
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
FMetaFunction& FMetaClass::RegisterFunction(FMetaFunction&& function) {
    Assert(not IsRegistered());
    Assert(not function.Name().empty());
#if USE_PPE_ASSERT
    // check that no property with the same name was already registered
    for (const FMetaFunction& f : _functionsSelf)
        Assert(function.Name() != f.Name());
#endif

    _functionsSelf.emplace_back(std::move(function));
    return _functionsSelf.back();
}
//----------------------------------------------------------------------------
bool FMetaClass::HasFunction(const FMetaFunction& func, bool inherited) const NOEXCEPT {
    return (inherited
        ? _functionsAll.Values().Contains(std::addressof(func))
        : _functionsSelf.MakeView().ContainsRef(func) );
}
//----------------------------------------------------------------------------
const FMetaFunction& FMetaClass::Function(const FName& name, EFunctionFlags flags/* = EFunctionFlags::All */, bool inherited/* = true */) const {
    const FMetaFunction* const pfunction = FunctionIFP(name, flags, inherited);
    return (*(pfunction ? pfunction : OnMissingFunction(name, flags)));
}
//----------------------------------------------------------------------------
const FMetaFunction* FMetaClass::FunctionIFP(const FName& name, EFunctionFlags flags/* = EFunctionFlags::All */, bool inherited/* = true */) const {
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
        pfunction = std::addressof(*it);
    }
    Assert(pfunction);

    return (u32(pfunction->Flags()) & u32(flags) ? pfunction : nullptr);
}
//----------------------------------------------------------------------------
const FMetaFunction* FMetaClass::FunctionIFP(const FStringView& name, EFunctionFlags flags/* = EFunctionFlags::All */, bool inherited/* = true */) const {
    return FunctionIFP(FLazyName{ name }, flags, inherited);
}
//----------------------------------------------------------------------------
const FMetaFunction* FMetaClass::FunctionIFP(const FLazyName& name, EFunctionFlags flags/* = EFunctionFlags::All */, bool inherited/* = true */) const {
    Assert(IsRegistered());
    Assert(not name.empty());

    const FMetaFunction* pfunction;
    if (inherited) {
        const auto it = _functionsAll.find_like(name, name.HashValue());
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
        pfunction = std::addressof(*it);
    }
    Assert(pfunction);

    return (u32(pfunction->Flags()) & u32(flags) ? pfunction : nullptr);
}
//----------------------------------------------------------------------------
const FMetaFunction* FMetaClass::OnMissingFunction(const FName& name, EFunctionFlags flags/* = EFunctionFlags::All */) const {
    Assert(IsRegistered());
    Assert(not name.empty());
    Unused(name);
    Unused(flags);

    PPE_LOG(RTTI, Error, "missing function {0}::{1} !", _name, name);

    AssertNotReached();
}
//----------------------------------------------------------------------------
// Properties
//----------------------------------------------------------------------------
FMetaProperty& FMetaClass::RegisterProperty(FMetaProperty&& property) {
    Assert(not IsRegistered());
    Assert(not property.Name().empty());

#if USE_PPE_ASSERT
    // check that no property with the same name was already registered
    for (const FMetaProperty& p : _propertiesSelf)
        Assert(property.Name() != p.Name());
#endif

    _propertiesSelf.emplace_back(std::move(property));
    return _propertiesSelf.back();
}
//----------------------------------------------------------------------------
bool FMetaClass::HasProperty(const FMetaProperty& property, bool inherited) const NOEXCEPT {
    return (inherited
         ? _propertiesAll.Values().Contains(std::addressof(property))
         : _propertiesSelf.MakeView().ContainsRef(property) );
}
//----------------------------------------------------------------------------
const FMetaProperty& FMetaClass::Property(const FName& name, EPropertyFlags flags/* = EPropertyFlags::All */, bool inherited/* = true */) const {
    const FMetaProperty* const pproperty = PropertyIFP(name, flags, inherited);
    return (*(pproperty ? pproperty : OnMissingProperty(name, flags)));
}
//----------------------------------------------------------------------------
const FMetaProperty* FMetaClass::PropertyIFP(const FName& name, EPropertyFlags flags/* = EPropertyFlags::All */, bool inherited/* = true */) const {
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
        pproperty = std::addressof(*it);
    }
    Assert(pproperty);

    return (u32(pproperty->Flags()) & u32(flags) ? pproperty : nullptr);
}
//----------------------------------------------------------------------------
const FMetaProperty* FMetaClass::PropertyIFP(const FStringView& name, EPropertyFlags flags/* = EPropertyFlags::All */, bool inherited/* = true */) const {
    return PropertyIFP(FLazyName{ name }, flags, inherited);
}
//----------------------------------------------------------------------------
const FMetaProperty* FMetaClass::PropertyIFP(const FLazyName& name, EPropertyFlags flags/* = EPropertyFlags::All */, bool inherited/* = true */) const {
    Assert(IsRegistered());
    Assert(not name.empty());

    const FMetaProperty* pproperty = nullptr;
    if (inherited) {
        const auto it = _propertiesAll.find_like(name, name.HashValue());
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
        pproperty = std::addressof(*it);
    }
    Assert(pproperty);

    return (u32(pproperty->Flags()) & u32(flags) ? pproperty : nullptr);
}
//----------------------------------------------------------------------------
const FMetaProperty* FMetaClass::OnMissingProperty(const FName& name, EPropertyFlags flags/* = EPropertyFlags::All */) const {
    Assert(IsRegistered());
    Assert(not name.empty());
    Unused(name);
    Unused(flags);

    PPE_LOG(RTTI, Error, "missing property {0}::{1} !", _name, name);

    AssertNotReached();
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

    _functionsSelf.shrink_to_fit();
    _propertiesSelf.shrink_to_fit();

    if (const FMetaClass* parent = Parent()) {
        if (parent->_module == _module)
            const_cast<FMetaClass*>(parent)->CallOnRegister_IFN();
        else
            Assert(parent->IsRegistered());

        _id = FClassId::Combine(_id, parent->_id);

        _propertiesAll.reserve(_propertiesSelf.size() + parent->_propertiesAll.size());
        _propertiesAll.append(parent->_propertiesAll);

        _functionsAll.reserve(_functionsSelf.size() + parent->_functionsAll.size());
        _functionsAll.append(parent->_functionsAll);

        Add_AssertUnique(parent->_children, this);
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

    if (const FMetaClass* parent = Parent())
        Remove_AssertExists(parent->_children, this);

    Assert(not IsRegistered());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
static TBasicTextWriter<_Char>& Print_(TBasicTextWriter<_Char>& oss, RTTI::EClassFlags flags) {
    auto sep = Fmt::NotFirstTime(STRING_LITERAL(_Char, '|'));

    if (flags & RTTI::EClassFlags::Concrete)    { oss << sep << STRING_LITERAL(_Char, "Concrete"); }
    if (flags & RTTI::EClassFlags::Abstract)    { oss << sep << STRING_LITERAL(_Char, "Abstract"); }
    if (flags & RTTI::EClassFlags::Dynamic)     { oss << sep << STRING_LITERAL(_Char, "Dynamic"); }
    if (flags & RTTI::EClassFlags::Public)      { oss << sep << STRING_LITERAL(_Char, "Public"); } else {
                                                  oss << sep << STRING_LITERAL(_Char, "Private"); }
    if (flags & RTTI::EClassFlags::Mergeable)   { oss << sep << STRING_LITERAL(_Char, "Mergeable"); }
    if (flags & RTTI::EClassFlags::Deprecated)  { oss << sep << STRING_LITERAL(_Char, "Deprecated"); }
    if (flags & RTTI::EClassFlags::Registered)  { oss << sep << STRING_LITERAL(_Char, "Registered"); }

    return oss;
}
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, RTTI::EClassFlags flags) {
    return Print_(oss, flags);
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, RTTI::EClassFlags flags) {
    return Print_(oss, flags);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
