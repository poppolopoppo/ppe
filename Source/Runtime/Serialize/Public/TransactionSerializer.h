#pragma once

#include "Serialize.h"

#include "Container/HashSet.h"
#include "IO/Dirpath.h"
#include "IO/Filename.h"
#include "IO/String.h"
#include "Memory/UniquePtr.h"
#include "Meta/Optional.h"

#include "RTTI_fwd.h"
#include "RTTI/Macros.h"

#include "MetaObject.h"

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FTransactionInputGlobbing {
    FDirpath Path;
    VECTORINSITU(MetaSerialize, FWString, 3) Patterns;
    void ListFiles(VECTOR(MetaSerialize, FFilename)* output) const;
};
RTTI_STRUCT_DECL(PPE_SERIALIZE_API, FTransactionInputGlobbing);
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FTransactionSerializer : public RTTI::FMetaObject {
    RTTI_CLASS_HEADER(FTransactionSerializer, RTTI::FMetaObject);
public:
    explicit FTransactionSerializer(RTTI::FConstructorTag);

    FTransactionSerializer(
        const RTTI::FName& namespace_,
        std::initializer_list<RTTI::FName> imports,
        std::initializer_list<FFilename> inputFiles,
        std::initializer_list<FTransactionInputGlobbing> globbings,
        bool autoBuild = true,
        bool autoMount = true );

    virtual ~FTransactionSerializer();

    const RTTI::FName& Namespace() const { return _namespace; }

    bool AutoBuild() const { return _autoBuild; }
    bool AutoMount() const { return _autoMount; }

    const VECTOR(MetaSerialize, RTTI::FName)& Imports() const { return _imports; }
    const VECTOR(MetaSerialize, FFilename)& InputFiles() const { return _inputFiles; }
    const VECTOR(MetaSerialize, FTransactionInputGlobbing)& Globbings() const { return _globbings; }

private:
    RTTI::FName _namespace;

    bool _autoBuild;
    bool _autoMount;

    VECTOR(MetaSerialize, RTTI::FName) _imports;
    VECTOR(MetaSerialize, FFilename) _inputFiles;
    VECTOR(MetaSerialize, FTransactionInputGlobbing) _globbings;

private: // transient
    u128 _fingerprint;
    FTimestamp _lastBuilt;
    VECTOR(MetaSerialize, FFilename) _sourceFiles;
    Meta::TOptional<RTTI::FMetaTransaction> _builtTransaction;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
