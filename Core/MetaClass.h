#pragma once

#include "Core.h"
#include "AssociativeVector.h"
#include "PointerWFlags.h"

#include "MetaClassName.h"
#include "MetaPropertyName.h"

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
    };

    MetaClass(const MetaClassName& name, Flags attributes, const MetaClass *parent);
    virtual ~MetaClass();

    MetaClass(const MetaClass&) = delete;
    MetaClass& operator =(const MetaClass&) = delete;

    const MetaClassName& Name() const { return _name; }
    Flags Attributes() const { return static_cast<Flags>(_parentWFlags.Flag01()); }
    const MetaClass *Parent() const { return _parentWFlags.Get(); }
    const ASSOCIATIVE_VECTOR(RTTI, MetaPropertyName, const MetaProperty *)& Properties() const { return _properties; }

    const MetaProperty *PropertyIFP(const char *name, size_t attributes = 0, bool inherited = true) const;
    const MetaProperty *PropertyIFP(const MetaPropertyName& name, size_t attributes = 0, bool inherited = true) const;

    bool InheritsFrom(const MetaClass *parent) const;
    bool IsAssignableFrom(const MetaClass *child) const;

    void Register(MetaClassHashMap& database) const;
    void Unregister(MetaClassHashMap& database) const;

    virtual MetaObject *CreateInstance() const = 0;

protected:
    MetaClassName _name;
    Meta::PointerWFlags<const MetaClass> _parentWFlags;
    ASSOCIATIVE_VECTOR(RTTI, MetaPropertyName, const MetaProperty *) _properties;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
