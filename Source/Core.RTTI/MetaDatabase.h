#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/Typedefs.h"

#include "Core/Container/HashMap.h"
#include "Core/Container/Vector.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Meta/Singleton.h"
#include "Core/ThreaD/ReadWriteLock.h"

namespace Core {
namespace RTTI {
FWD_REFPTR(MetaAtom);
FWD_REFPTR(MetaObject);
class FMetaClass;
class FMetaNamespace;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMetaDatabase : Meta::TSingleton<FMetaDatabase> {
private:
    typedef Meta::TSingleton<FMetaDatabase> singleton_type;
    friend class singleton_type;

    FMetaDatabase();
    ~FMetaDatabase();

public:
    using singleton_type::Create;
    using singleton_type::Destroy;
    using singleton_type::HasInstance;
    using singleton_type::Instance;

    template <typename T>
    using TCollector = VECTOR(RTTI, T);

    void RegisterNamespace(const FMetaNamespace* metaNamespace);
    void UnregisterNamespace(const FMetaNamespace* metaNamespace);

    const FMetaClass* FindClass(const FName& name) const;
    const FMetaClass* FindClassIFP(const FName& name) const;
    void AllClasses(TCollector<const FMetaClass*>& instances) const;

    const FMetaNamespace* FindNamespace(const FName& name) const;
    const FMetaNamespace* FindNamespaceIFP(const FName& name) const;
    void AllNamespaces(TCollector<const FMetaNamespace*>& instances) const;

    void RegisterAtom(const FName& name, FMetaAtom* metaAtom, bool allowOverride);
    void UnregisterAtom(const FName& name, FMetaAtom* metaAtom);

    FMetaAtom* FindAtom(const FName& name) const;
    FMetaAtom* FindAtomIFP(const FName& name) const;
    void AllAtoms(TCollector<PMetaAtom>& instances) const;

    void RegisterObject(FMetaObject* object);
    void UnregisterObject(FMetaObject* object);

    FMetaObject* FindObject(const FName& name) const;
    FMetaObject* FindObjectIFP(const FName& name) const;
    void AllObjects(TCollector<PMetaObject>& instances) const;
    void FindObjectByClass(const FMetaClass* metaClass, TCollector<PMetaObject>& instances) const;

    template <typename _MetaClass>
    void FindObjectByClass(TCollector<PMetaObject>& instances) const {
        FindObjectByClass(GetMetaClass<_MetaClass>(), instances);
    }

private:
    FReadWriteLock _barrier;

    HASHMAP(RTTI, FName, const FMetaNamespace*) _namespaces;
    HASHMAP(RTTI, FName, PMetaAtom) _atoms;
};
//----------------------------------------------------------------------------
inline FMetaDatabase& MetaDB() {
    return FMetaDatabase::Instance();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
