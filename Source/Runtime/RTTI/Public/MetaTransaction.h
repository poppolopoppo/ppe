#pragma once

#include "RTTI_fwd.h"

#include "RTTI/Typedefs.h"

#include "Container/HashSet.h"
#include "Container/Vector.h"
#include "IO/TextWriter.h"
#include "Memory/RefPtr.h"

namespace PPE {
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
using FLinearizedTransaction = VECTORINSITU(MetaTransaction, SMetaObject, 8);
//----------------------------------------------------------------------------
class PPE_RTTI_API FMetaTransaction : public FRefCountable {
public:
    explicit FMetaTransaction(const FName& name);
    FMetaTransaction(const FName& name, VECTOR(MetaTransaction, PMetaObject)&& objects);
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

    const VECTOR(MetaTransaction, SCMetaTransaction)& ImportedTransactions() const {
        Assert(IsLoaded());
        return _importedTransactions;
    }

    void reserve(size_t capacity);

    bool DeepEquals(const FMetaTransaction& other) const;

    void Linearize(FLinearizedTransaction* linearized) const;

    struct FLoadingScope {
        FMetaTransaction& Transaction;
        explicit FLoadingScope(FMetaTransaction& transaction);
        ~FLoadingScope();
    };

private:
    FName _name;
    ETransactionFlags _flags;
    VECTOR(MetaTransaction, PMetaObject) _topObjects; // hold lifetime of top objects only

    HASHSET(MetaTransaction, SMetaObject) _exportedObjects;
    HASHSET(MetaTransaction, SMetaObject) _loadedObjects;

    VECTOR(MetaTransaction, SCMetaTransaction) _importedTransactions;

    friend class FTransactionLoadContext;
    friend class FTransactionUnloadContext;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, RTTI::ETransactionFlags flags);
PPE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, RTTI::ETransactionFlags flags);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
