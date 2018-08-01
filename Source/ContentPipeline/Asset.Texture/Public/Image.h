#pragma once

#include "Pixmap.h"

#include "Enums.h"
#include "PixelStorage.h"

#include "Allocator/PoolAllocator.h"
#include "Memory/RefPtr.h"

namespace PPE {
class FFilename;
class IStreamWriter;
namespace Pixmap {
class FFloatImage;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(Image);
class PPE_PIXMAP_API FImage : public FRefCountable {
public:
    // raw data stores pixel data in rgba order by default :
    typedef PIXELSTORAGE(Image, u8) raw_data_type;

    FImage();
    FImage( size_t width, size_t height,
            EColorDepth depth = EColorDepth::_8bits,
            EColorMask mask = EColorMask::RGBA,
            EColorSpace space = EColorSpace::sRGB );
    FImage( raw_data_type&& rdata,
            size_t width, size_t height,
            EColorDepth depth = EColorDepth::_8bits,
            EColorMask mask = EColorMask::RGBA,
            EColorSpace space = EColorSpace::sRGB );
    ~FImage();

    size_t Width() const { return _width; }
    size_t Height() const { return _height; }

    EColorDepth Depth() const { return _depth; }
    EColorMask Mask() const { return _mask; }
    EColorSpace Space() const { return _space; }

    size_t PixelSizeInBytes() const;
    size_t TotalSizeInBytes() const { return _data.size(); }

    TMemoryView<u8> MakeView() { return _data.MakeView(); }
    TMemoryView<const u8> MakeConstView() const { return _data.MakeConstView(); }

    TMemoryView<u8> Scanline(size_t row);
    TMemoryView<const u8> Scanline(size_t row) const;

    template <typename T>
    void Fill(T value);

    void Resize_DiscardData(size_t width, size_t height);
    void Resize_DiscardData(size_t width, size_t height, EColorDepth depth, EColorMask mask, EColorSpace space);

    void ConvertFrom(const FFloatImage* src);
    void ConvertTo(FFloatImage* dst) const;

    friend bool Load(FImage* dst, EColorDepth depth, EColorSpace space, const FFilename& filename);
    friend bool Load(FImage* dst, EColorDepth depth, EColorSpace space, const TMemoryView<const u8>& content);

    friend bool Save(const FImage* src, const FFilename& filename);
    friend bool Save(const FImage* src, const FFilename& filename, IStreamWriter* writer);

    SINGLETON_POOL_ALLOCATED_DECL();

    static void Start();
    static void Shutdown();

private:
    size_t _width;
    size_t _height;

    EColorDepth _depth;
    EColorMask _mask;
    EColorSpace _space;

    raw_data_type _data;
};
//----------------------------------------------------------------------------
PPE_PIXMAP_API bool Load(FImage* dst, EColorDepth depth, EColorSpace space, const FFilename& filename);
PPE_PIXMAP_API bool Load(FImage* dst, EColorDepth depth, EColorSpace space, const TMemoryView<const u8>& content);
//----------------------------------------------------------------------------
PPE_PIXMAP_API bool Save(const FImage* src, const FFilename& filename);
PPE_PIXMAP_API bool Save(const FImage* src, const FFilename& filename, IStreamWriter* writer);
//----------------------------------------------------------------------------
template <typename T>
void FImage::Fill(T value) {
    Assert(sizeof(T) == PixelSizeInBytes());
    u8* const pend = pdata + _data.size();
    for (u8* pdata = _data.data(); pdata < pend; pdata += sizeof(T))
        *reinterpret_cast<T*>(pdata) = value;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Pixmap
} //!namespace PPE
