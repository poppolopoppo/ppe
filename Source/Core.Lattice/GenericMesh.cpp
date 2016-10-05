#include "stdafx.h"

#include "GenericMesh.h"

#include "Lattice_fwd.h"

#include "Core.Graphics/Device/Geometry/IndexElementSize.h"
#include "Core.Graphics/Device/Geometry/VertexDeclaration.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Container/Hash.h"

namespace Core {
namespace Lattice {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Lattice, FGenericMesh, );
//----------------------------------------------------------------------------
FGenericMesh::FGenericMesh() : _indexCount(0), _vertexCount(0) {}
//----------------------------------------------------------------------------
FGenericMesh::~FGenericMesh() {
    Clear();
}
//----------------------------------------------------------------------------
void FGenericMesh::AddTriangle(size_t i0, size_t i1, size_t i2, size_t offset/* = 0 */) {
    const u32 src[3] = {
        checked_cast<u32>(offset + i0),
        checked_cast<u32>(offset + i1),
        checked_cast<u32>(offset + i2)
    };

    _indexCount += 3;
    _indices.WriteArray(src);
}
//----------------------------------------------------------------------------
const FGenericVertexData* FGenericMesh::GetVertexData(const Graphics::FVertexSemantic& semantic, size_t index, Graphics::EValueType type) const {
    const FGenericVertexData* vertices = GetVertexDataIFP(semantic, index, type);
    Assert(nullptr != vertices);
    return vertices;
}
//----------------------------------------------------------------------------
const FGenericVertexData* FGenericMesh::GetVertexDataIFP(const Graphics::FVertexSemantic& semantic, size_t index, Graphics::EValueType type) const {
    Assert(!semantic.empty());
    Assert(Graphics::EValueType::Void != type);

    for (const FGenericVertexData& vertices : _vertices) {
        if (vertices.Semantic() == semantic &&
            vertices.Index() == index) {
            Assert(vertices.Type() == type);
            return &vertices;
        }
    }

    return nullptr;
}
//----------------------------------------------------------------------------
void FGenericMesh::AddVertexData(const Graphics::FVertexSemantic& semantic, size_t index, Graphics::EValueType type) {
    Assert(nullptr == GetVertexDataIFP(semantic, index, type));

    _vertices.Push(this, semantic, index, type);
}
//----------------------------------------------------------------------------
FGenericVertexData* FGenericMesh::GetOrAddVertexData(const Graphics::FVertexSemantic& semantic, size_t index, Graphics::EValueType type) {
    const FGenericVertexData* vertices = GetVertexDataIFP(semantic, index, type);
    if (nullptr == vertices) {
        _vertices.Push(this, semantic, index, type);
        return _vertices.Peek();
    }
    else {
        return remove_const(vertices);
    }
}
//----------------------------------------------------------------------------
bool FGenericMesh::AreVertexEquals(size_t v0, size_t v1) const {
    Assert(v0 < _vertexCount);
    Assert(v1 < _vertexCount);
    if (v0 == v1)
        return true;

    for (const FGenericVertexData& vertices : _vertices) {
        Assert(_vertexCount == vertices.VertexCount());
        const size_t strideInBytes = vertices.StrideInBytes();

        const TMemoryView<const u8> rawData = vertices.MakeView();
        const TMemoryView<const u8> lhsData = rawData.SubRange(v0 * strideInBytes, strideInBytes);
        const TMemoryView<const u8> rhsData = rawData.SubRange(v1 * strideInBytes, strideInBytes);

        if (not Graphics::ValueEquals(vertices.Type(), lhsData, rhsData))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
hash_t FGenericMesh::VertexHash(size_t v, size_t seed/* = 0 */) const {
    Assert(v < _vertexCount);

    hash_t result = seed;

    for (const FGenericVertexData& vertices : _vertices) {
        Assert(_vertexCount == vertices.VertexCount());
        const TMemoryView<const u8>& rawData = vertices.MakeView();
        const size_t strideInBytes = vertices.StrideInBytes();

        const auto subpart = rawData.SubRange(v * strideInBytes, strideInBytes);
        const hash_t h = Graphics::ValueHash(vertices.Type(), subpart);

        hash_combine(result, h);
    }

    return result;
}
//----------------------------------------------------------------------------
void FGenericMesh::VertexCopy(size_t dst, size_t src) {
    Assert(dst < _vertexCount);
    Assert(src < _vertexCount);
    if (src == dst)
        return;

    for (FGenericVertexData& vertices : _vertices) {
        Assert(_vertexCount == vertices.VertexCount());
        const size_t strideInBytes = vertices.StrideInBytes();

        const TMemoryView<u8> rawData = vertices.MakeView();

#if 1
        // no need for memory semantic, assumes pod and should be faster
        memcpy( rawData.data() + dst * strideInBytes,
                rawData.data() + src * strideInBytes,
                strideInBytes );
#else
        const TMemoryView<u8> dstData = rawData.SubRange(dst * strideInBytes, strideInBytes);
        const TMemoryView<const u8> srcData = rawData.SubRange(src * strideInBytes, strideInBytes);
        Graphics::ValueCopy(vertices.Type(), dstData, srcData);
#endif
    }
}
//----------------------------------------------------------------------------
void FGenericMesh::VertexSwap(size_t v0, size_t v1) {
    Assert(v0 < _vertexCount);
    Assert(v1 < _vertexCount);
    if (v0 == v1)
        return;

    for (FGenericVertexData& vertices : _vertices) {
        Assert(_vertexCount == vertices.VertexCount());
        const size_t strideInBytes = vertices.StrideInBytes();

        const TMemoryView<u8> rawData = vertices.MakeView();
        const TMemoryView<u8> lhsData = rawData.SubRange(v0 * strideInBytes, strideInBytes);
        const TMemoryView<u8> rhsData = rawData.SubRange(v1 * strideInBytes, strideInBytes);

        Graphics::ValueSwap(vertices.Type(), lhsData, rhsData);
    }
}
//----------------------------------------------------------------------------
void FGenericMesh::IndexAndVertexSwap(size_t v0, size_t v1) {
    VertexSwap(v0, v1);

    const u32 iv0 = checked_cast<u32>(v0);
    const u32 iv1 = checked_cast<u32>(v1);

    for (u32& index : _indices.MakeView().Cast<u32>()) {
        if (index == v0)
            index = iv1;
        else if (index == v1)
            index = iv0;
    }
}
//----------------------------------------------------------------------------
void FGenericMesh::Resize(size_t indexCount, size_t vertexCount, bool keepData/* = true */) {
    if (_indexCount != indexCount) {
        _indexCount = indexCount;
        _indices.resize(indexCount * sizeof(u32), keepData);
    }
    if (_vertexCount != vertexCount) {
        _vertexCount = vertexCount;
        for (FGenericVertexData& vertices : _vertices)
            vertices.Resize(vertexCount, keepData);
    }
}
//----------------------------------------------------------------------------
void FGenericMesh::Reserve(size_t indexCount, size_t vertexCount, bool additional/* = false */) {
    if (additional) {
        indexCount += _indexCount;
        vertexCount += _vertexCount;
    }
    if (_indices.capacity() < indexCount) {
        _indices.reserve(indexCount * sizeof(u32));
    }
    if (_vertexCount < vertexCount) {
        for (FGenericVertexData& vertices : _vertices)
            vertices.Reserve(vertexCount);
    }
}
//----------------------------------------------------------------------------
bool FGenericMesh::ExportIndices(const TMemoryView<u16>& dst) const {
    return ExportIndices(Graphics::IndexElementSize::SixteenBits, dst.Cast<u8>());
}
//----------------------------------------------------------------------------
bool FGenericMesh::ExportIndices(const TMemoryView<u32>& dst) const {
    return ExportIndices(Graphics::IndexElementSize::ThirtyTwoBits, dst.Cast<u8>());
}
//----------------------------------------------------------------------------
bool FGenericMesh::ExportIndices(Graphics::IndexElementSize eltSize, const TMemoryView<u8>& dst) const  {
    Assert(dst.SizeInBytes() == size_t(eltSize) * _indexCount);

    if (Graphics::IndexElementSize::SixteenBits == eltSize) {
        if (_indexCount > TNumericLimits<u16>::MaxValue())
            return false;

        const u32* psrc = _indices.MakeView().Cast<const u32>().data();
        u16* pdst = dst.Cast<u16>().data();
        u16* pend = pdst + _indexCount;

        while (pend != pdst)
            *pdst++ = checked_cast<u16>(*psrc++);
    }
    else {
        Assert(Graphics::IndexElementSize::ThirtyTwoBits == eltSize);

        Copy(dst.Cast<u32>(), _indices.MakeView().Cast<const u32>());
    }

    return true;
}
//----------------------------------------------------------------------------
bool FGenericMesh::ExportVertices(const Graphics::FVertexDeclaration* vdecl, const TMemoryView<u8>& dst) const {
    Assert(vdecl);

    const size_t vertexSizeInBytes = vdecl->SizeInBytes();
    Assert(dst.SizeInBytes() == vertexSizeInBytes * _vertexCount);

    const TMemoryView<const Graphics::FValueBlock::TField> subParts = vdecl->SubParts();
    STACKLOCAL_POD_STACK(const FGenericVertexData*, channels, subParts.size());

    for (const Graphics::FValueBlock::TField& subPart : subParts) {
        bool found = false;

        for (const FGenericVertexData& vertices : _vertices) {
            Assert(vertices.VertexCount() == _vertexCount);

            if (subPart.Name() == vertices.Semantic() &&
                subPart.Index() == vertices.Index() &&
                subPart.IsPromotableFrom(vertices.Type()) ) {

                Assert(not channels.Contains(&vertices));
                channels.Push(&vertices);
                found = true;
            }
        }

        if (not found)
            return false;
    }

    if (0 == _vertexCount) // skips only after compatibility tests
        return true;

    Assert(channels.size() == subParts.size());

    forrange(c, 0, channels.size()) {
        const FGenericVertexData* pchannel = channels[c];
        const Graphics::FValueBlock::TField& subPart = vdecl->SubPartByIndex(c);

        if (not subPart.PromoteArray(
            dst, vertexSizeInBytes,
            pchannel->Type(),
            pchannel->MakeView(),
            pchannel->StrideInBytes(),
            _vertexCount )) {
            AssertNotReached();
        }
    }

    return true;
}
//----------------------------------------------------------------------------
void FGenericMesh::Clear() {
#ifdef WITH_CORE_ASSERT
    Assert((_indexCount % 3) == 0);
    for (const FGenericVertexData& vertices : _vertices) {
        Assert(vertices.VertexCount() == _vertexCount);
        Assert(vertices.VertexCount() * vertices.StrideInBytes() == vertices.SizeInBytes());
    }
    for (u32 index : Indices()) {
        Assert(index < _vertexCount);
    }
#endif

    ClearIndices();
    ClearVertices();
}
//----------------------------------------------------------------------------
void FGenericMesh::ClearIndices() {
    _indexCount = 0;
    _indices.Clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
void FGenericMesh::ClearVertices() {
    _vertexCount = 0;
    _vertices.clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FGenericVertexData::FGenericVertexData(FGenericMesh* owner, const Graphics::FVertexSemantic& semantic, size_t index, Graphics::EValueType type)
    : _owner(owner), _semantic(semantic), _type(type), _index(index), _vertexCount(0) {
    Assert(nullptr != _owner);
    Assert(Graphics::EValueType::Void != _type);
}
//----------------------------------------------------------------------------
FGenericVertexData::~FGenericVertexData() {}
//----------------------------------------------------------------------------
void FGenericVertexData::Resize(size_t count, bool keepData/* = true */) {
    if (count != _vertexCount) {
        _vertexCount = count;
        _stream.resize(StrideInBytes() * count, keepData);
    }
}
//----------------------------------------------------------------------------
void FGenericVertexData::Reserve(size_t count) {
    if (count > _vertexCount)
        _stream.reserve(StrideInBytes() * count);
}
//----------------------------------------------------------------------------
TMemoryView<const u8> FGenericVertexData::VertexView(size_t v) const {
    Assert(v < _vertexCount);

    const size_t strideInBytes = StrideInBytes();
    return _stream.MakeView().SubRange(v * strideInBytes, strideInBytes);
}
//----------------------------------------------------------------------------
void FGenericVertexData::CopyVertex(size_t dst, size_t src) {
    Assert(dst < _vertexCount);
    Assert(src < _vertexCount);

    if (dst == src)
        return;

    const size_t strideInBytes = StrideInBytes();
    const TMemoryView<u8> storage = _stream.MakeView();

    memcpy(&storage[dst * strideInBytes], &storage[src * strideInBytes], strideInBytes);
}
//----------------------------------------------------------------------------
void FGenericVertexData::ReadVertex(size_t v, Graphics::FValue& dst) const {
    dst.SetRaw(_type, VertexView(v));
}
//----------------------------------------------------------------------------
void FGenericVertexData::WriteVertex(size_t v, const Graphics::FValue& src) {
    Graphics::ValuePromote(_type, VertexView(v).RemoveConst(), src.Type(), src.MakeView());
}
//----------------------------------------------------------------------------
TMemoryView<u8> FGenericVertexData::SubRange(size_t start, size_t count) {
    Assert(start + count <= _vertexCount);

    const size_t strideInBytes = StrideInBytes();
    return _stream.MakeView().SubRange(start * strideInBytes, count * strideInBytes);
}
//----------------------------------------------------------------------------
TMemoryView<const u8> FGenericVertexData::SubRange(size_t start, size_t count) const {
    Assert(start + count <= _vertexCount);

    const size_t strideInBytes = StrideInBytes();
    return _stream.MakeView().SubRange(start * strideInBytes, count * strideInBytes);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define GENERICVERTEX_SEMANTIC_DEF(_NAME, _INDEX, _TYPE) \
    TGenericVertexSubPart< _TYPE > FGenericMesh::_NAME ## _INDEX ## f(size_t index) { \
        return TGenericVertexSubPart< _TYPE >( GetOrAddVertexData(Graphics::FVertexSemantic::_NAME, index, TGenericVertexSubPart< _TYPE >::Type) ); \
    } \
    TGenericVertexSubPart< _TYPE > FGenericMesh::_NAME ## _INDEX ## f_IFP(size_t index) const { \
        return TGenericVertexSubPart< _TYPE >( GetVertexData(Graphics::FVertexSemantic::_NAME, index, TGenericVertexSubPart< _TYPE >::Type) ); \
    }
//----------------------------------------------------------------------------
GENERICVERTEX_SEMANTIC_DEF(Position,    3, float3)
GENERICVERTEX_SEMANTIC_DEF(Position,    4, float4)
GENERICVERTEX_SEMANTIC_DEF(TexCoord,    2, float2)
GENERICVERTEX_SEMANTIC_DEF(TexCoord,    3, float3)
GENERICVERTEX_SEMANTIC_DEF(TexCoord,    4, float4)
GENERICVERTEX_SEMANTIC_DEF(Color,       4, float4)
GENERICVERTEX_SEMANTIC_DEF(Normal,      3, float3)
GENERICVERTEX_SEMANTIC_DEF(Tangent,     3, float3)
GENERICVERTEX_SEMANTIC_DEF(Tangent,     4, float4)
GENERICVERTEX_SEMANTIC_DEF(Binormal,    3, float3)
//----------------------------------------------------------------------------
#undef GENERICVERTEX_SEMANTIC_DEF
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace Core
