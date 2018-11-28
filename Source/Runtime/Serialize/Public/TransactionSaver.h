#pragma once

#include "Serialize_fwd.h"

#include "Container/Vector.h"
#include "IO/Filename.h"
#include "MetaTransaction.h"
#include "RTTI_fwd.h"
#include "RTTI/Typedefs.h"

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FTransactionSaver : Meta::FNonCopyableNorMovable {
public:
    FTransactionSaver(
        const RTTI::FName& namespace_,
        const FFilename& filename );
    FTransactionSaver(
        const RTTI::FMetaTransaction& transaction,
        const FFilename& filename );
    ~FTransactionSaver();

    const RTTI::FName& Namespace() const { return _namespace; }
    const FFilename& Filename() const { return _filename; }

    const RTTI::FLinearizedTransaction& Objects() const { return _objects; }

    void Append(const RTTI::FMetaObjectRef& obj); // don't hold lifetime !
    void Append(const RTTI::FMetaTransaction& transaction); // linearize the transaction

private:
    const RTTI::FName _namespace;
    const FFilename _filename;

    RTTI::FLinearizedTransaction _objects;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
