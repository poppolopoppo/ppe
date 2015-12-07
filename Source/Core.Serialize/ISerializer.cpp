#include "stdafx.h"

#include "ISerializer.h"

#include "Core/Container/RawStorage.h"
#include "Core/IO/FS/Filename.h"
#include "Core/IO/StreamProvider.h"

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ISerializer::Deserialize(VECTOR(Transaction, RTTI::PMetaObject)& objects, IStreamReader* iss, const wchar_t *sourceName/* = nullptr */) {
    Assert(iss);

    RAWSTORAGE_THREAD_LOCAL(Serializer, u8) storage;
    iss->ReadAll(storage);

    Deserialize(objects, storage.MakeConstView(), sourceName);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
