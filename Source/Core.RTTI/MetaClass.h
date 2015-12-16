#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core/Container/AssociativeVector.h"

#include "Core.RTTI/MetaClassName.h"
#include "Core.RTTI/MetaPropertyName.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MetaClassHashMap;
class MetaObject;
class MetaProperty;
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
    const MemoryView<const Pair<MetaPropertyName, const MetaProperty *>> Properties() const { return _properties.MakeView(); }

    bool IsAbstract()   const { return Meta::HasFlag(_attributes, Abstract); }
    bool IsConcrete()   const { return Meta::HasFlag(_attributes, Concrete); }
    bool IsDynamic()    const { return Meta::HasFlag(_attributes, Dynamic); }
    bool IsMergeable()  const { return Meta::HasFlag(_attributes, Mergeable); }
    bool IsDeprecated() const { return Meta::HasFlag(_attributes, Deprecated); }

    const MetaProperty *PropertyIFP(const char *name, size_t attributes = 0, bool inherited = true) const;
    const MetaProperty *PropertyIFP(const MetaPropertyName& name, size_t attributes = 0, bool inherited = true) const;

    bool InheritsFrom(const MetaClass *parent) const;
    bool IsAssignableFrom(const MetaClass *child) const;

    void Register(MetaClassHashMap& database) const;
    void Unregister(MetaClassHashMap& database) const;

    MetaObject* CreateInstance() const;

protected:
    virtual MetaObject *VirtualCreateInstance() const = 0;

    MetaClassName _name;
    const MetaClass* _parent;
    Flags _attributes;
    ASSOCIATIVE_VECTOR(RTTI, MetaPropertyName, const MetaProperty *) _properties;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
