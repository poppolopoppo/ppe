#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/MetaClassName.h"

#include "Core/Container/HashMap.h"
#include "Core/Meta/Singleton.h"
#include "Core/Thread/ReadWriteLock.h"

#include <mutex>

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MetaClass;
//----------------------------------------------------------------------------
class MetaClassHashMap {
public:
    MetaClassHashMap();
    ~MetaClassHashMap();

    MetaClassHashMap(const MetaClassHashMap&) = delete;
    MetaClassHashMap& operator =(const MetaClassHashMap&) = delete;

    void Add(const MetaClass *metaClass);
    void Remove(const MetaClass *metaClass);

    const MetaClass *GetIFP(const MetaClassName& name) const;

    void Clear();

private:
    ReadWriteLock _barrier;
    HASHMAP(RTTI, MetaClassName, const MetaClass *) _classes;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MetaClassDatabase : Meta::Singleton<MetaClassHashMap, MetaClassDatabase> {
    typedef Meta::Singleton<MetaClassHashMap, MetaClassDatabase> parent_type;
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
