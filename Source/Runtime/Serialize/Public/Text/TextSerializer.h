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
class FTextSerializer : public ISerializer {
public:
    explicit FTextSerializer(bool minify = true);
    virtual ~FTextSerializer();

    using ISerializer::Deserialize;
    using ISerializer::Serialize;

    bool Minify() const { return _minify; }
    void SetMinify(bool value) { _minify = value; }

protected: //ISerializer
    virtual void DeserializeImpl(RTTI::FMetaTransaction* transaction, IStreamReader* input, const wchar_t *sourceName) override;
    virtual void SerializeImpl(IStreamWriter* output, const RTTI::FMetaTransaction* transaction) override;

private:
    bool _minify;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
