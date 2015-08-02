#pragma once

#include "Core/Core.h"

#include "Core/Container/HashMap.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Meta/Singleton.h"

#include "Core.RTTI/Object/MetaObjectName.h"

#include <mutex>

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(MetaAtom);
FWD_REFPTR(MetaObject);
//----------------------------------------------------------------------------
class MetaAtomHashMap {
public:
    MetaAtomHashMap();
    ~MetaAtomHashMap();

    MetaAtomHashMap(const MetaAtomHashMap&) = delete;
    MetaAtomHashMap& operator =(const MetaAtomHashMap&) = delete;

    void Add(const MetaObjectName& name, MetaAtom *metaAtom, bool allowOverride);
    void Remove(const MetaObjectName& name, MetaAtom *metaAtom);

    void Add(MetaObject *object);
    void Remove(MetaObject *object);

    MetaAtom *GetIFP(const MetaObjectName& name) const;

    void Clear();

private:
    mutable std::mutex _barrier;
    HASHMAP(RTTI, MetaObjectName, PMetaAtom) _objects;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MetaAtomDatabase : Meta::Singleton<MetaAtomHashMap, MetaAtomDatabase> {
    typedef Meta::Singleton<MetaAtomHashMap, MetaAtomDatabase> parent_type;
public:
    using parent_type::Instance;
    using parent_type::HasInstance;
    using parent_type::Destroy;

    static void Create() { parent_type::Create(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
