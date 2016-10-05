#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Container/RawStorage.h"
#include "Core/IO/VirtualFileSystem_fwd.h"
#include "Core/Memory/UniquePtr.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
class FSurfaceFormat;
class FTexture;
class FTexture2D;
class FTextureCube;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FTextureHeader {
    u32 Width;
    u32 Height;
    u32 Depth;
    u32 LevelCount;
    u32 ArraySize;
    u32 SizeInBytes;
    bool IsCubeMap;
    const Graphics::FSurfaceFormat *Format;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FBasicTextureLoader {
public:
    ~FBasicTextureLoader() {}

    FBasicTextureLoader(const FBasicTextureLoader& ) = delete;
    FBasicTextureLoader& operator =(const FBasicTextureLoader& ) = delete;

    const FTextureHeader& FHeader() const { return _header; }

    template <typename T, typename _Allocator>
    bool Read(TRawStorage<T, _Allocator>& pixels, const FFilename& filename);

    static bool ReadHeader(FTextureHeader *header, const FFilename& filename, IVirtualFileSystemIStream *stream);
    static bool ReadPixels(const TMemoryView<u8>& pixels,  const FTextureHeader *header, const FFilename& filename, IVirtualFileSystemIStream *stream);

protected:
    FBasicTextureLoader();
    FBasicTextureLoader(const FTextureHeader& header);

    FTextureHeader _header;
};
//----------------------------------------------------------------------------
class FTexture2DLoader : public FBasicTextureLoader {
public:
    FTexture2DLoader() {}
    ~FTexture2DLoader() {}

    FTexture2DLoader(const FTextureHeader& header)
    :   FBasicTextureLoader(header) {}

    Graphics::FTexture2D *CreateTexture2D(   Graphics::IDeviceAPIEncapsulator *device,
                                            const TMemoryView<const u8>& pixels,
                                            FString&& name,
                                            bool useSRGB ) const;
    Graphics::FTextureCube *CreateTextureCube(   Graphics::IDeviceAPIEncapsulator *device,
                                                const TMemoryView<const u8>& pixels,
                                                FString&& name,
                                                bool useSRGB ) const;

    // choose the right texture class :
    Graphics::FTexture *CreateTexture(   Graphics::IDeviceAPIEncapsulator *device,
                                        const TMemoryView<const u8>& pixels,
                                        FString&& name,
                                        bool useSRGB ) const;

    static Graphics::FTexture *Load(Graphics::IDeviceAPIEncapsulator *device, const FFilename& filename, bool useSRGB);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace DDS {
//----------------------------------------------------------------------------
bool ReadTextureHeader(FTextureHeader& header, IVirtualFileSystemIStream *stream);
//----------------------------------------------------------------------------
bool ReadTextureData(const FTextureHeader& header, const TMemoryView<u8>& pixels, IVirtualFileSystemIStream *stream);
//----------------------------------------------------------------------------
} //!namespace DDS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core

#include "Core.Engine/Texture/TextureLoader-inl.h"
