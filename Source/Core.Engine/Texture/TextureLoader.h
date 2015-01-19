#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Container/RawStorage.h"
#include "Core/IO/VirtualFileSystem_fwd.h"
#include "Core/Memory/UniquePtr.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
class SurfaceFormat;
class Texture;
class Texture2D;
class TextureCube;
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
    u32 ArraySize;
    u32 SizeInBytes;
    bool IsCubeMap;
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
    Graphics::TextureCube *CreateTextureCube(   Graphics::IDeviceAPIEncapsulator *device,
                                                const MemoryView<const u8>& pixels,
                                                String&& name,
                                                bool useSRGB ) const;

    // choose the right texture class :
    Graphics::Texture *CreateTexture(   Graphics::IDeviceAPIEncapsulator *device,
                                        const MemoryView<const u8>& pixels,
                                        String&& name,
                                        bool useSRGB ) const;

    static Graphics::Texture *Load(Graphics::IDeviceAPIEncapsulator *device, const Filename& filename, bool useSRGB);
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

#include "Core.Engine/Texture/TextureLoader-inl.h"
