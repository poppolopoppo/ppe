#pragma once

#include "RTTI_fwd.h"

#include "RTTI/Typedefs.h"

#include "Container/HashSet.h"
#include "Container/Vector.h"
#include "IO/TextWriter.h"
#include "Memory/RefPtr.h"
#include "Meta/PointerWFlags.h"

namespace PPE {
namespace RTTI {
FWD_REFPTR(MetaObject);
FWD_REFPTR(MetaTransaction);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ETransactionState : u32 {
    Unloaded        = 0,
    Loading         ,
    Loaded          ,
    Unloading       ,
};
//----------------------------------------------------------------------------
enum class ETransactionFlags : u32 {
    Default         = 0,
    KeepDeprecated  = 1<<0,
    KeepTransient   = 1<<1,
};
ENUM_FLAGS(ETransactionFlags);
//----------------------------------------------------------------------------
struct PPE_RTTI_API FMetaObjectRef : Meta::TPointerWFlags<FMetaObject> {
    bool IsExport() const { return Flag0(); }
    bool IsImport() const { return Flag1(); }
    static FMetaObjectRef Make(FMetaObject* obj, bool intern);
};
using FLinearizedTransaction = VECTORINSITU(MetaTransaction, FMetaObjectRef, 8);
//----------------------------------------------------------------------------
class PPE_RTTI_API FMetaTransaction : public FRefCountable {
public:
    explicit FMetaTransaction(
        const FName& name,
        ETransactionFlags flags = ETransactionFlags::Default );
    FMetaTransaction(const FName& name, VECTOR(MetaTransaction, PMetaObject)&& objects);
    virtual ~FMetaTransaction();

    const FName& Name() const { return _name; }
    ETransactionFlags Flags() const { return _flags; }
    ETransactionState State() const { return _state; }

    size_t NumTopObjects() const { return _topObjects.size(); }
    size_t NumExportedObjects() const { Assert_NoAssume(IsLoaded()); return _exportedObjects.size(); }
    size_t NumLoadedObjects() const { Assert_NoAssume(IsLoaded()); return _loadedObjects.size(); }
    size_t NumImportedTransactions() const { Assert_NoAssume(IsLoaded()); return _importedTransactions.size(); }

    bool KeepDeprecated() const { return (_flags ^ ETransactionFlags::KeepDeprecated); }
    bool KeepTransient() const { return (_flags ^ ETransactionFlags::KeepTransient); }

    bool IsLoaded() const { return (_state == ETransactionState::Loaded); }
    bool IsLoading() const { return (_state == ETransactionState::Loading); }
    bool IsUnloaded() const { return (_state == ETransactionState::Unloaded); }
    bool IsUnloading() const { return (_state == ETransactionState::Unloading); }

    bool Contains(const FMetaObject* object) const;
    void RegisterObject(FMetaObject* object);
    void UnregisterObject(FMetaObject* object);

    void Load();
    void Unload();
    void Reload();

    TMemoryView<const PMetaObject> TopObjects() const {
        return _topObjects.MakeConstView();
    }

    const auto& ImportedTransactions() const {
        Assert_NoAssume(IsLoaded());
        return _importedTransactions;
    }

    void reserve(size_t capacity);

    bool DeepEquals(const FMetaTransaction& other) const;
    void Linearize(FLinearizedTransaction* linearized) const;

    struct PPE_RTTI_API FLoadingScope {
        FMetaTransaction& Transaction;
        explicit FLoadingScope(FMetaTransaction& transaction);
        ~FLoadingScope();
    };

private:
    FName _name;
    ETransactionFlags _flags;
    ETransactionState _state;
    VECTOR(MetaTransaction, PMetaObject) _topObjects; // hold lifetime of top objects only

    HASHSET(MetaTransaction, SMetaObject) _exportedObjects;
    HASHSET(MetaTransaction, SMetaObject) _loadedObjects;

    VECTORINSITU(MetaTransaction, SCMetaTransaction, 3) _importedTransactions;

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
PPE_ASSUME_TYPE_AS_POD(RTTI::FMetaObjectRef);
//----------------------------------------------------------------------------
PPE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, RTTI::ETransactionFlags flags);
PPE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, RTTI::ETransactionFlags flags);
//----------------------------------------------------------------------------
PPE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, RTTI::ETransactionState state);
PPE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, RTTI::ETransactionState state);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
