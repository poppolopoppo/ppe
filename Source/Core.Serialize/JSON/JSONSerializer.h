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
class FJSONSerializerException : public FSerializeException {
public:
    FJSONSerializerException(const char* what) : FSerializeException(what) {}
};
//----------------------------------------------------------------------------
class CORE_SERIALIZE_API FJSONSerializer : public ISerializer {
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
CORE_SERIALIZE_API void RTTI_to_JSON(class FJSON& dst, const TMemoryView<const RTTI::PMetaObject>& src);
//----------------------------------------------------------------------------
CORE_SERIALIZE_API bool JSON_to_RTTI(VECTOR_THREAD_LOCAL(Serialize, RTTI::PMetaObject)& dst, const class FJSON& src);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
