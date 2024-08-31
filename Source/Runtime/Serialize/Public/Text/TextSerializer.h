#pragma once

#include "Serialize.h"

#include "ISerializer.h"
#include "SerializeExceptions.h"

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FTextSerializerException : public FSerializeException {
public:
    FTextSerializerException(const char* what) : FSerializeException(what) {}
};
//----------------------------------------------------------------------------
class FTextSerializer final : public ISerializer {
    explicit FTextSerializer(bool minify = true);
    
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
