#pragma once

#include "RTTI.h"

#include "Typedefs.h"
#include "MetaNamespace.h"
#include "MetaFunction.h"
#include "MetaProperty.h"

#include "Container/HashMap.h"
#include "Container/Vector.h"
#include "Memory/MemoryDomain.h"
#include "IO/TextWriter_fwd.h"

#if USE_PPE_MEMORYDOMAINS
#   define NEW_RTTI(T) new (*::PPE::RTTI::MetaClass<T>()) T
#else
#   define NEW_RTTI(T) NEW_REF(MetaObject, T)
#endif

namespace PPE {
namespace RTTI {
FWD_REFPTR(MetaObject);
class FMetaNamespace;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EClassFlags {
    Concrete    = 1<<0,
    Abstract    = 1<<1,
    Dynamic     = 1<<2,

    Private     = 0,
    Public      = 1<<3,

    Mergeable   = 1<<4,
    Deprecated  = 1<<5,

    Registered  = 1<<6,
};
ENUM_FLAGS(EClassFlags);
//----------------------------------------------------------------------------
template <typename T> const FMetaClass* MetaClass();
//----------------------------------------------------------------------------
class PPE_RTTI_API FMetaClass {
public:
    FMetaClass(FClassId id, const FName& name, EClassFlags flags, const FMetaNamespace* metaNamespace);
    virtual ~FMetaClass();

    FMetaClass(const FMetaClass& ) = delete;
    FMetaClass& operator =(const FMetaClass& ) = delete;

    FClassId Id() const { return _id; }
    const FName& Name() const { return _name; }
    EClassFlags Flags() const { return _flags; }
    const FMetaNamespace* Namespace() const { return _namespace; }

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

    // Functions

    size_t NumFunctions(bool inherited = true) const {
        return (inherited ? _functionsAll.size() : _functionsSelf.size());
    }

    void RegisterFunction(FMetaFunction&& function);

    auto AllFunctions() const { return _functionsAll.Values(); }

    TMemoryView<const FMetaFunction> SelfFunctions() const { return _functionsSelf.MakeConstView(); }

    const FMetaFunction& Function(const FName& name, EFunctionFlags flags = EFunctionFlags(0), bool inherited = true) const;
    const FMetaFunction* FunctionIFP(const FName& name, EFunctionFlags flags = EFunctionFlags(0), bool inherited = true) const;
    const FMetaFunction* FunctionIFP(const FStringView& name, EFunctionFlags flags = EFunctionFlags(0), bool inherited = true) const;

    virtual const FMetaFunction* OnMissingFunction(const FName& name, EFunctionFlags flags = EFunctionFlags(0)) const;

    // Properties

    size_t NumProperties(bool inherited = true) const {
        return (inherited ? _propertiesAll.size() : _propertiesSelf.size());
    }

    void RegisterProperty(FMetaProperty&& property);

    auto AllProperties() const { return _propertiesAll.Values(); }

    TMemoryView<const FMetaProperty> SelfProperties() const { return _propertiesSelf.MakeConstView(); }

    const FMetaProperty& Property(const FName& name, EPropertyFlags flags = EPropertyFlags(0), bool inherited = true) const;
    const FMetaProperty* PropertyIFP(const FName& name, EPropertyFlags flags = EPropertyFlags(0), bool inherited = true) const;
    const FMetaProperty* PropertyIFP(const FStringView& name, EPropertyFlags flags = EPropertyFlags(0), bool inherited = true) const;

    virtual const FMetaProperty* OnMissingProperty(const FName& name, EPropertyFlags flags = EPropertyFlags(0)) const;

    // Virtual helpers

    virtual const FMetaClass* Parent() const = 0;

    virtual bool CreateInstance(PMetaObject& dst, bool resetToDefaultValue = true) const = 0;

    // Called by meta namespace

    void CallOnRegister_IFN();

    virtual void OnRegister();
    virtual void OnUnregister();

private:
    FClassId _id;
    EClassFlags _flags;

    const FName _name;
    const FMetaNamespace* _namespace;

    HASHMAP(MetaClass, FName, const FMetaProperty*) _propertiesAll;
    HASHMAP(MetaClass, FName, const FMetaFunction*) _functionsAll;

    VECTORINSITU(MetaClass, FMetaProperty, 8) _propertiesSelf;
    VECTORINSITU(MetaClass, FMetaFunction, 4) _functionsSelf;

#if USE_PPE_MEMORYDOMAINS
public:
    FMemoryTracking& TrackingData() const { return _trackingData; }
protected: // for access in CreateInstance()
    mutable FMemoryTracking _trackingData;
#endif
};
//----------------------------------------------------------------------------
template <typename T>
using TMetaClass = typename T::RTTI_FMetaClass;
//----------------------------------------------------------------------------
namespace details {
template <typename T>
static const FMetaClass* MetaClass_(std::false_type) { return nullptr; }
template <typename T>
static const FMetaClass* MetaClass_(std::true_type) { return TMetaClass<T>::Get(); }
} //!details
template <typename T>
const FMetaClass* MetaClass() {
    return details::MetaClass_< Meta::TDecay<T> >(
        typename std::is_base_of< FMetaObject, Meta::TDecay<T> >::type {}
    );
}
//----------------------------------------------------------------------------
inline FStringView MetaClassName(const FMetaClass* metaClass) {
    Assert(metaClass);
    return metaClass->Name().MakeView();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename T>
bool CreateMetaObject_(PMetaObject& dst, bool resetToDefaultValue, std::true_type ) {
    dst.reset(NEW_RTTI(T)());
    Assert(dst);
    if (resetToDefaultValue)
        dst->RTTI_ResetToDefaultValue();
    return true;
}
template <typename T>
FMetaObject* CreateMetaObject_(PMetaObject&, bool resetToDefaultValue, std::false_type) {
    return false;
}
inline void DeleteMetaClass_(FMetaClass* metaClass) {
    checked_delete(metaClass);
}
} //!details
//----------------------------------------------------------------------------
template <typename T>
class TInScopeMetaClass : public FMetaClass {
protected:
    TInScopeMetaClass(FClassId id, const FName& name, EClassFlags flags, const FMetaNamespace* metaNamespace)
        : FMetaClass(id, name, FlagsForT_(flags), metaNamespace)
    {}

public:
    virtual const FMetaClass* Parent() const override final {
        typedef typename TMetaClass<T>::parent_type parent_type;
        return RTTI::MetaClass<parent_type>();
    }

    virtual bool CreateInstance(PMetaObject& dst, bool resetToDefaultValue = true) const override final {
        Assert(not dst);
        return details::CreateMetaObject_<T>(dst, resetToDefaultValue, typename std::is_default_constructible<T>::type{});
    }

    static const FMetaClass* Get() {
        Assert(GMetaClassHandle.Class());
        return GMetaClassHandle.Class();
    }

private:
    static const FMetaClassHandle GMetaClassHandle;

    static constexpr EClassFlags FlagsForT_(EClassFlags flags) {
        return (std::is_abstract<T>::value
            ? flags + EClassFlags::Abstract
            : flags + EClassFlags::Concrete );
    }

    static FMetaClass* CreateMetaClass_(FClassId id, const FMetaNamespace* metaNamespace) {
        return new TMetaClass<T>(id, metaNamespace);
    }
};
//----------------------------------------------------------------------------
template <typename T>
const FMetaClassHandle TInScopeMetaClass<T>::GMetaClassHandle(
    TMetaClass<T>::Namespace(),
    &TInScopeMetaClass<T>::CreateMetaClass_,
    &details::DeleteMetaClass_
);
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