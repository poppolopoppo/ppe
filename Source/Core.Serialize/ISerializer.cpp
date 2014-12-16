#include "stdafx.h"

#include "ISerializer.h"

#include "Core/Container/RawStorage.h"
#include "Core/IO/VFS/VirtualFileSystemStream.h"

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ISerializer::Deserialize(VECTOR(Transaction, RTTI::PMetaAtom)& atoms, IVirtualFileSystemIStream *iss) {
    Assert(iss);

    RAWSTORAGE(Serializer, u8) storage;
    iss->ReadAll(storage);

    char sourceName[MAX_PATH];
    iss->SourceFilename().ToCStr(sourceName);

    Deserialize(atoms, storage, sourceName);
}
//----------------------------------------------------------------------------
void ISerializer::Serialize(IVirtualFileSystemOStream *oss, const MemoryView<const RTTI::PCMetaAtom>& atoms) {
    Assert(oss);

    RAWSTORAGE(Serializer, u8) storage;
    Serialize(storage, atoms);

    oss->Write(storage.Pointer(), storage.SizeInBytes());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
