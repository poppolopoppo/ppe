#include "stdafx.h"

#include "ISerializer.h"

#include "MetaTransaction.h"

#include "Container/RawStorage.h"
#include "IO/Filename.h"
#include "IO/StreamProvider.h"
#include "Memory/MemoryProvider.h"

namespace PPE {
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
} //!namespace PPE
