#include "stdafx.h"

#include "FileIdentity.h"

#include "Core/Container/RawStorage.h"
#include "Core/IO/FS/FileStat.h"
#include "Core/IO/VirtualFileSystem.h"
#include "Core/Memory/HashFunctions.h"

namespace Core {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FileIdentity::FileIdentity(u64 sizeInBytes, const Timestamp& lastModified, const u128& fingerprint)
:   _sizeInBytes(sizeInBytes)
,   _lastModified(lastModified)
,   _fingerprint(fingerprint) {}
//----------------------------------------------------------------------------
bool FileIdentity::CreateFromFile(FileIdentity *pidentifier, const Filename& sourceFile) {
    Assert(pidentifier);

    FileStat stat;
    if (!VFS_FileStats(&stat, sourceFile))
        return false;

    RAWSTORAGE_THREAD_LOCAL(FileSystem, u8) data;
    if (!VFS_ReadAll(&data, sourceFile))
        return false;

    AssertRelease(data.SizeInBytes() == stat.SizeInBytes);

    const u128 fingerprint = Fingerprint128(data.Pointer(), data.SizeInBytes());
    *pidentifier = FileIdentity(stat.SizeInBytes,  stat.LastModified, fingerprint);

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace Core