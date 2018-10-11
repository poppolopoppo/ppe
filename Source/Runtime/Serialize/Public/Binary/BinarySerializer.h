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
    virtual ~FBinarySerializer();

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

    FBinarySerializer();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
