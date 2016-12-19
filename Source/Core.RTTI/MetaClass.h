#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core/Container/Vector.h"
#include "Core/Memory/UniquePtr.h"

#include "Core.RTTI/MetaClassSingleton.h"
#include "Core.RTTI/MetaType.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMetaClassHashMap;
class FMetaObject;
FWD_UNIQUEPTR(MetaProperty);
//----------------------------------------------------------------------------
class FMetaClass {
public:
    friend class FMetaClassList;

    enum EFlags {
        Concrete    = 1<<0,
        Abstract    = 1<<1,
        Dynamic     = 1<<2,
        Mergeable   = 1<<3,
        Deprecated  = 1<<4,

        Default     = Concrete
    };

    FMetaClass(const FName& name, EFlags attributes);
    virtual ~FMetaClass();

    FMetaClass(const FMetaClass&) = delete;
    FMetaClass& operator =(const FMetaClass&) = delete;

    const FName& Name() const { return _name; }
    EFlags Attributes() const { return _attributes; }

    bool IsAbstract()   const { return Meta::HasFlag(_attributes, Abstract); }
    bool IsConcrete()   const { return Meta::HasFlag(_attributes, Concrete); }
    bool IsDynamic()    const { return Meta::HasFlag(_attributes, Dynamic); }
    bool IsMergeable()  const { return Meta::HasFlag(_attributes, Mergeable); }
    bool IsDeprecated() const { return Meta::HasFlag(_attributes, Deprecated); }

    bool InheritsFrom(const FMetaClass *parent) const;
    bool IsAssignableFrom(const FMetaClass *child) const;

    template <typename T>
    bool InheritsFrom() const {
        typedef typename T::FMetaClass metaclass_type;
        return InheritsFrom(metaclass_type::Instance());
    }

    template <typename T>
    bool IsAssignableFrom() const {
        typedef typename T::FMetaClass metaclass_type;
        return IsAssignableFrom(metaclass_type::Instance());
    }

    void Register(FMetaClassHashMap& database) const;
    void Unregister(FMetaClassHashMap& database) const;

    const FMetaClass *Parent() const;

    TMemoryView<const UCMetaProperty> Properties() const;

    const FMetaProperty *PropertyIFP(const FStringView& name, size_t attributes = 0, bool inherited = true) const;
    const FMetaProperty *PropertyIFP(const FName& name, size_t attributes = 0, bool inherited = true) const;

    FMetaObject* CreateInstance() const;

protected:
    virtual const FMetaClass* VirtualParent() const = 0;

    virtual TMemoryView<const UCMetaProperty> VirtualProperties() const = 0;

    virtual const FMetaProperty *VirtualPropertyIFP(const FStringView& name, size_t attributes) const = 0;
    virtual const FMetaProperty *VirtualPropertyIFP(const FName& name, size_t attributes) const = 0;

    virtual FMetaObject* VirtualCreateInstance() const = 0;

private:
    FName _name;
    EFlags _attributes;
};
//----------------------------------------------------------------------------
template <typename _Visitor = void (*)(const FMetaClass* metaClass, const FMetaProperty* prop) >
void ForEachProperty(const FMetaClass* metaClass, const _Visitor& visitor) {
    while (nullptr != metaClass) {
        for (const UCMetaProperty& prop : metaClass->Properties()) {
            Assert(prop);
            visitor(metaClass, prop.get());
        }
        metaClass = metaClass->Parent();
    }
}
//----------------------------------------------------------------------------
template <typename _Pred = bool (*)(const FMetaClass* metaClass, const FMetaProperty* prop) >
const FMetaProperty* FindProperty(const FMetaClass* metaClass, const _Pred& pred) {
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
class InScopeMetaClass : public FMetaClass {
public:
    InScopeMetaClass(const FName& name, EFlags attributes);
    virtual ~InScopeMetaClass();

protected:
    void RegisterProperty(UCMetaProperty&& prop);

private:
    virtual TMemoryView<const UCMetaProperty> VirtualProperties() const override final;

    virtual const FMetaProperty *VirtualPropertyIFP(const FStringView& name, size_t attributes) const override final;
    virtual const FMetaProperty *VirtualPropertyIFP(const FName& name, size_t attributes) const override final;

    VECTOR(RTTI, UCMetaProperty) _properties;
};
//----------------------------------------------------------------------------
template <typename T>
class TDefaultMetaClass : public InScopeMetaClass {
public:
    TDefaultMetaClass(const FName& name, EFlags attributes)
        : InScopeMetaClass(name, attributes) {
        STATIC_ASSERT(not std::is_abstract<TDefaultMetaClass>::value);
    }

protected:
    virtual const FMetaClass* VirtualParent() const override final {
        typedef typename T::FMetaClass metaclass_type;
        typedef typename metaclass_type::parent_type parent_type;
        return GetMetaClass<parent_type>();
    }

    virtual FMetaObject* VirtualCreateInstance() const override final {
        typedef typename std::is_default_constructible<T>::type constructible_type;
        return CreateInstance_<T>(constructible_type());
    }

private:
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
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
