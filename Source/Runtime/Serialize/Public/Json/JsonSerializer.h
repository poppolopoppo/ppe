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
class FJsonSerializerException : public FSerializeException {
public:
    FJsonSerializerException(const char* what) : FSerializeException(what) {}
};
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FJsonSerializer : public ISerializer {
public:
    explicit FJsonSerializer(bool minify = true);
    virtual ~FJsonSerializer();

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
PPE_SERIALIZE_API void RTTI_to_Json(class FJson& dst, const TMemoryView<const RTTI::PMetaObject>& src, const RTTI::FMetaTransaction* outer = nullptr);
//----------------------------------------------------------------------------
PPE_SERIALIZE_API bool Json_to_RTTI(VECTOR(Transient, RTTI::PMetaObject)& dst, const class FJson& src);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
