#include "stdafx.h"

#include "ISerializer.h"

#include "Core.RTTI/MetaTransaction.h"

#include "Core/Container/RawStorage.h"
#include "Core/IO/FS/Filename.h"
#include "Core/IO/StreamProvider.h"
#include "Core/Memory/MemoryProvider.h"

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ISerializer::Deserialize(RTTI::FMetaTransaction* transaction, IStreamReader* input, const wchar_t *sourceName/* = nullptr */) {
    Assert(transaction);
    Assert(not transaction->IsLoaded());
    Assert(input);

    if (nullptr == sourceName)
        sourceName = L"@memory";

    DeserializeImpl(transaction, input, sourceName);
}
//----------------------------------------------------------------------------
void ISerializer::Deserialize(RTTI::FMetaTransaction* transaction, const TMemoryView<const u8>& rawData, const wchar_t *sourceName/* = nullptr */) {
    FMemoryViewReader reader(rawData);
    Deserialize(transaction, &reader, sourceName);
}
//----------------------------------------------------------------------------
void ISerializer::Serialize(IStreamWriter* output, const RTTI::FMetaTransaction* transaction) {
    Assert(output);
    Assert(transaction);
    Assert(transaction->IsLoaded());

    SerializeImpl(output, transaction);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
