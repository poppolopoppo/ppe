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
class FBinarySerializerException : public FSerializeException {
public:
    FBinarySerializerException(const char* what) : FSerializeException(what) {}
};
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FBinarySerializer : public ISerializer {
public:
    FBinarySerializer();
    virtual ~FBinarySerializer();

    using ISerializer::Deserialize;
    using ISerializer::Serialize;

protected: //ISerializer
    virtual void DeserializeImpl(RTTI::FMetaTransaction* transaction, IStreamReader* input, const wchar_t *sourceName) override;
    virtual void SerializeImpl(IStreamWriter* output, const RTTI::FMetaTransaction* transaction) override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
