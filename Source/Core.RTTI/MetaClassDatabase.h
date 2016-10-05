#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/MetaType.h"

#include "Core/Container/HashMap.h"
#include "Core/Meta/Singleton.h"
#include "Core/Thread/ReadWriteLock.h"

#include <mutex>

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMetaClass;
//----------------------------------------------------------------------------
class FMetaClassHashMap {
public:
    FMetaClassHashMap();
    ~FMetaClassHashMap();

    FMetaClassHashMap(const FMetaClassHashMap&) = delete;
    FMetaClassHashMap& operator =(const FMetaClassHashMap&) = delete;

    void Add(const FMetaClass *metaClass);
    void Remove(const FMetaClass *metaClass);

    const FMetaClass *GetIFP(const FName& name) const;

    void Clear();

private:
    FReadWriteLock _barrier;
    HASHMAP(RTTI, FName, const FMetaClass *) _classes;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMetaClassDatabase : Meta::TSingleton<FMetaClassHashMap, FMetaClassDatabase> {
    typedef Meta::TSingleton<FMetaClassHashMap, FMetaClassDatabase> parent_type;
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
