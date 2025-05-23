#pragma once

#include "RTTI_fwd.h"

#include "MetaFunction.h"
#include "MetaProperty.h"
#include "RTTI/Typedefs.h"
#include "RTTI/TypeTraits.h"
#include "RTTI/UserFacet.h"

#include "Container/HashMap.h"
#include "Container/Vector.h"
#include "IO/TextWriter_fwd.h"
#include "Memory/MemoryDomain.h"
#include "Meta/Optional.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EClassFlags : u32 {
    // /!\ Report changes to MetaEnumHelpers.cpp

    None        = 0,

    Concrete    = 1<<0,
    Abstract    = 1<<1,
    Dynamic     = 1<<2,

    Private     = 0,
    Public      = 1<<3,

    Mergeable   = 1<<4,
    Deprecated  = 1<<5,

    Registered  = 1<<6,

    All         = UINT32_MAX
};
ENUM_FLAGS(EClassFlags);
//----------------------------------------------------------------------------
template <typename T> const FMetaClass* MetaClass();
//----------------------------------------------------------------------------
class PPE_RTTI_API FMetaClass {
public:
    FMetaClass(FClassId id, const FName& name, EClassFlags flags, const FMetaModule* module) NOEXCEPT;
    virtual ~FMetaClass();

    FMetaClass(const FMetaClass& ) = delete;
    FMetaClass& operator =(const FMetaClass& ) = delete;

    FMetaClass(FMetaClass&& ) = delete;
    FMetaClass& operator =(FMetaClass&& ) = delete;

    FClassId Id() const { return _id; }
    const FName& Name() const { return _name; }
    EClassFlags Flags() const { return _flags; }
    const FMetaModule* Module() const { return _module; }

    FMetaClassFacet& Facets() { return _facets; }
    const FMetaClassFacet& Facets() const { return _facets; }

    // Status

    bool IsAbstract()   const { return (_flags ^ EClassFlags::Abstract); }
    bool IsConcrete()   const { return (_flags ^ EClassFlags::Concrete); }
    bool IsDynamic()    const { return (_flags ^ EClassFlags::Dynamic); }
    bool IsMergeable()  const { return (_flags ^ EClassFlags::Mergeable); }
    bool IsDeprecated() const { return (_flags ^ EClassFlags::Deprecated); }
    bool IsRegistered() const { return (_flags ^ EClassFlags::Registered); }

    // Cast / Inheritance

    bool CastTo(const FMetaClass& other) const;
    bool InheritsFrom(const FMetaClass& parent) const;
    bool IsAssignableFrom(const FMetaClass& child) const;

    auto Children() const { return _children.MakeView(); }

    // Functions

    size_t NumFunctions(bool inherited = true) const {
        return (inherited ? _functionsAll.size() : _functionsSelf.size());
    }

    FMetaFunction& RegisterFunction(FMetaFunction&& function);

    TIterable<TValueIterator<HASHMAP(MetaClass, FName, const FMetaFunction*)::const_iterator>> AllFunctions() const { return _functionsAll.Values(); }

    TMemoryView<const FMetaFunction> SelfFunctions() const { return _functionsSelf.MakeConstView(); }

    bool HasFunction(const FMetaFunction& func, bool inherited = true) const NOEXCEPT;

    const FMetaFunction& Function(const FName& name, EFunctionFlags flags = EFunctionFlags::All, bool inherited = true) const;
    const FMetaFunction* FunctionIFP(const FName& name, EFunctionFlags flags = EFunctionFlags::All, bool inherited = true) const;
    const FMetaFunction* FunctionIFP(const FStringView& name, EFunctionFlags flags = EFunctionFlags::All, bool inherited = true) const;
    const FMetaFunction* FunctionIFP(const FLazyName& name, EFunctionFlags flags = EFunctionFlags::All, bool inherited = true) const;

    template <typename _Facet>
    auto FunctionsByTag() const {
        return AllFunctions().Select(
            [](const FMetaFunction* func) -> Meta::TOptional<TPair<const FMetaFunction*, const _Facet*>> {
                if (const _Facet* facet = UserFacetIFP<_Facet>(*func))
                    return MakePair(func, facet);
                return {};
            });
    }

    virtual const FMetaFunction* OnMissingFunction(const FName& name, EFunctionFlags flags = EFunctionFlags::All) const;

    // Properties

    size_t NumProperties(bool inherited = true) const {
        return (inherited ? _propertiesAll.size() : _propertiesSelf.size());
    }

    FMetaProperty& RegisterProperty(FMetaProperty&& property);

    TIterable<TValueIterator<HASHMAP(MetaClass, FName, const FMetaProperty*)::const_iterator>> AllProperties() const { return _propertiesAll.Values(); }

    TMemoryView<const FMetaProperty> SelfProperties() const { return _propertiesSelf.MakeConstView(); }

    bool HasProperty(const FMetaProperty& property, bool inherited = true) const NOEXCEPT;

    const FMetaProperty& Property(const FName& name, EPropertyFlags flags = EPropertyFlags::All, bool inherited = true) const;
    const FMetaProperty* PropertyIFP(const FName& name, EPropertyFlags flags = EPropertyFlags::All, bool inherited = true) const;
    const FMetaProperty* PropertyIFP(const FStringView& name, EPropertyFlags flags = EPropertyFlags::All, bool inherited = true) const;
    const FMetaProperty* PropertyIFP(const FLazyName& name, EPropertyFlags flags = EPropertyFlags::All, bool inherited = true) const;

    template <typename _Facet>
    auto PropertiesByTag() const {
        return AllProperties().Select(
            [](const FMetaProperty* prop) -> Meta::TOptional<TPair<const FMetaProperty*, const _Facet*>> {
                if (const _Facet* facet = UserFacetIFP<_Facet>(*prop))
                    return MakePair(prop, facet);
                return {};
            });
    }

    virtual const FMetaProperty* OnMissingProperty(const FName& name, EPropertyFlags flags = EPropertyFlags::All) const;

    // Virtual helpers

    virtual const FMetaClass* Parent() const NOEXCEPT = 0;
    virtual bool CreateInstance(PMetaObject& dst, bool resetToDefaultValue = true) const = 0;
    virtual PTypeTraits MakeTraits() const NOEXCEPT = 0;

    // Called by meta namespace

    void CallOnRegister_IFN();

    virtual void OnRegister();
    virtual void OnUnregister();

private:
    FClassId _id;
    EClassFlags _flags;

    const FName _name;
    const FMetaModule* _module;

    HASHMAP(MetaClass, FName, const FMetaProperty*) _propertiesAll;
    HASHMAP(MetaClass, FName, const FMetaFunction*) _functionsAll;

    VECTORINSITU(MetaClass, FMetaProperty, 8) _propertiesSelf;
    VECTORINSITU(MetaClass, FMetaFunction, 4) _functionsSelf;

    mutable VECTORINSITU(MetaClass, const FMetaClass*, 1) _children;

    FMetaClassFacet _facets;

#if USE_PPE_MEMORYDOMAINS
public:
    FMemoryTracking& TrackingData() const { return _trackingData; }
protected: // for access in CreateInstance()
    mutable FMemoryTracking _trackingData;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Class>
using TMetaClass = typename _Class::RTTI_FMetaClass;
//----------------------------------------------------------------------------
template <typename _Class>
const FMetaClass* MetaClass() {
    STATIC_ASSERT(std::is_base_of_v<FMetaObject, _Class>);
    return TMetaClass<_Class>::Get();
}
//----------------------------------------------------------------------------
inline FStringLiteral MetaClassName(const FMetaClass* metaClass) {
    Assert(metaClass);
    return metaClass->Name().MakeLiteral();
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
PPE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, RTTI::EClassFlags flags);
PPE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, RTTI::EClassFlags flags);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
