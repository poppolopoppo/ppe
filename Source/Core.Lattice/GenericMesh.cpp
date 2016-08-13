#include "stdafx.h"

#include "GenericMesh.h"

#include "Core.Graphics/Device/Geometry/IndexElementSize.h"
#include "Core.Graphics/Device/Geometry/VertexDeclaration.h"

#include "Core/Container/Hash.h"

namespace Core {
namespace Lattice {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
GenericVertexData::GenericVertexData(GenericMesh* owner, const Graphics::VertexSemantic& semantic, size_t index, Graphics::ValueType type)
    : _owner(owner), _semantic(semantic), _type(type), _index(index), _vertexCount(0) {
    Assert(nullptr != _owner);
    Assert(Graphics::ValueType::Void != _type);
}
//----------------------------------------------------------------------------
GenericVertexData::~GenericVertexData() {}
//----------------------------------------------------------------------------
void GenericVertexData::Resize(size_t count, bool keepData/* = true */) {
    if (count != _vertexCount) {
        _vertexCount = count;
        _stream.resize(StrideInBytes() * count, keepData);
    }
}
//----------------------------------------------------------------------------
void GenericVertexData::Reserve(size_t count) {
    if (count > _vertexCount)
        _stream.reserve(StrideInBytes() * count);
}
//----------------------------------------------------------------------------
MemoryView<const u8> GenericVertexData::VertexView(size_t v) const {
    Assert(v < _vertexCount);

    const size_t strideInBytes = StrideInBytes();
    return _stream.MakeView().SubRange(v * strideInBytes, strideInBytes);
}
//----------------------------------------------------------------------------
void GenericVertexData::CopyVertex(size_t dst, size_t src) {
    Assert(dst < _vertexCount);
    Assert(src < _vertexCount);

    if (dst == src)
        return;

    const size_t strideInBytes = StrideInBytes();
    const MemoryView<u8> storage = _stream.MakeView();

    memcpy(&storage[dst * strideInBytes], &storage[src * strideInBytes], strideInBytes);
}
//----------------------------------------------------------------------------
void GenericVertexData::ReadVertex(size_t v, Graphics::Value& dst) const {
    dst.SetRaw(_type, VertexView(v));
}
//----------------------------------------------------------------------------
void GenericVertexData::WriteVertex(size_t v, const Graphics::Value& src) {
    Graphics::ValuePromote(_type, VertexView(v).RemoveConst(), src.Type(), src.MakeView());
}
//----------------------------------------------------------------------------
MemoryView<u8> GenericVertexData::SubRange(size_t start, size_t count) {
    Assert(start + count <= _vertexCount);

    const size_t strideInBytes = StrideInBytes();
    return _stream.MakeView().SubRange(start * strideInBytes, count * strideInBytes);
}
//----------------------------------------------------------------------------
MemoryView<const u8> GenericVertexData::SubRange(size_t start, size_t count) const {
    Assert(start + count <= _vertexCount);

    const size_t strideInBytes = StrideInBytes();
    return _stream.MakeView().SubRange(start * strideInBytes, count * strideInBytes);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
GenericMesh::GenericMesh() : _indexCount(0), _vertexCount(0) {}
//----------------------------------------------------------------------------
GenericMesh::~GenericMesh() {
    Clear();
}
//----------------------------------------------------------------------------
void GenericMesh::AddTriangle(size_t i0, size_t i1, size_t i2, size_t offset/* = 0 */) {
    const u32 src[3] = {
        checked_cast<u32>(offset + i0),
        checked_cast<u32>(offset + i1),
        checked_cast<u32>(offset + i2)
    };

    _indexCount += 3;
    _indices.WriteArray(src);
}
//----------------------------------------------------------------------------
const GenericVertexData* GenericMesh::GetVertexData(const Graphics::VertexSemantic& semantic, size_t index, Graphics::ValueType type) const {
    const GenericVertexData* vertices = GetVertexDataIFP(semantic, index, type);
    Assert(nullptr != vertices);
    return vertices;
}
//----------------------------------------------------------------------------
const GenericVertexData* GenericMesh::GetVertexDataIFP(const Graphics::VertexSemantic& semantic, size_t index, Graphics::ValueType type) const {
    Assert(!semantic.empty());
    Assert(Graphics::ValueType::Void != type);

    for (const GenericVertexData& vertices : _vertices) {
        if (vertices.Semantic() == semantic &&
            vertices.Index() == index) {
            Assert(vertices.Type() == type);
            return &vertices;
        }
    }

    return nullptr;
}
//----------------------------------------------------------------------------
void GenericMesh::AddVertexData(const Graphics::VertexSemantic& semantic, size_t index, Graphics::ValueType type) {
    Assert(nullptr == GetVertexDataIFP(semantic, index, type));

    _vertices.Push(this, semantic, index, type);
}
//----------------------------------------------------------------------------
GenericVertexData* GenericMesh::GetOrAddVertexData(const Graphics::VertexSemantic& semantic, size_t index, Graphics::ValueType type) {
    const GenericVertexData* vertices = GetVertexDataIFP(semantic, index, type);
    if (nullptr == vertices) {
        _vertices.Push(this, semantic, index, type);
        return _vertices.Peek();
    }
    else {
        return const_cast<GenericVertexData*>(vertices);
    }
}
//----------------------------------------------------------------------------
bool GenericMesh::AreVertexEquals(size_t v0, size_t v1) const {
    Assert(v0 < _vertexCount);
    Assert(v1 < _vertexCount);
    if (v0 == v1)
        return true;

    for (const GenericVertexData& vertices : _vertices) {
        Assert(_vertexCount == vertices.VertexCount());
        const size_t strideInBytes = vertices.StrideInBytes();

        const MemoryView<const u8> rawData = vertices.MakeView();
        const MemoryView<const u8> lhsData = rawData.SubRange(v0 * strideInBytes, strideInBytes);
        const MemoryView<const u8> rhsData = rawData.SubRange(v1 * strideInBytes, strideInBytes);

        if (not Graphics::ValueEquals(vertices.Type(), lhsData, rhsData))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
hash_t GenericMesh::VertexHash(size_t v, size_t seed/* = 0 */) const {
    Assert(v < _vertexCount);

    hash_t result = seed;

    for (const GenericVertexData& vertices : _vertices) {
        Assert(_vertexCount == vertices.VertexCount());
        const MemoryView<const u8>& rawData = vertices.MakeView();
        const size_t strideInBytes = vertices.StrideInBytes();

        const auto subpart = rawData.SubRange(v * strideInBytes, strideInBytes);
        const hash_t h = Graphics::ValueHash(vertices.Type(), subpart);

        hash_combine(result, h);
    }

    return result;
}
//----------------------------------------------------------------------------
void GenericMesh::VertexCopy(size_t dst, size_t src) {
    Assert(dst < _vertexCount);
    Assert(src < _vertexCount);
    if (src == dst)
        return;

    for (GenericVertexData& vertices : _vertices) {
        Assert(_vertexCount == vertices.VertexCount());
        const size_t strideInBytes = vertices.StrideInBytes();

        const MemoryView<u8> rawData = vertices.MakeView();

#if 1
        // no need for memory semantic, assumes pod and should be faster
        memcpy( rawData.data() + dst * strideInBytes,
                rawData.data() + src * strideInBytes,
                strideInBytes );
#else
        const MemoryView<u8> dstData = rawData.SubRange(dst * strideInBytes, strideInBytes);
        const MemoryView<const u8> srcData = rawData.SubRange(src * strideInBytes, strideInBytes);
        Graphics::ValueCopy(vertices.Type(), dstData, srcData);
#endif
    }
}
//----------------------------------------------------------------------------
void GenericMesh::VertexSwap(size_t v0, size_t v1) {
    Assert(v0 < _vertexCount);
    Assert(v1 < _vertexCount);
    if (v0 == v1)
        return;

    for (GenericVertexData& vertices : _vertices) {
        Assert(_vertexCount == vertices.VertexCount());
        const size_t strideInBytes = vertices.StrideInBytes();

        const MemoryView<u8> rawData = vertices.MakeView();
        const MemoryView<u8> lhsData = rawData.SubRange(v0 * strideInBytes, strideInBytes);
        const MemoryView<u8> rhsData = rawData.SubRange(v1 * strideInBytes, strideInBytes);

        Graphics::ValueSwap(vertices.Type(), lhsData, rhsData);
    }
}
//----------------------------------------------------------------------------
void GenericMesh::IndexAndVertexSwap(size_t v0, size_t v1) {
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
void GenericMesh::Resize(size_t indexCount, size_t vertexCount, bool keepData/* = true */) {
    if (_indexCount != indexCount) {
        _indexCount = indexCount;
        _indices.resize(indexCount * sizeof(u32), keepData);
    }
    if (_vertexCount != vertexCount) {
        _vertexCount = vertexCount;
        for (GenericVertexData& vertices : _vertices)
            vertices.Resize(vertexCount, keepData);
    }
}
//----------------------------------------------------------------------------
void GenericMesh::Reserve(size_t indexCount, size_t vertexCount, bool additional/* = false */) {
    if (additional) {
        indexCount += _indexCount;
        vertexCount += _vertexCount;
    }
    if (_indices.capacity() < indexCount) {
        _indices.reserve(indexCount * sizeof(u32));
    }
    if (_vertexCount < vertexCount) {
        for (GenericVertexData& vertices : _vertices)
            vertices.Reserve(vertexCount);
    }
}
//----------------------------------------------------------------------------
bool GenericMesh::ExportIndices(const MemoryView<u16>& dst) const {
    return ExportIndices(Graphics::IndexElementSize::SixteenBits, dst.Cast<u8>());
}
//----------------------------------------------------------------------------
bool GenericMesh::ExportIndices(const MemoryView<u32>& dst) const {
    return ExportIndices(Graphics::IndexElementSize::ThirtyTwoBits, dst.Cast<u8>());
}
//----------------------------------------------------------------------------
bool GenericMesh::ExportIndices(Graphics::IndexElementSize eltSize, const MemoryView<u8>& dst) const  {
    Assert(dst.SizeInBytes() == size_t(eltSize) * _indexCount);

    if (Graphics::IndexElementSize::SixteenBits == eltSize) {
        if (_indexCount > NumericLimits<u16>::MaxValue())
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
bool GenericMesh::ExportVertices(const Graphics::VertexDeclaration* vdecl, const MemoryView<u8>& dst) const {
    Assert(vdecl);

    const size_t vertexSizeInBytes = vdecl->SizeInBytes();
    Assert(dst.SizeInBytes() == vertexSizeInBytes * _vertexCount);

    if (0 == _vertexCount)
        return true;

    const MemoryView<const Graphics::ValueBlock::Field> subParts = vdecl->SubParts();
    STACKLOCAL_POD_STACK(const GenericVertexData*, channels, subParts.size());

    for (const Graphics::ValueBlock::Field& subPart : subParts) {
        bool found = false;

        for (const GenericVertexData& vertices : _vertices) {
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

    Assert(channels.size() == subParts.size());

    forrange(c, 0, channels.size()) {
        const GenericVertexData* pchannel = channels[c];
        const Graphics::ValueBlock::Field& subPart = vdecl->SubPartByIndex(c);

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
void GenericMesh::Clear() {
#ifdef WITH_CORE_ASSERT
    Assert((_indexCount % 3) == 0);
    for (const GenericVertexData& vertices : _vertices) {
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
void GenericMesh::ClearIndices() {
    _indexCount = 0;
    _indices.Clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
void GenericMesh::ClearVertices() {
    _vertexCount = 0;
    _vertices.clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define GENERICVERTEX_SEMANTIC_DEF(_NAME, _INDEX, _TYPE) \
    GenericVertexSubPart< _TYPE > GenericMesh::_NAME ## _INDEX ## f(size_t index) { \
        return GenericVertexSubPart< _TYPE >( GetOrAddVertexData(Graphics::VertexSemantic::_NAME, index, GenericVertexSubPart< _TYPE >::Type) ); \
    } \
    GenericVertexSubPart< _TYPE > GenericMesh::_NAME ## _INDEX ## f_IFP(size_t index) const { \
        return GenericVertexSubPart< _TYPE >( GetVertexData(Graphics::VertexSemantic::_NAME, index, GenericVertexSubPart< _TYPE >::Type) ); \
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
