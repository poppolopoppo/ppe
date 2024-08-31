#pragma once

#include "Serialize.h"

#include "ISerializer.h"
#include "SerializeExceptions.h"

#include "RTTI_fwd.h"

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FFATSerializerException : public FSerializeException {
public:
    explicit FFATSerializerException(const char* what) : FSerializeException(what) {}

    PPE_DEFAULT_EXCEPTION_DESCRIPTION(FBinarySerializerException)
};
//----------------------------------------------------------------------------
class FFATSerializer final : public ISerializer {
    FFATSerializer() = default;
    
public:
    NODISCARD PPE_SERIALIZE_API static FExtname Extname();
    NODISCARD PPE_SERIALIZE_API static USerializer Get();

public: // ISerializer
    virtual void Deserialize(IStreamReader& input, FTransactionLinker* linker) const override final;
    virtual void Serialize(const FTransactionSaver& saver, IStreamWriter* output) const override final;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
