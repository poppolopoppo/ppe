#include "stdafx.h"

#include "ISerializer.h"

#include "Binary/BinarySerializer.h"
#include "Json/JsonSerializer.h"
#include "Text/TextSerializer.h"

#include "IO/Extname.h"
#include "Memory/MemoryProvider.h"

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ISerializer::Deserialize(
    const ISerializer& serializer,
    const FWStringView& fragment,
    const TMemoryView<const u8>& rawData,
    RTTI::FMetaTransaction* loaded) {
    Assert(not rawData.empty());

    FMemoryViewReader reader(rawData);
    serializer.Deserialize(fragment, reader, loaded);
}
//----------------------------------------------------------------------------
PSerializer ISerializer::FromExtname(const FExtname& ext) {
    Assert(not ext.empty());

    if (ext == FBinarySerializer::Extname())
        return FBinarySerializer::Get();
    else if (ext == FJsonSerializer::Extname())
        return FJsonSerializer::Get();
    else if (ext == FTextSerializer::Extname())
        return FTextSerializer::Get();

    return PSerializer(); // unknown extension
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
