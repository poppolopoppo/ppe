#pragma once

#include "Serialize.h"

#include "Container/Vector.h"
#include "Memory/MemoryView.h"
#include "RTTI_fwd.h"

namespace PPE {
class IStreamReader;
class IStreamWriter;
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API ISerializer {
public:
    typedef VECTOR(Transient, RTTI::PMetaObject) FMetaObjectVector;

    ISerializer() {}
    virtual ~ISerializer() {}

    ISerializer(const ISerializer& ) = delete;
    ISerializer& operator =(const ISerializer& ) = delete;

    void Deserialize(RTTI::FMetaTransaction* transaction, IStreamReader* input, const wchar_t *sourceName = nullptr);
    void Deserialize(RTTI::FMetaTransaction* transaction, const TMemoryView<const u8>& rawData, const wchar_t *sourceName = nullptr);

    void Serialize(IStreamWriter* output, const RTTI::FMetaTransaction* transaction);

protected:
    virtual void DeserializeImpl(RTTI::FMetaTransaction* transaction, IStreamReader* input, const wchar_t *sourceName) = 0;
    virtual void SerializeImpl(IStreamWriter* output, const RTTI::FMetaTransaction* transaction) = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
