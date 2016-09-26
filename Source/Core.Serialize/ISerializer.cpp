#include "stdafx.h"

#include "ISerializer.h"

#include "Core/Container/RawStorage.h"
#include "Core/IO/FS/Filename.h"
#include "Core/IO/StreamProvider.h"
#include "Core/Memory/MemoryProvider.h"

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ISerializer::Deserialize(RTTI::MetaTransaction* transaction, const MemoryView<const u8>& rawData, const wchar_t *sourceName/* = nullptr */) {
    MemoryViewReader reader(rawData);
    Deserialize(transaction, &reader, sourceName);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
