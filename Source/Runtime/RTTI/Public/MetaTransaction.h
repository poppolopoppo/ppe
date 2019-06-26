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
    Mounting        ,
    Mounted         ,
    Unmounting      ,
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
struct PPE_RTTI_API FLinearizedTransaction {
    FLinearizedTransaction();
    ~FLinearizedTransaction();

    using FReferences = VECTORINSITU(MetaTransaction, SMetaObject, 4);

    FReferences ImportedRefs;   // exported, imported from other transaction
    FReferences LoadedRefs;     // exported or not, belong to current transaction
    FReferences ExportedRefs;   // only exported, belong to current transaction

    bool HasImport(const FMetaTransaction& other) const;
    bool HasExports() const { return (not ExportedRefs.empty()); }
    void Reset();
};
//----------------------------------------------------------------------------
class PPE_RTTI_API FMetaTransaction : public FRefCountable {
public:
    explicit FMetaTransaction(
        const FName& name,
        ETransactionFlags flags = ETransactionFlags::Default );
    virtual ~FMetaTransaction();

    const FName& Name() const { return _name; }
    ETransactionFlags Flags() const { return _flags; }
    ETransactionState State() const { return _state; }

    bool KeepDeprecated() const { return (_flags ^ ETransactionFlags::KeepDeprecated); }
    bool KeepTransient() const { return (_flags ^ ETransactionFlags::KeepTransient); }

    bool empty() const { return _topObjects.empty(); }

    bool IsLoaded() const { return (_state == ETransactionState::Loaded); }
    bool IsLoading() const { return (_state == ETransactionState::Loading); }
    bool IsMounting() const { return (_state == ETransactionState::Mounting); }
    bool IsMounted() const { return (_state == ETransactionState::Mounted); }
    bool IsUnmounting() const { return (_state == ETransactionState::Unmounting); }
    bool IsUnloaded() const { return (_state == ETransactionState::Unloaded); }
    bool IsUnloading() const { return (_state == ETransactionState::Unloading); }

    void RegisterObject(FMetaObject* object);
    void UnregisterObject(FMetaObject* object);

    void Load();
    void Unload();
    void Reload();

    void Mount();
    void Unmount();

    void LoadAndMount();
    void UnmountAndUnload();

    const auto& TopObjects() const { return _topObjects; }
    const FLinearizedTransaction& Linearized() const {
        AssertRelease(IsLoaded() || IsMounted());
        return _linearized;
    }

    bool DeepEquals(const FMetaTransaction& other) const;

    void reserve(size_t capacity);

private:
    FName _name;
    ETransactionFlags _flags;
    ETransactionState _state;

    VECTORINSITU(MetaTransaction, PMetaObject, 8) _topObjects; // hold lifetime of top objects only

    FLinearizedTransaction _linearized; // filled upon load, cleared when unloaded

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
PPE_RTTI_API FTextWriter& operator <<(FTextWriter& oss, RTTI::ETransactionState state);
PPE_RTTI_API FWTextWriter& operator <<(FWTextWriter& oss, RTTI::ETransactionState state);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
