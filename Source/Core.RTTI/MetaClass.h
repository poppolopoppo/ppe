#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/Memory/UniquePtr.h"

#include "Core.RTTI/MetaClassName.h"
#include "Core.RTTI/MetaPropertyName.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MetaClassHashMap;
class MetaObject;
FWD_UNIQUEPTR(MetaProperty);
//----------------------------------------------------------------------------
class MetaClass {
public:
    enum Flags {
        Concrete    = 0<<0,
        Abstract    = 1<<0,
        Dynamic     = 1<<1,
        Mergeable   = 1<<2,
        Deprecated  = 1<<3,

        Default     = Concrete
    };

    MetaClass(const MetaClassName& name, Flags attributes, const MetaClass *parent);
    virtual ~MetaClass();

    MetaClass(const MetaClass&) = delete;
    MetaClass& operator =(const MetaClass&) = delete;

    const MetaClassName& Name() const { return _name; }
    const MetaClass *Parent() const { return _parent; }
    Flags Attributes() const { return _attributes; }

    bool IsAbstract()   const { return Meta::HasFlag(_attributes, Abstract); }
    bool IsConcrete()   const { return Meta::HasFlag(_attributes, Concrete); }
    bool IsDynamic()    const { return Meta::HasFlag(_attributes, Dynamic); }
    bool IsMergeable()  const { return Meta::HasFlag(_attributes, Mergeable); }
    bool IsDeprecated() const { return Meta::HasFlag(_attributes, Deprecated); }

    bool InheritsFrom(const MetaClass *parent) const;
    bool IsAssignableFrom(const MetaClass *child) const;

    void Register(MetaClassHashMap& database) const;
    void Unregister(MetaClassHashMap& database) const;

    MemoryView<const UCMetaProperty> Properties() const;

    const MetaProperty *PropertyIFP(const char *name, size_t attributes = 0, bool inherited = true) const;
    const MetaProperty *PropertyIFP(const MetaPropertyName& name, size_t attributes = 0, bool inherited = true) const;

    MetaObject* CreateInstance() const;

protected:
    virtual MemoryView<const UCMetaProperty> VirtualProperties() const = 0;

    virtual const MetaProperty *VirtualPropertyIFP(const char *name, size_t attributes) const = 0;
    virtual const MetaProperty *VirtualPropertyIFP(const MetaPropertyName& name, size_t attributes) const = 0;

    virtual MetaObject* VirtualCreateInstance() const = 0;

private:
    MetaClassName _name;
    const MetaClass* _parent;
    Flags _attributes;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class InScopeMetaClass : public MetaClass {
public:
    InScopeMetaClass(const MetaClassName& name, Flags attributes, const MetaClass *parent);
    virtual ~InScopeMetaClass();

protected:
    void RegisterProperty(UCMetaProperty&& prop);

private:
    virtual MemoryView<const UCMetaProperty> VirtualProperties() const override;

    virtual const MetaProperty *VirtualPropertyIFP(const char *name, size_t attributes) const override;
    virtual const MetaProperty *VirtualPropertyIFP(const MetaPropertyName& name, size_t attributes) const override;

    VECTOR(RTTI, UCMetaProperty) _properties;
};
//----------------------------------------------------------------------------
template <typename T>
class DefaultMetaClass : public InScopeMetaClass {
public:
    DefaultMetaClass(const MetaClassName& name, Flags attributes, const MetaClass *parent)
        : InScopeMetaClass(name, attributes, parent) {
        STATIC_ASSERT(not std::is_abstract<DefaultMetaClass>::value);
    }

private:
    template <typename U>
    static MetaObject* CreateInstance_(std::true_type) {
        STATIC_ASSERT(std::is_same<T, U>::value);
        MetaObject* const obj = new U();
        Assert(obj);
        return obj;
    }

    template <typename U>
    static MetaObject* CreateInstance_(std::false_type) {
        AssertNotReached(); // abstract class
        return nullptr;
    }

    virtual MetaObject* VirtualCreateInstance() const override {
        typedef typename std::is_default_constructible<T>::type constructible_type;
        return CreateInstance_<T>(constructible_type());
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
