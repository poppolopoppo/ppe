#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core.Serialize/Exceptions.h"
#include "Core.Serialize/ISerializer.h"

#include "Core.RTTI/RTTI_fwd.h"

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FBinarySerializerException : public FSerializeException {
public:
    FBinarySerializerException(const char* what) : FSerializeException(what) {}
};
//----------------------------------------------------------------------------
class CORE_SERIALIZE_API FBinarySerializer : public ISerializer {
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
} //!namespace Core
