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
class FName;
FWD_REFPTR(MetaAtom);
FWD_REFPTR(MetaObject);
//----------------------------------------------------------------------------
class FMetaAtomHashMap {
public:
    FMetaAtomHashMap();
    ~FMetaAtomHashMap();

    FMetaAtomHashMap(const FMetaAtomHashMap&) = delete;
    FMetaAtomHashMap& operator =(const FMetaAtomHashMap&) = delete;

    void Add(const FName& name, FMetaAtom *metaAtom, bool allowOverride);
    void Remove(const FName& name, FMetaAtom *metaAtom);

    void Add(FMetaObject *object);
    void Remove(FMetaObject *object);

    FMetaAtom *GetIFP(const FName& name) const;

    void Clear();

private:
    FReadWriteLock _barrier;
    HASHMAP(RTTI, FName, PMetaAtom) _objects;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMetaAtomDatabase : Meta::TSingleton<FMetaAtomHashMap, FMetaAtomDatabase> {
    typedef Meta::TSingleton<FMetaAtomHashMap, FMetaAtomDatabase> parent_type;
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
