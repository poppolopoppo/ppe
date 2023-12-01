#pragma once

#include "RHI_fwd.h"

#include "RHI/BufferView.h"

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
        size_t rowPitch,
        size_t  slicePitch,
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
    size_t SlicePitch() const { return _rowPitch; }
    size_t BitsPerPixel() const { return _bitsPerPixel; }
    EPixelFormat Format() const { return _format; }

    size_t RowSize() const { return (_dimensions.x * _bitsPerPixel) / 8; }
    size_t SliceSize() const { return (_rowPitch * _dimensions.y); }

    // implementation guaranties that single row completely placed to solid memory block.
    inline subpart_type Row(u32 y, u32 z = 0) const NOEXCEPT;

    // implementation doesn't guarantee that a whole slice will fit in a single memory block,
    // so check result size with 'SliceSize()'.
    inline subpart_type Slice(u32 z) const NOEXCEPT;

    inline subpart_type Pixel(const uint3& point) const NOEXCEPT;

    inline subpart_type Texel(const float3& clip) const NOEXCEPT;

    PPE_RHI_API void Load(FRgba32f* pcol, const uint3& point) const NOEXCEPT;
    PPE_RHI_API void Load(FRgba32u* pcol, const uint3& point) const NOEXCEPT;
    PPE_RHI_API void Load(FRgba32i* pcol, const uint3& point) const NOEXCEPT;

    PPE_RHI_API void Load(FRgba32f* pcol, const float3& uvw) const NOEXCEPT;
    PPE_RHI_API void Load(FRgba32u* pcol, const float3& uvw) const NOEXCEPT;
    PPE_RHI_API void Load(FRgba32i* pcol, const float3& uvw) const NOEXCEPT;

    PPE_RHI_API void Load(FRgba32f* pcol, const float3& uvw, ETextureFilter filter) const NOEXCEPT;

private:
    FBufferView _parts;
    uint3 _dimensions{ 0 };
    size_t _rowPitch{ 0 };
    size_t _slicePitch{ 0 };
    size_t _bitsPerPixel{ 0 };
    EPixelFormat _format{ Default };

    FPixelDecodeRGBA32f _loadRGBA32f{ nullptr };
    FPixelDecodeRGBA32i _loadRGBA32i{ nullptr };
    FPixelDecodeRGBA32u _loadRGBA32u{ nullptr };
};
PPE_ASSUME_TYPE_AS_POD(FImageView)

//----------------------------------------------------------------------------
// implementation guaranties that single row completely placed to solid memory block.
auto FImageView::Row(u32 y, u32 z) const NOEXCEPT -> subpart_type {
    Assert(y < _dimensions.y);
    Assert(z < _dimensions.z);

    const size_t rowSize = RowSize() / sizeof(value_type);
    const size_t rowOffset = static_cast<size_t>(_slicePitch) * z + static_cast<size_t>(_rowPitch) * y;
    auto row = _parts.SubRange(rowOffset, rowSize);

    Assert_NoAssume(row.size() == rowSize);
    return row;
}
//----------------------------------------------------------------------------
// implementation doesn't guarantee that a whole slice will fit in a single memory block,
// so check result size with 'SliceSize()'.
auto FImageView::Slice(u32 z) const NOEXCEPT -> subpart_type {
    Assert(z < _dimensions.z);

    const size_t sliceSize = SliceSize() / sizeof(value_type);
    return _parts.SubRange(_slicePitch * z, sliceSize);
}
//----------------------------------------------------------------------------
auto FImageView::Pixel(const uint3& point) const NOEXCEPT -> subpart_type {
    Assert(AllLess(point, _dimensions));

    return Row(point.y, point.z).SubRange((_bitsPerPixel * point.x) / 8, (_bitsPerPixel + 7) / 8);
}
//----------------------------------------------------------------------------
auto FImageView::Texel(const float3& clip) const NOEXCEPT -> subpart_type {
    Assert(AllLessEqual(clip, float3(1.f)));
    Assert(AllGreaterEqual(clip, float3(-1.f)));

    const uint3 point = TruncToUnsigned(
        (clip + 1.0f) * 0.5f * float3(Dimensions()) + (0.5f - Epsilon));
    return Pixel(point);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
