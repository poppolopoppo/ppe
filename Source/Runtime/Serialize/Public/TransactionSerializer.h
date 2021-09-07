#pragma once

#include "Serialize_fwd.h"

#include "IO/Dirpath.h"
#include "IO/Filename.h"
#include "IO/String.h"

#include "MetaObject.h"
#include "RTTI_fwd.h"
#include "RTTI/Macros.h"

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
RTTI_ENUM_HEADER(PPE_SERIALIZE_API, ESerializeFormat);
//----------------------------------------------------------------------------
enum class ETransactionFlags : u32 {
    None                = 0,

    AutoBuild           = 1 << 0,
    AutoMount           = 1 << 1,
    AutoImport          = 1 << 2,

    Compressed          = 1 << 5,
    Merged              = 1 << 6, // #TODO

    Automated           = AutoBuild | AutoMount | AutoImport,
    Default             = Automated | Compressed | Merged
};
ENUM_FLAGS(ETransactionFlags);
RTTI_ENUM_HEADER(PPE_SERIALIZE_API, ETransactionFlags);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FTransactionSources = VECTORINSITU(MetaSerialize, FFilename, 3);
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FTransactionSerializer : public RTTI::FMetaObject {
    RTTI_CLASS_HEADER(PPE_SERIALIZE_API, FTransactionSerializer, RTTI::FMetaObject);
public:
    FTransactionSerializer(
        const RTTI::FName& id,
        const RTTI::FName& namespace_,
        ETransactionFlags flags = ETransactionFlags::Default );
    ~FTransactionSerializer() override;

    FTransactionSerializer(const FTransactionSerializer&) = delete;
    FTransactionSerializer& operator =(const FTransactionSerializer&) = delete;

    const RTTI::FName& Id() const { return _id; }
    const RTTI::FName& Namespace() const { return _namespace; }
    ETransactionFlags Flags() const { return _flags; }

    bool HasAutoBuild() const { return (_flags ^ ETransactionFlags::AutoBuild); }
    bool HasAutoMount() const { return (_flags ^ ETransactionFlags::AutoMount); }
    bool HasAutoImport() const { return (_flags ^ ETransactionFlags::AutoImport); }

    const RTTI::PMetaTransaction& Transaction() const { return _transaction; }

    using FSources = FTransactionSources;

    void BuildTransaction(FSources& sources);
    void SaveTransaction();

    void LoadTransaction();
    void UnloadTransaction();

    void MountToDB();
    void UnmountFromDB();

    static const FDirpath& TransactionPath();
    static FFilename IdToTransaction(const RTTI::FName& name);
    static RTTI::FName TransactionToId(const FFilename& transaction);

protected:
    virtual void FetchSources(FSources& sources) = 0;

public: // RTTI
    explicit FTransactionSerializer(RTTI::FConstructorTag);

    virtual void RTTI_Load(RTTI::ILoadContext& context) override;
    virtual void RTTI_Unload(RTTI::IUnloadContext& context) override;

#ifdef WITH_RTTI_VERIFY_PREDICATES
    virtual void RTTI_VerifyPredicates() const PPE_THROW() override;
#endif

private:
    RTTI::FName _id;
    RTTI::FName _namespace;
    ETransactionFlags _flags;

    RTTI::PMetaTransaction _transaction;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FDirectoryTransaction : public FTransactionSerializer {
    RTTI_CLASS_HEADER(PPE_SERIALIZE_API, FDirectoryTransaction, FTransactionSerializer);
public:
    using FInputPaths = VECTORINSITU(MetaSerialize, FDirpath, 3);

    FDirectoryTransaction(
        const RTTI::FName& id,
        const RTTI::FName& namespace_,
        FWString&& inputPattern,
        FInputPaths&& inputPaths,
        ETransactionFlags flags = ETransactionFlags::Default );
    ~FDirectoryTransaction() override;

    const FWString& InputPattern() const { return _inputPattern; }
    TMemoryView<const FDirpath> InputPaths() const { return _inputPaths; }

protected:
    virtual void FetchSources(FSources& sources) override final;

public: // RTTI
    explicit FDirectoryTransaction(RTTI::FConstructorTag);

#ifdef WITH_RTTI_VERIFY_PREDICATES
    virtual void RTTI_VerifyPredicates() const PPE_THROW() override;
#endif

private:
    FWString _inputPattern;
    FInputPaths _inputPaths;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
