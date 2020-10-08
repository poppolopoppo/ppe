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
class PPE_SERIALIZE_API FFATSerializer final : public ISerializer {
public:
    virtual ~FFATSerializer();

    static FExtname Extname();
    static PSerializer Get();

public: // ISerializer
    virtual void Deserialize(IStreamReader& input, FTransactionLinker* linker) const override final;
    virtual void Serialize(const FTransactionSaver& saver, IStreamWriter* output) const override final;

private:
    friend struct TInSituPtr<ISerializer>;

    FFATSerializer();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
