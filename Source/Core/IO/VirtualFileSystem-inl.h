#pragma once

#include "Core/IO/VirtualFileSystem.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void VirtualFileSystemRoot::ReadAll_DiscardData(const Filename& filename, RawStorage<T, _Allocator>& storage, AccessPolicy::Mode policy/* = AccessPolicy::None */) {
    const UniquePtr<IVirtualFileSystemIStream> istream = OpenReadable(filename, static_cast<AccessPolicy::Mode>(policy|AccessPolicy::Ate|AccessPolicy::Binary));

    const std::streamsize sizeInBytes = istream->TellI();
    Assert((sizeInBytes % sizeof(T)) == 0);
    storage.Resize_DiscardData(checked_cast<size_t>(sizeInBytes / sizeof(T)));

    istream->SeekI(0);
    istream->Read(storage.Pointer(), sizeInBytes);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
