#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/Typedefs.h"
#include "Core.RTTI/MetaNamespace.h"

#include "Core/Container/HashMap.h"
#include "Core/Container/Vector.h"
#include "Core/Memory/UniquePtr.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMetaObject;
FWD_UNIQUEPTR(MetaFunction);
FWD_UNIQUEPTR(MetaProperty);
//----------------------------------------------------------------------------
class FMetaClassGuid {
public:
    FMetaClassGuid() : FMetaClassGuid(0) {}

    FMetaClassGuid(const FMetaClassGuid&) = default;
    FMetaClassGuid& operator =(const FMetaClassGuid&) = default;

    bool InheritsFrom(FMetaClassGuid parent) const;
    FMetaClassGuid Combine(FMetaClassGuid other) const;

    inline friend bool operator ==(FMetaClassGuid lhs, FMetaClassGuid rhs) { return (lhs._value == rhs._value); }
    inline friend bool operator !=(FMetaClassGuid lhs, FMetaClassGuid rhs) { return (lhs._value != rhs._value); }

    inline friend bool operator < (FMetaClassGuid lhs, FMetaClassGuid rhs) { return (lhs._value <  rhs._value); }
    inline friend bool operator >=(FMetaClassGuid lhs, FMetaClassGuid rhs) { return (lhs._value >= rhs._value); }

    template <typename _Char, typename _Traits>
    inline friend std::basic_ostream<_Char, _Traits>& operator <<(
        std::basic_ostream<_Char, _Traits>& oss,
        const FMetaClassGuid& guid) {
        return (oss << guid._value);
    }

private:
    friend class FMetaNamespace;
    FMetaClassGuid(u64 value) : _value(value) {}

    u64 _value;
};
//----------------------------------------------------------------------------
template <typename T> // valid RTTI parent
static const RTTI::FMetaClass *MetaClass(typename std::enable_if< std::is_base_of<RTTI::FMetaObject, T>::value >::type* = 0) {
    typedef Meta::TDecay<T> metaobject_type;
    typedef typename metaobject_type::FMetaClass metaclass_type;
    return metaclass_type::Instance();
}
template <typename T> // no parent
static const RTTI::FMetaClass *MetaClass(typename std::enable_if< std::is_void<T>::value >::type* = 0) {
    return nullptr;
}
//----------------------------------------------------------------------------
class FMetaClass {
public:
    enum EFlags {
        Concrete    = 1<<0,
        Abstract    = 1<<1,
        Dynamic     = 1<<2,
        Mergeable   = 1<<3,
        Deprecated  = 1<<4,

        Default     = Concrete
    };
    ENUM_FLAGS_FRIEND(EFlags);

protected:
    FMetaClass(
        FMetaClassGuid guid,
        EFlags attributes,
        const FName& name,
        const FMetaNamespace* metaNamespace );

public:
    virtual ~FMetaClass();

    FMetaClass(const FMetaClass&) = delete;
    FMetaClass& operator =(const FMetaClass&) = delete;

    FMetaClassGuid Guid() const { return _guid; } // this id can change across multiple runs !
    EFlags Attributes() const { return _attributes; }
    const FName& Name() const { return _name; }
    const FMetaNamespace* Namespace() const { return _namespace; }

    bool IsAbstract()   const { return (_attributes ^ Abstract); }
    bool IsConcrete()   const { return (_attributes ^ Concrete); }
    bool IsDynamic()    const { return (_attributes ^ Dynamic); }
    bool IsMergeable()  const { return (_attributes ^ Mergeable); }
    bool IsDeprecated() const { return (_attributes ^ Deprecated); }

    bool CastTo(const FMetaClass* other) const { return (InheritsFrom(other) || IsAssignableFrom(this)); }
    bool InheritsFrom(const FMetaClass* parent) const { Assert(parent); return (_guid.InheritsFrom(parent->_guid)); }
    bool IsAssignableFrom(const FMetaClass* child) const { Assert(child); return (child->_guid.InheritsFrom(_guid)); }

    template <typename T>
    bool CastTo() const { return CastTo(RTTI::MetaClass<T>()); }
    template <typename T>
    bool InheritsFrom() const { return InheritsFrom(RTTI::MetaClass<T>()); }
    template <typename T>
    bool IsAssignableFrom() const { return IsAssignableFrom(RTTI::MetaClass<T>()); }

    auto AllFunctions() const { return _functionsInherited.Values(); }
    auto AllProperties() const { return _propertiesInherited.Values(); }

    TMemoryView<const UCMetaFunction> Functions() const { return _functions.MakeConstView(); }
    TMemoryView<const UCMetaProperty> Properties() const { return _properties.MakeConstView(); }

    const FMetaFunction* Function(const FName& name, size_t attributes = 0, bool inherited = true) const;
    const FMetaFunction* Function(const FStringView& name, size_t attributes = 0, bool inherited = true) const;

    const FMetaFunction* FunctionIFP(const FName& name, size_t attributes = 0, bool inherited = true) const;
    const FMetaFunction* FunctionIFP(const FStringView& name, size_t attributes = 0, bool inherited = true) const;

    const FMetaProperty* Property(const FName& name, size_t attributes = 0, bool inherited = true) const;
    const FMetaProperty* Property(const FStringView& name, size_t attributes = 0, bool inherited = true) const;

    const FMetaProperty* PropertyIFP(const FName& name, size_t attributes = 0, bool inherited = true) const;
    const FMetaProperty* PropertyIFP(const FStringView& name, size_t attributes = 0, bool inherited = true) const;

    virtual const FMetaClass* Parent() const = 0;
    virtual FMetaObject* CreateInstance() const = 0;

    void Initialize(); // called by namespace

protected:
    void RegisterFunction(UCMetaFunction&& func);
    void RegisterProperty(UCMetaProperty&& prop);

    // Only available for dynamic metaclasses
    virtual const FMetaFunction* OnMissingFunction(const FName& name, size_t attributes = 0) const = 0;
    virtual const FMetaProperty* OnMissingProperty(const FName& name, size_t attributes = 0) const = 0;

private:
    FMetaClassGuid _guid;
    const EFlags _attributes;

    const FName _name;
    const FMetaNamespace* const _namespace;

    VECTOR(RTTI, UCMetaFunction) _functions;
    VECTOR(RTTI, UCMetaProperty) _properties;

    HASHMAP(RTTI, FName, const FMetaFunction*) _functionsInherited;
    HASHMAP(RTTI, FName, const FMetaProperty*) _propertiesInherited;
};
//----------------------------------------------------------------------------
template <typename _Visitor = void(*)(const FMetaClass* metaClass, const FMetaFunction* func) >
void ForEachFunction(const FMetaClass* metaClass, const _Visitor& visitor);
//----------------------------------------------------------------------------
template <typename _Pred = bool(*)(const FMetaClass* metaClass, const FMetaFunction* func) >
const FMetaFunction* FindFunction(const FMetaClass* metaClass, const _Pred& pred);
//----------------------------------------------------------------------------
template <typename _Visitor = void (*)(const FMetaClass* metaClass, const FMetaProperty* prop) >
void ForEachProperty(const FMetaClass* metaClass, const _Visitor& visitor);
//----------------------------------------------------------------------------
template <typename _Pred = bool (*)(const FMetaClass* metaClass, const FMetaProperty* prop) >
const FMetaProperty* FindProperty(const FMetaClass* metaClass, const _Pred& pred);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TInScopeMetaClass : public FMetaClass {
public:
    using FMetaClass::EFlags;

    virtual const FMetaClass* Parent() const override final {
        typedef typename T::FMetaClass metaclass_type;
        typedef typename metaclass_type::parent_type parent_type;
        return RTTI::MetaClass<parent_type>();
    }

    virtual FMetaObject* CreateInstance() const override final {
        typedef typename std::is_default_constructible<T>::type constructible_type;
        return CreateInstance_<T>(constructible_type());
    }

    static bool HasInstance() {
        return (nullptr != gMetaClassHandle.MetaClass());
    }

    static const FMetaClass* Instance() {
        Assert(gMetaClassHandle.MetaClass());
        return gMetaClassHandle.MetaClass();
    }

    static const FMetaClassHandle& Handle() { return gMetaClassHandle; }

protected:
    TInScopeMetaClass(
        FMetaClassGuid guid,
        EFlags attributes,
        const FName& name,
        const FMetaNamespace* metaNamespace )
        : FMetaClass(guid, InferAttributes_(attributes), name, metaNamespace) {}

    virtual const FMetaFunction* OnMissingFunction(const FName& , size_t ) const override { AssertNotReached(); return nullptr; }
    virtual const FMetaProperty* OnMissingProperty(const FName& , size_t ) const override { AssertNotReached(); return nullptr; }

private:
    static EFlags InferAttributes_(EFlags attributes) {
        return (attributes | (std::is_abstract<T>::value ? EFlags::Abstract : EFlags::Concrete));
    }

    template <typename U>
    static FMetaObject* CreateInstance_(std::true_type) {
        STATIC_ASSERT(std::is_same<T, U>::value);
        FMetaObject* const obj = new U();
        Assert(obj);
        return obj;
    }

    template <typename U>
    static FMetaObject* CreateInstance_(std::false_type) {
        AssertNotReached(); // abstract class
        return nullptr;
    }

    static void CreateMetaClass_(
        const FMetaClass** pMetaClass,
        FMetaClassGuid classGuid,
        const FMetaNamespace* metaNamespace) {
        typedef typename T::FMetaClass metaclass_type;
        *pMetaClass = new metaclass_type(classGuid, metaNamespace);
    }

    static const FMetaClassHandle gMetaClassHandle;
};
//----------------------------------------------------------------------------
template <typename T>
const FMetaClassHandle TInScopeMetaClass<T>::gMetaClassHandle(
    T::FMetaClass::Namespace(),
    &TInScopeMetaClass<T>::CreateMetaClass_ );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

#include "Core.RTTI/MetaClass-inl.h"
