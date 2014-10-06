#pragma once

#include "Engine.h"

#include "Core/RawStorage.h"
#include "Core/UniquePtr.h"
#include "Core/VirtualFileSystem_fwd.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
class SurfaceFormat;
class Texture2D;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct TextureHeader {
    u32 Width;
    u32 Height;
    u32 Depth;
    u32 LevelCount;
    u32 FaceCount;
    u32 ArraySize;
    u32 SizeInBytes;
    const Graphics::SurfaceFormat *Format;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class BasicTextureLoader {
public:
    ~BasicTextureLoader() {}

    BasicTextureLoader(const BasicTextureLoader& ) = delete;
    BasicTextureLoader& operator =(const BasicTextureLoader& ) = delete;

    const TextureHeader& Header() const { return _header; }

    template <typename T, typename _Allocator>
    bool Read(RawStorage<T, _Allocator>& pixels, const Filename& filename);

    static bool ReadHeader(TextureHeader *header, const Filename& filename, IVirtualFileSystemIStream *stream);
    static bool ReadPixels(const MemoryView<u8>& pixels,  const TextureHeader *header, const Filename& filename, IVirtualFileSystemIStream *stream);

protected:
    BasicTextureLoader();
    BasicTextureLoader(const TextureHeader& header);

    TextureHeader _header;
};
//----------------------------------------------------------------------------
class Texture2DLoader : public BasicTextureLoader {
public:
    Texture2DLoader() {}
    ~Texture2DLoader() {}

    Texture2DLoader(const TextureHeader& header)
    :   BasicTextureLoader(header) {}

    Graphics::Texture2D *CreateTexture2D(   Graphics::IDeviceAPIEncapsulator *device,
                                            const MemoryView<const u8>& pixels,
                                            String&& name,
                                            bool useSRGB ) const;

    static Graphics::Texture2D *Load(Graphics::IDeviceAPIEncapsulator *device, const Filename& filename, bool useSRGB);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace DDS {
//----------------------------------------------------------------------------
bool ReadTextureHeader(TextureHeader& header, IVirtualFileSystemIStream *stream);
//----------------------------------------------------------------------------
bool ReadTextureData(const TextureHeader& header, const MemoryView<u8>& pixels, IVirtualFileSystemIStream *stream);
//----------------------------------------------------------------------------
} //!namespace DDS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core

#include "TextureLoader-inl.h"
