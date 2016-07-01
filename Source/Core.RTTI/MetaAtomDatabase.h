#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core/Container/HashMap.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Meta/Singleton.h"
#include "Core/Thread/ReadWriteLock.h"

#include <mutex>

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Name;
FWD_REFPTR(MetaAtom);
FWD_REFPTR(MetaObject);
//----------------------------------------------------------------------------
class MetaAtomHashMap {
public:
    MetaAtomHashMap();
    ~MetaAtomHashMap();

    MetaAtomHashMap(const MetaAtomHashMap&) = delete;
    MetaAtomHashMap& operator =(const MetaAtomHashMap&) = delete;

    void Add(const RTTI::Name& name, MetaAtom *metaAtom, bool allowOverride);
    void Remove(const RTTI::Name& name, MetaAtom *metaAtom);

    void Add(MetaObject *object);
    void Remove(MetaObject *object);

    MetaAtom *GetIFP(const RTTI::Name& name) const;

    void Clear();

private:
    ReadWriteLock _barrier;
    HASHMAP(RTTI, RTTI::Name, PMetaAtom) _objects;
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
