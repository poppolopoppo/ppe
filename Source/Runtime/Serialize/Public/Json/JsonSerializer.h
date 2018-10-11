#pragma once

#include "Serialize.h"

#include "ISerializer.h"

#include "Container/Vector.h"
#include "SerializeExceptions.h"
#include "RTTI_fwd.h"

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
class PPE_SERIALIZE_API FJsonSerializer : public ISerializer {
public:
    virtual ~FJsonSerializer();

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

    explicit FJsonSerializer(bool minify = true);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_SERIALIZE_API void RTTI_to_Json(class FJson& dst, const TMemoryView<const RTTI::PMetaObject>& src, const RTTI::FMetaTransaction* outer = nullptr);
//----------------------------------------------------------------------------
PPE_SERIALIZE_API bool Json_to_RTTI(VECTOR(Transient, RTTI::PMetaObject)& dst, const class FJson& src);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
