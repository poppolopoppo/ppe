#pragma once

#include "Serialize.h"

#include "ISerializer.h"
#include "SerializeExceptions.h"

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FTextSerializerException : public FSerializeException {
public:
    FTextSerializerException(const char* what) : FSerializeException(what) {}
};
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FTextSerializer : public ISerializer {
public:
    virtual ~FTextSerializer();

    static FExtname Extname();
    static PSerializer Get();

public: // ISerializer
    virtual void Deserialize(IStreamReader& input, FTransactionLinker* linker) const override final;
    virtual void Serialize(const FTransactionSaver& saver, IStreamWriter* output) const override final;

private:
    friend struct TInSituPtr<ISerializer>;

    explicit FTextSerializer(bool minify = true);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
