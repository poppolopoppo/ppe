#pragma once

#include "RHI_fwd.h"

#include "RHI/BufferView.h"
#include "RHI/SamplerEnums.h"

#include "Maths/ScalarVectorHelpers.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FImageView {
    using value_type = FBufferView::value_type;
    using subpart_type = FBufferView::subpart_type;
    using iterator = FBufferView::FIterator;

    FImageView() = default;

    PPE_RHI_API FImageView(
        const TMemoryView<const subpart_type>& parts,
        const uint3& dimensions,
        EPixelFormat format,
        EImageAspect aspect,
        bool bTilable = false ) NOEXCEPT;
    PPE_RHI_API FImageView(
        const TMemoryView<const subpart_type>& parts,
        const uint3& dimensions,
        EPixelFormat format,
        EImageAspect aspect,
        EAddressMode addressMode) NOEXCEPT;

    PPE_RHI_API FImageView(
        const TMemoryView<const subpart_type>& parts,
        const uint3& dimensions,
        size_t rowPitch,
        size_t slicePitch,
        EPixelFormat format,
        EImageAspect aspect) NOEXCEPT;

    bool empty() const { return _parts.empty(); }
    size_t size() const { return _parts.size(); }
    const value_type* data() const { return _parts.data(); }

    iterator begin() const { return _parts.begin(); }
    iterator end() const { return _parts.end(); }

    TMemoryView<const subpart_type> Parts() const { return _parts.Parts(); }
    const uint3& Dimensions() const { return _dimensions; }
    size_t RowPitch() const { return _rowPitch; }
    size_t SlicePitch() const { return _slicePitch; }
    size_t BitsPerPixel() const { return _bitsPerPixel; }
    EPixelFormat Format() const { return _format; }

    // implementation guaranties that single row completely placed to solid memory block.
    inline subpart_type Row(u32 y, u32 z = 0) const NOEXCEPT;

    // implementation doesn't guarantee that a whole slice will fit in a single memory block,
    // so check result size with 'SliceSize()'.
    inline subpart_type Slice(u32 z) const NOEXCEPT;
    inline subpart_type At(const uint3& safePoint) const NOEXCEPT;
    inline subpart_type Pixel(const int3& point) const NOEXCEPT;
    inline subpart_type Texel(const float3& uvw) const NOEXCEPT;

    inline float3 PointToTexCoords(const int3& point) const NOEXCEPT;
    inline int3 TexCoordsToPoint(const float3& uvw) const NOEXCEPT;

    PPE_RHI_API void Load(FRgba32f* pcol, const uint3& safePoint) const NOEXCEPT;
    PPE_RHI_API void Load(FRgba32u* pcol, const uint3& safePoint) const NOEXCEPT;
    PPE_RHI_API void Load(FRgba32i* pcol, const uint3& safePoint) const NOEXCEPT;

    PPE_RHI_API void Load(FRgba32f* pcol, const int3& point) const NOEXCEPT;
    PPE_RHI_API void Load(FRgba32u* pcol, const int3& point) const NOEXCEPT;
    PPE_RHI_API void Load(FRgba32i* pcol, const int3& point) const NOEXCEPT;

    PPE_RHI_API void Load(FRgba32f* pcol, const float3& uvw) const NOEXCEPT;
    PPE_RHI_API void Load(FRgba32u* pcol, const float3& uvw) const NOEXCEPT;
    PPE_RHI_API void Load(FRgba32i* pcol, const float3& uvw) const NOEXCEPT;

    PPE_RHI_API void Load(FRgba32f* pcol, const float3& uvw, ETextureFilter filter) const NOEXCEPT;

    PPE_RHI_API void Store(const uint3& safePoint, const FRgba32f& col) const NOEXCEPT;
    PPE_RHI_API void Store(const uint3& safePoint, const FRgba32u& col) const NOEXCEPT;
    PPE_RHI_API void Store(const uint3& safePoint, const FRgba32i& col) const NOEXCEPT;

    PPE_RHI_API void Store(const int3& point, const FRgba32f& col) const NOEXCEPT;
    PPE_RHI_API void Store(const int3& point, const FRgba32u& col) const NOEXCEPT;
    PPE_RHI_API void Store(const int3& point, const FRgba32i& col) const NOEXCEPT;

    // https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#textures-wrapping-operation
    static CONSTEXPR u32 Mirror(i32 n) { return checked_cast<u32>(n >= 0 ? n : -(1 + n)); }
    static CONSTEXPR u32 Repeat(i32 value, u32 size) { return (value % size); }
    static CONSTEXPR u32 MirroredRepeat(i32 value, u32 size) { return ((size - 1) - Mirror((value % (2 * size)) - size)); }
    static CONSTEXPR u32 ClampToEdge(i32 value, u32 size) { return checked_cast<u32>(Clamp(value, 0_i32, static_cast<i32>(size) - 1)); }
    static CONSTEXPR u32 ClampToBorder(i32 value, u32 size) { return checked_cast<u32>(Clamp(value, -1_i32, static_cast<i32>(size))); }
    static CONSTEXPR u32 MirrorClampToEdge(i32 value, u32 size) { return Clamp(Mirror(value), 0_u32, size - 1); }

private:
    FBufferView _parts;

    size_t _rowPitch{ 0 };
    size_t _slicePitch{ 0 };
    size_t _bitsPerPixel{ 0 };

    uint3 _dimensions{ 0 };

    EPixelFormat _format{ Default };
    EAddressMode _addressMode{ Default };

    FPixelDecodeRGBA32f _loadRGBA32f{ nullptr };
    FPixelDecodeRGBA32i _loadRGBA32i{ nullptr };
    FPixelDecodeRGBA32u _loadRGBA32u{ nullptr };

    FPixelEncodeRGBA32f _storeRGBA32f{ nullptr };
    FPixelEncodeRGBA32i _storeRGBA32i{ nullptr };
    FPixelEncodeRGBA32u _storeRGBA32u{ nullptr };
};
PPE_ASSUME_TYPE_AS_POD(FImageView)
//----------------------------------------------------------------------------
// implementation guaranties that single row completely placed to solid memory block.
auto FImageView::Row(u32 y, u32 z) const NOEXCEPT -> subpart_type {
    Assert(y < _dimensions.y);
    Assert(z < _dimensions.z);

    return _parts.SubRange(static_cast<size_t>(_slicePitch) * z + static_cast<size_t>(_rowPitch) * y, _rowPitch);
}
//----------------------------------------------------------------------------
// implementation doesn't guarantee that a whole slice will fit in a single memory block,
// so check result size with 'SliceSize()'.
auto FImageView::Slice(u32 z) const NOEXCEPT -> subpart_type {
    Assert(z < _dimensions.z);

    return _parts.SubRange(_slicePitch * z, _slicePitch);
}
//----------------------------------------------------------------------------
auto FImageView::At(const uint3& point) const NOEXCEPT -> subpart_type {
    Assert(AllLess(point, _dimensions));

    return Row(point.y, point.z).SubRange((_bitsPerPixel * point.x) / 8, (_bitsPerPixel + 7) / 8);
}
//----------------------------------------------------------------------------
auto FImageView::Pixel(const int3& point) const NOEXCEPT -> subpart_type {
    uint3 safePoint;
    switch (_addressMode) {
    case EAddressMode::Repeat:
        safePoint = {   Repeat(point.x, _dimensions.x),
                        Repeat(point.y, _dimensions.y),
                        Repeat(point.z, _dimensions.z) };
        break;
    case EAddressMode::ClampToBorder:
        safePoint = {   ClampToBorder(point.x, _dimensions.x),
                        ClampToBorder(point.y, _dimensions.y),
                        ClampToBorder(point.z, _dimensions.z) };
        break;
    case EAddressMode::ClampToEdge:
        safePoint = {   ClampToEdge(point.x, _dimensions.x),
                        ClampToEdge(point.y, _dimensions.y),
                        ClampToEdge(point.z, _dimensions.z) };
        break;
    case EAddressMode::MirrorRepeat:
        safePoint = {   MirroredRepeat(point.x, _dimensions.x),
                        MirroredRepeat(point.y, _dimensions.y),
                        MirroredRepeat(point.z, _dimensions.z) };
        break;
    case EAddressMode::MirrorClampToEdge:
        safePoint = {   MirrorClampToEdge(point.x, _dimensions.x),
                        MirrorClampToEdge(point.y, _dimensions.y),
                        MirrorClampToEdge(point.z, _dimensions.z) };
        break;

    default:
        AssertNotImplemented();
    }

    return At(safePoint);
}
//----------------------------------------------------------------------------
auto FImageView::Texel(const float3& uvw) const NOEXCEPT -> subpart_type {
    Assert(AllLessEqual(uvw, float3(1.f)));
    Assert(AllGreaterEqual(uvw, float3(0.f)));

    return Pixel(TexCoordsToPoint(uvw));
}
//----------------------------------------------------------------------------
float3 FImageView::PointToTexCoords(const int3& point) const NOEXCEPT {
    return ((float3(point) + 0.5f) / float3(Dimensions()));
}
//----------------------------------------------------------------------------
int3 FImageView::TexCoordsToPoint(const float3& uvw) const NOEXCEPT {
    return TruncToInt(uvw * float3(Dimensions()));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
