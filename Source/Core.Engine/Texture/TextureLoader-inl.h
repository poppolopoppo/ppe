#pragma once

#include "Core.Engine/Texture/TextureLoader.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool BasicTextureLoader::Read(RawStorage<T, _Allocator>& pixels, const Filename& filename) {
    Assert(filename.Extname().Equals(L".dds")); // only format supported for now

    UniquePtr<IVirtualFileSystemIStream> iss = VFS_OpenReadable(filename, AccessPolicy::Binary);
    Assert(iss);

    if (!ReadHeader(&_header, filename, iss.get()) )
        return false;

    Assert(0 == (_header.SizeInBytes % sizeof(T)) );
    const size_t sizeInPixels = _header.SizeInBytes / sizeof(T);
    pixels.Resize_DiscardData(sizeInPixels);

    if (!ReadPixels(pixels.MakeView().Cast<u8>(), &_header, filename, iss.get()) )
        return false;

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
