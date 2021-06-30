#pragma once

#include "RHI_fwd.h"

#include "RHI/BufferView.h"
#include "RHI/PixelFormatHelpers.h"
#include "RHI/ResourceEnums.h"

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

    FImageView(
        const TMemoryView<const subpart_type>& parts,
        const uint3& dimensions,
        u32 rowPitch,
        u32 slicePitch,
        EPixelFormat format,
        EImageAspect aspect ) NOEXCEPT
    :   _parts(parts)
    ,   _dimensions(dimensions)
    ,   _rowPitch(rowPitch)
    ,   _slicePitch(slicePitch)
    ,   _format(format) {
        const auto encoding = EPixelFormat_Encoding(_format, aspect);
        _bitsPerPixel = encoding.BitsPerPixel;
        _loadRGBA32f = encoding.DecodeRGBA32f;
        _loadRGBA32i = encoding.DecodeRGBA32i;
        _loadRGBA32u = encoding.DecodeRGBA32u;
    }

    bool empty() const { return _parts.empty(); }
    size_t size() const { return _parts.size(); }
    const value_type* data() const { return _parts.data(); }

    iterator begin() const { return _parts.begin(); }
    iterator end() const { return _parts.end(); }

    TMemoryView<const subpart_type> Parts() const { return _parts.Parts(); }
    const uint3& Dimensions() const { return _dimensions; }
    u32 RowPitch() const { return _rowPitch; }
    u32 SlicePitch() const { return _rowPitch; }
    u32 BitsPerPixel() const { return _bitsPerPixel; }
    EPixelFormat Format() const { return _format; }

    u32 RowSize() const { return (_dimensions.x * _bitsPerPixel) / 8; }
    u32 SliceSize() const { return (_rowPitch * _dimensions.y); }

    // implementation guaranties that single row completely placed to solid memory block.
    subpart_type Row(u32 y, u32 z = 0) const {
        Assert(y < _dimensions.y);
        Assert(z < _dimensions.z);

        const size_t rowSize = RowSize() / sizeof(value_type);
        auto row = _parts.SubRange(_slicePitch * z + _rowPitch * y, rowSize);

        Assert_NoAssume(row.size() == rowSize);
        return row;
    }

    // implementation doesn't guaranties that single slice complete placed to solid memory block,
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
    u32 _rowPitch{ 0 };
    u32 _slicePitch{ 0 };
    u32 _bitsPerPixel{ 0 };
    EPixelFormat _format{ Default };

    FPixelDecodeRGBA32f _loadRGBA32f{ nullptr };
    FPixelDecodeRGBA32i _loadRGBA32i{ nullptr };
    FPixelDecodeRGBA32u _loadRGBA32u{ nullptr };
};
PPE_ASSUME_TYPE_AS_POD(FImageView)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
