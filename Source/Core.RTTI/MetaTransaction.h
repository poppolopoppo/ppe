#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core/Container/HashMap.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace RTTI {
class MetaAtomHashMap;
class MetaClassHashMap;
class MetaClassName;
class MetaObjectName;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(MetaAtom);
FWD_REFPTR(MetaObject);
class MetaClass;
//----------------------------------------------------------------------------
class MetaTransaction : public RefCountable {
public:
    MetaTransaction();
    ~MetaTransaction();

    MetaTransaction(const MetaTransaction&) = delete;
    MetaTransaction& operator =(const MetaTransaction&) = delete;

    void Export(const MetaClassName& name, const MetaClass *metaclass, bool allowOverride);
    const MetaClass* GetIFP(const MetaClassName& name) const;
    void Remove(const MetaClassName& name, const MetaClass *metaclass);

    void Export(const MetaObjectName& name, MetaAtom *atom, bool allowOverride);
    void Export(const MetaObjectName& name, MetaObject *object, bool allowOverride);
    const MetaAtom* GetIFP(const MetaObjectName& name) const;
    void Remove(const MetaObjectName& name, MetaAtom *atom);

    void Clear();

private:
    struct MetaAtomBinding {
        PMetaAtom       Old;
        PMetaAtom       New;
    };

    struct MetaClassBinding {
        const MetaClass *Old;
        const MetaClass *New;
    };

    static void RevertBinding_(MetaAtomHashMap& instances, const MetaObjectName& name, const MetaAtomBinding& binding);
    static void RevertBinding_(MetaClassHashMap& instances, const MetaClassName& name, const MetaClassBinding& binding);

    HASHMAP(RTTI, MetaObjectName, MetaAtomBinding) _atoms;
    HASHMAP(RTTI, MetaClassName, MetaClassBinding) _classes;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
