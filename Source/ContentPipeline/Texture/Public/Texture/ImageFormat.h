#pragma once

#include "Texture_fwd.h"

#include "IO/FileSystem_fwd.h"
#include "IO/Stream_fwd.h"
#include "Memory/InSituPtr.h"
#include "Meta/Optional.h"

namespace PPE {
class FUniqueBuffer;
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FTextureImporterResult = Meta::TOptional<FUniqueBuffer>;
//----------------------------------------------------------------------------
// Should be state-less
class IImageFormat : Meta::FNonCopyableNorMovable {
public:
    virtual ~IImageFormat() = default;

    NODISCARD virtual EImageFormat ImageFormat() const NOEXCEPT = 0;

    NODISCARD virtual bool SupportsImageView(RHI::EImageView view) const NOEXCEPT = 0;
    NODISCARD virtual bool SupportsTextureFormat(ETextureSourceFormat fmt) const NOEXCEPT = 0;
    NODISCARD virtual bool SupportsTextureSource(const FTextureSourceProperties& properties) const NOEXCEPT = 0;

    NODISCARD virtual bool ExportTexture2D(IStreamWriter* output, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const = 0;
    NODISCARD virtual bool ExportTexture2DArray(IStreamWriter* output, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const = 0;
    NODISCARD virtual bool ExportTexture3D(IStreamWriter* output, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const = 0;
    NODISCARD virtual bool ExportTextureCube(IStreamWriter* output, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const = 0;
    NODISCARD virtual bool ExportTextureCubeArray(IStreamWriter* output, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const = 0;
    NODISCARD virtual bool ExportTextureCubeLongLat(IStreamWriter* output, const FTextureSourceProperties& properties, const FRawMemoryConst& bulk) const = 0;

    NODISCARD virtual FTextureImporterResult ImportTexture2D(FTextureSourceProperties* outProperties, const FRawMemoryConst& memory) const = 0;
    NODISCARD virtual FTextureImporterResult ImportTexture2DArray(FTextureSourceProperties* outProperties, const FRawMemoryConst& memory) const = 0;
    NODISCARD virtual FTextureImporterResult ImportTexture3D(FTextureSourceProperties* outProperties, const FRawMemoryConst& memory) const = 0;
    NODISCARD virtual FTextureImporterResult ImportTextureCube(FTextureSourceProperties* outProperties, const FRawMemoryConst& memory) const = 0;
    NODISCARD virtual FTextureImporterResult ImportTextureCubeArray(FTextureSourceProperties* outProperties, const FRawMemoryConst& memory) const = 0;
    NODISCARD virtual FTextureImporterResult ImportTextureCubeLongLat(FTextureSourceProperties* outProperties, const FRawMemoryConst& memory) const = 0;

    NODISCARD virtual FTextureImporterResult ImportTexture2D(FTextureSourceProperties* outProperties, IStreamReader& input) const = 0;
    NODISCARD virtual FTextureImporterResult ImportTexture2DArray(FTextureSourceProperties* outProperties, IStreamReader& input) const = 0;
    NODISCARD virtual FTextureImporterResult ImportTexture3D(FTextureSourceProperties* outProperties, IStreamReader& input) const = 0;
    NODISCARD virtual FTextureImporterResult ImportTextureCube(FTextureSourceProperties* outProperties, IStreamReader& input) const = 0;
    NODISCARD virtual FTextureImporterResult ImportTextureCubeArray(FTextureSourceProperties* outProperties, IStreamReader& input) const = 0;
    NODISCARD virtual FTextureImporterResult ImportTextureCubeLongLat(FTextureSourceProperties* outProperties, IStreamReader& input) const = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
