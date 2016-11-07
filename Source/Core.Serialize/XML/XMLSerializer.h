#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core.Serialize/Exceptions.h"
#include "Core.Serialize/ISerializer.h"

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FXMLSerializerException : public FSerializeException {
public:
    FXMLSerializerException(const char* what) : FSerializeException(what) {}
};
//----------------------------------------------------------------------------
class FXMLSerializer : public ISerializer {
public:
    FXMLSerializer();
    virtual ~FXMLSerializer();

    using ISerializer::Deserialize;
    using ISerializer::Serialize;

    virtual void Deserialize(RTTI::FMetaTransaction* transaction, IStreamReader* input, const wchar_t *sourceName = nullptr) override;
    virtual void Serialize(IStreamWriter* output, const RTTI::FMetaTransaction* transaction) override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
