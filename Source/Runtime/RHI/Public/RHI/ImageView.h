#pragma once

#include "RHI_fwd.h"
#include "Maths/ScalarVectorHelpers.h"

#include "RHI/BufferView.h"
#include "RHI/ResourceEnums.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FImageView {
    using value_type = FBufferView::value_type;
    using subpart_type = FBufferView::subpart_type;
    using iterator = FBufferView::FIterator;

    using FDecodeRGBA32f = void (*)(FRgba32f*, subpart_type);
    using FDecodeRGBA32u = void (*)(FRgba32u*, subpart_type);
    using FDecodeRGBA32i = void (*)(FRgba32i*, subpart_type);

    FImageView() = default;
    PPE_RHI_API FImageView(
        const TMemoryView<subpart_type>& parts,
        const uint3& dimensions,
        size_t rowPitch,
        size_t slicePitch,
        EPixelFormat format,
        EImageAspect aspect );

    bool empty() const { return _parts.empty(); }
    size_t size() const { return _parts.size(); }
    const value_type* data() const { return _parts.data(); }

    iterator begin() const { return _parts.begin(); }
    iterator end() const { return _parts.end(); }

    const TMemoryView<subpart_type>& Parts() const { return _parts.Parts(); }
    const uint3& Dimensions() const { return _dimensions; }
    size_t RowPitch() const { return _rowPitch; }
    size_t SlicePitch() const { return _rowPitch; }
    u32 BitsPerPixel() const { return _bitsPerPixel; }
    EPixelFormat Format() const { return _format; }

    size_t RowSize() const { return (_dimensions.x * _bitsPerPixel) / 8; }
    size_t SliceSize() const { return (_rowPitch * _dimensions.y); }

    // implementation garanties that single row completely placed to solid memory block.
    subpart_type Row(u32 y, u32 z = 0) const {
        Assert(y < _dimensions.y);
        Assert(z < _dimensions.z);

        const size_t rowSize = RowSize() / sizeof(value_type);
        auto row = _parts.SubRange(_slicePitch * z + _rowPitch * y, rowSize);

        Assert_NoAssume(row.size() == rowSize);
        return row;
    }

    // implementation doesn't garanties that single slice compilete placed to solid memory block,
    // so check result size with 'SliceSize()'.
    subpart_type Slice(u32 z) const {
        Assert(z < _dimensions.z);

        const size_t sliceSize = SliceSize() / sizeof(value_type);
        return _parts.SubRange(_slicePitch * z, sliceSize);
    }

    subpart_type Pixel(const uint3& point) const {
        Assert(AllLess(point, _dimensions));

        return Row(point.x, point.y).SubRange((_bitsPerPixel * point.x) / 8, (_bitsPerPixel + 7) / 8);
    }

    void Load(FRgba32f* pcol, const uint3& point) const {
        Assert(pcol);
        Assert(_loadRGBA32f);
        return _loadRGBA32f(pcol, Pixel(point));
    }
    void Load(FRgba32u* pcol, const uint3& point) const {
        Assert(pcol);
        Assert(_loadRGBA32u);
        return _loadRGBA32u(pcol, Pixel(point));
    }
    void Load(FRgba32i* pcol, const uint3& point) const {
        Assert(pcol);
        Assert(_loadRGBA32i);
        return _loadRGBA32i(pcol, Pixel(point));
    }

private:
    FBufferView _parts;
    uint3 _dimensions{ 0 };
    size_t _rowPitch{ 0 };
    size_t _slicePitch{ 0 };
    u32 _bitsPerPixel{ 0 };
    EPixelFormat _format{ Default };

    FDecodeRGBA32f _loadRGBA32f{ nullptr };
    FDecodeRGBA32u _loadRGBA32u{ nullptr };
    FDecodeRGBA32i _loadRGBA32i{ nullptr };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
