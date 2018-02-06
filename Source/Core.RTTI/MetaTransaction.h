#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/Typedefs.h"

#include "Core/Container/HashSet.h"
#include "Core/Container/Vector.h"
#include "Core/IO/TextWriter.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace RTTI {
FWD_REFPTR(MetaObject);
FWD_REFPTR(MetaTransaction);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ETransactionFlags : u32 {
    Unloaded    = 0,
    Loading     ,
    Loaded      ,
    Unloading   ,
};
//----------------------------------------------------------------------------
class CORE_RTTI_API FMetaTransaction : public FRefCountable {
public:
    explicit FMetaTransaction(const FName& name);
    FMetaTransaction(const FName& name, VECTOR(RTTI, PMetaObject)&& objects);
    virtual ~FMetaTransaction();

    const FName& Name() const { return _name; }
    ETransactionFlags Flags() const { return _flags; }

    size_t NumTopObjects() const { return _topObjects.size(); }
    size_t NumExportedObjects() const { Assert(IsLoaded()); return _exportedObjects.size(); }
    size_t NumLoadedObjects() const { Assert(IsLoaded()); return _loadedObjects.size(); }
    size_t NumImportedTransactions() const { Assert(IsLoaded()); return _importedTransactions.size(); }

    bool IsLoaded() const { return (_flags == ETransactionFlags::Loaded); }
    bool IsLoading() const { return (_flags == ETransactionFlags::Loading); }
    bool IsUnloaded() const { return (_flags == ETransactionFlags::Unloaded); }
    bool IsUnloading() const { return (_flags == ETransactionFlags::Unloading); }

    bool Contains(const FMetaObject* object) const;
    void RegisterObject(FMetaObject* object);
    void UnregisterObject(FMetaObject* object);

    void Load();
    void Unload();
    void Reload();

    TMemoryView<const PMetaObject> TopObjects() const {
        return _topObjects.MakeConstView();
    }

    const VECTOR(RTTI, SCMetaTransaction)& ImportedTransactions() const {
        Assert(IsLoaded());
        return _importedTransactions;
    }

    void reserve(size_t capacity);

    bool DeepEquals(const FMetaTransaction& other) const;

private:
    FName _name;
    ETransactionFlags _flags;
    VECTOR(RTTI, PMetaObject) _topObjects;

    HASHSET(RTTI, SMetaObject) _exportedObjects;
    HASHSET(RTTI, SMetaObject) _loadedObjects;

    VECTOR(RTTI, SCMetaTransaction) _importedTransactions;

    friend class FTransactionLoadContext;
    friend class FTransactionUnloadContext;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, RTTI::ETransactionFlags flags);
CORE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, RTTI::ETransactionFlags flags);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
