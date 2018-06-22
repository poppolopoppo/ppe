#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core.Serialize/Exceptions.h"
#include "Core.Serialize/ISerializer.h"

#include "Core.RTTI/RTTI_fwd.h"

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FJsonSerializerException : public FSerializeException {
public:
    FJsonSerializerException(const char* what) : FSerializeException(what) {}
};
//----------------------------------------------------------------------------
class CORE_SERIALIZE_API FJsonSerializer : public ISerializer {
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
CORE_SERIALIZE_API void RTTI_to_Json(class FJson& dst, const TMemoryView<const RTTI::PMetaObject>& src, const RTTI::FMetaTransaction* outer = nullptr);
//----------------------------------------------------------------------------
CORE_SERIALIZE_API bool Json_to_RTTI(VECTOR(Transient, RTTI::PMetaObject)& dst, const class FJson& src);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
