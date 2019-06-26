#pragma once

#include "Serialize_fwd.h"

#include "RTTI_fwd.h"
#include "IO/Filename.h"

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FTransactionSaver : Meta::FNonCopyableNorMovable {
public:
    FTransactionSaver(
        const RTTI::FMetaTransaction& outer,
        const FFilename& filename );
    ~FTransactionSaver();

    const RTTI::FMetaTransaction& Outer() const { return (*_outer); }
    const FFilename& Filename() const { return _filename; }

    TMemoryView<const RTTI::PMetaObject> TopRefs() const;
    TMemoryView<const RTTI::SMetaObject> ImportedRefs() const;
    TMemoryView<const RTTI::SMetaObject> LoadedRefs() const;
    TMemoryView<const RTTI::SMetaObject> ExportedRefs() const;

private:
    RTTI::SCMetaTransaction _outer;
    const FFilename _filename;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
