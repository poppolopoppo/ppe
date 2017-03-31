#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core.Serialize/Exceptions.h"
#include "Core.Serialize/ISerializer.h"

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FJSONSerializerException : public FSerializeException {
public:
    FJSONSerializerException(const char* what) : FSerializeException(what) {}
};
//----------------------------------------------------------------------------
class FJSONSerializer : public ISerializer {
public:
    FJSONSerializer(bool minify = true);
    virtual ~FJSONSerializer();

    using ISerializer::Deserialize;
    using ISerializer::Serialize;

    virtual void Deserialize(RTTI::FMetaTransaction* transaction, IStreamReader* input, const wchar_t *sourceName = nullptr) override;
    virtual void Serialize(IStreamWriter* output, const RTTI::FMetaTransaction* transaction) override;

private:
    bool _minify;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
