#pragma once

#include "Serialize.h"

#include "Exceptions.h"
#include "ISerializer.h"

#include "IO/String.h"

// TODO
#error "TODO"

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FJsonSerializerException : public FSerializeException {
public:
    FJsonSerializerException(const char* what) : FSerializeException(what) {}
};
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FMarkupSerializer : public ISerializer {
public:
    FMarkupSerializer(bool minify = true);
    virtual ~FMarkupSerializer();

    bool Minify() const { return _minify; }
    void SetMinify(bool value) { _minify = value; }

    using ISerializer::Deserialize;
    using ISerializer::Serialize;

    virtual void Deserialize(RTTI::FMetaTransaction* transaction, IStreamReader* input, const wchar_t *sourceName = nullptr) override;
    virtual void Serialize(IStreamWriter* output, const RTTI::FMetaTransaction* transaction) override;

private:
    FString _header;
    bool _minify;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
