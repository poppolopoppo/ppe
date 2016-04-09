#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core/Container/Vector.h"
#include "Core/Memory/UniquePtr.h"

#include "Core.RTTI/MetaClassName.h"
#include "Core.RTTI/MetaClassSingleton.h"
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
    friend class MetaClassList;

    enum Flags {
        Concrete    = 0<<0,
        Abstract    = 1<<0,
        Dynamic     = 1<<1,
        Mergeable   = 1<<2,
        Deprecated  = 1<<3,

        Default     = Concrete
    };

    MetaClass(const MetaClassName& name, Flags attributes);
    virtual ~MetaClass();

    MetaClass(const MetaClass&) = delete;
    MetaClass& operator =(const MetaClass&) = delete;

    const MetaClassName& Name() const { return _name; }
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

    const MetaClass *Parent() const;

    MemoryView<const UCMetaProperty> Properties() const;

    const MetaProperty *PropertyIFP(const StringSlice& name, size_t attributes = 0, bool inherited = true) const;
    const MetaProperty *PropertyIFP(const MetaPropertyName& name, size_t attributes = 0, bool inherited = true) const;

    MetaObject* CreateInstance() const;

protected:
    virtual const MetaClass* VirtualParent() const = 0;

    virtual MemoryView<const UCMetaProperty> VirtualProperties() const = 0;

    virtual const MetaProperty *VirtualPropertyIFP(const StringSlice& name, size_t attributes) const = 0;
    virtual const MetaProperty *VirtualPropertyIFP(const MetaPropertyName& name, size_t attributes) const = 0;

    virtual MetaObject* VirtualCreateInstance() const = 0;

private:
    MetaClassName _name;
    Flags _attributes;
};
//----------------------------------------------------------------------------
template <typename _Visitor = void (*)(const MetaClass* metaClass, const MetaProperty* prop) >
void ForEachProperty(const MetaClass* metaClass, const _Visitor& visitor) {
    while (nullptr != metaClass) {
        for (const UCMetaProperty& prop : metaClass->Properties()) {
            Assert(prop);
            visitor(metaClass, prop.get());
        }
        metaClass = metaClass->Parent();
    }
}
//----------------------------------------------------------------------------
template <typename _Pred = bool (*)(const MetaClass* metaClass, const MetaProperty* prop) >
const MetaProperty* FindProperty(const MetaClass* metaClass, const _Pred& pred) {
    while (nullptr != metaClass) {
        for (const UCMetaProperty& prop : metaClass->Properties()) {
            Assert(prop);
            if (pred(metaClass, prop.get()))
                return prop.get();
        }
        metaClass = metaClass->Parent();
    }
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class InScopeMetaClass : public MetaClass {
public:
    InScopeMetaClass(const MetaClassName& name, Flags attributes);
    virtual ~InScopeMetaClass();

protected:
    void RegisterProperty(UCMetaProperty&& prop);

private:
    virtual MemoryView<const UCMetaProperty> VirtualProperties() const override;

    virtual const MetaProperty *VirtualPropertyIFP(const StringSlice& name, size_t attributes) const override;
    virtual const MetaProperty *VirtualPropertyIFP(const MetaPropertyName& name, size_t attributes) const override;

    VECTOR(RTTI, UCMetaProperty) _properties;
};
//----------------------------------------------------------------------------
template <typename T>
class DefaultMetaClass : public InScopeMetaClass {
public:
    DefaultMetaClass(const MetaClassName& name, Flags attributes)
        : InScopeMetaClass(name, attributes) {
        STATIC_ASSERT(not std::is_abstract<DefaultMetaClass>::value);
    }

protected:
    virtual const MetaClass* VirtualParent() const override {
        typedef typename T::MetaClass metaclass_type;
        typedef typename metaclass_type::parent_type parent_type;
        return GetMetaClass<parent_type>();
    }

    virtual MetaObject* VirtualCreateInstance() const override {
        typedef typename std::is_default_constructible<T>::type constructible_type;
        return CreateInstance_<T>(constructible_type());
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
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
