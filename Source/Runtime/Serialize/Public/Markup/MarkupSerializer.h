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

    PPE_DEFAULT_EXCEPTION_DESCRIPTION(FJsonSerializerException)
};
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FMarkupSerializer : public ISerializer {
public:
    FMarkupSerializer(bool minify = true);
    virtual ~FMarkupSerializer() override;

public:
    virtual void Deserialize(
        const FWStringView& fragment,
        IStreamReader& input,
        RTTI::FMetaTransaction* loaded) const final;

    virtual void Serialize(
        const FWStringView& fragment,
        const RTTI::FMetaTransaction& saved,
        IStreamWriter* output) const final;

private:
    FString _header;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
