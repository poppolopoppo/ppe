#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core/Container/Vector.h"
#include "Core/Memory/MemoryView.h"
#include "Core.RTTI/RTTI_fwd.h"

namespace Core {
class IStreamReader;
class IStreamWriter;
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ISerializer {
public:
    typedef VECTOR(RTTI, RTTI::PMetaObject) MetaObjectVector;

    ISerializer() {}
    virtual ~ISerializer() {}

    ISerializer(const ISerializer& ) = delete;
    ISerializer& operator =(const ISerializer& ) = delete;

    virtual void Deserialize(RTTI::MetaTransaction* transaction, IStreamReader* input, const wchar_t *sourceName = nullptr) = 0;
    virtual void Serialize(IStreamWriter* output, const RTTI::MetaTransaction* transaction) = 0;

    void Deserialize(RTTI::MetaTransaction* transaction, const MemoryView<const u8>& rawData, const wchar_t *sourceName = nullptr);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
