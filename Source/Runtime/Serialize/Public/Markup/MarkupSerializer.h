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

public:
    virtual void Deserialize(
        const FWStringView& fragment,
        IStreamReader& input,
        RTTI::FMetaTransaction* loaded) const override final;

    virtual void Serialize(
        const FWStringView& fragment,
        const RTTI::FMetaTransaction& saved,
        IStreamWriter* output) const override final;

private:
    FString _header;
    bool _minify;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
