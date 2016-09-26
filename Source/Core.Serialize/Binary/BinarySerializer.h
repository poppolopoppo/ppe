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
class BinarySerializerException : public SerializeException {
public:
    BinarySerializerException(const char* what) : SerializeException(what) {}
};
//----------------------------------------------------------------------------
class BinarySerializer : public ISerializer {
public:
    BinarySerializer();
    virtual ~BinarySerializer();

    using ISerializer::Deserialize;
    using ISerializer::Serialize;

    virtual void Deserialize(RTTI::MetaTransaction* transaction, IStreamReader* input, const wchar_t *sourceName = nullptr) override;
    virtual void Serialize(IStreamWriter* output, const RTTI::MetaTransaction* transaction) override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
