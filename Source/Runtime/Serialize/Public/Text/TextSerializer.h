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
    virtual ~FTextSerializer();

    static FExtname Extname();
    static PSerializer Get();

public: // ISerializer
    virtual void Deserialize(
        const FWStringView& fragment,
        IStreamReader& input,
        RTTI::FMetaTransaction* loaded) const override final;

    virtual void Serialize(
        const FWStringView& fragment,
        const RTTI::FMetaTransaction& saved,
        IStreamWriter* output) const override final;

private:
    friend struct TInSituPtr<ISerializer>;

    explicit FTextSerializer(bool minify = true);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
