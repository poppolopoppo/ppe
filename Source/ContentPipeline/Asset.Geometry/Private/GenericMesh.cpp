// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "GenericMesh.h"

#include "Lattice_fwd.h"

#include "GenericMeshHelpers.h"

#include "Device/Geometry/IndexElementSize.h"
#include "Device/Geometry/VertexDeclaration.h"

#include "Allocator/PoolAllocator-impl.h"
#include "Container/Hash.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformMemory.h"
#include "Maths/ScalarBoundingBox.h"

namespace PPE {
namespace Lattice {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Lattice, FGenericMesh, );
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Lattice, FGenericVertexData, );
//----------------------------------------------------------------------------
FGenericMesh::FGenericMesh() : _indexCount(0), _vertexCount(0) {}
//----------------------------------------------------------------------------
FGenericMesh::~FGenericMesh() {
    Clear();
}
//----------------------------------------------------------------------------
void FGenericMesh::AddIndices(const TMemoryView<const u32>& indices) {
    _indexCount += indices.size();
    _indices.WriteView(indices);
}
//----------------------------------------------------------------------------
void FGenericMesh::AddTriangle(size_t i0, size_t i1, size_t i2, size_t offset/* = 0 */) {
    _indexCount += 3;
    _indices.WritePOD(checked_cast<u32>(offset + i0));
    _indices.WritePOD(checked_cast<u32>(offset + i1));
    _indices.WritePOD(checked_cast<u32>(offset + i2));
}
//----------------------------------------------------------------------------
const FGenericVertexData* FGenericMesh::GetVertexData(const Graphics::FVertexSemantic& semantic, size_t index, Graphics::EValueType type) const {
    const FGenericVertexData* vertices = GetVertexDataIFP(semantic, index, type);
    AssertRelease(nullptr != vertices);
    return vertices;
}
//----------------------------------------------------------------------------
const FGenericVertexData* FGenericMesh::GetVertexDataIFP(const Graphics::FVertexSemantic& semantic, size_t index, Graphics::EValueType type) const {
    Assert(!semantic.empty());
    Assert(Graphics::EValueType::Void != type);

    for (const UGenericVertexData& vertices : _vertices) {
        if (vertices->Semantic() == semantic &&
            vertices->Index() == index &&
            vertices->Type() == type) {
            return vertices.get();
        }
    }

    return nullptr;
}
//----------------------------------------------------------------------------
FGenericVertexData* FGenericMesh::AddVertexData(const Graphics::FVertexSemantic& semantic, size_t index, Graphics::EValueType type) {
    Assert(nullptr == GetVertexDataIFP(semantic, index, type));

    _vertices.Push(new FGenericVertexData(this, semantic, index, type));
    return _vertices.Peek()->get();
}
//----------------------------------------------------------------------------
FGenericVertexData* FGenericMesh::GetOrAddVertexData(const Graphics::FVertexSemantic& semantic, size_t index, Graphics::EValueType type) {
    const FGenericVertexData* vertices = GetVertexDataIFP(semantic, index, type);
    if (nullptr == vertices) {
        _vertices.Push(new FGenericVertexData(this, semantic, index, type));
        return _vertices.Peek()->get();
    }
    else {
        return remove_const(vertices);
    }
}
//----------------------------------------------------------------------------
void FGenericMesh::RemoveVertexData(const FGenericVertexData* data) {
    Assert(data);

    const auto it = std::find_if(_vertices.begin(), _vertices.end(), [data](const UGenericVertexData& v) {
        return (v.get() == data);
    });
    Assert(_vertices.end() != it);

    const size_t index = (&(*it) - &_vertices[0]);
    if (index + 1 < _vertices.size())
        swap(*_vertices.Peek(), _vertices[index]);

    _vertices.Pop();
}
//----------------------------------------------------------------------------
void FGenericMesh::RemoveVertexData(const Graphics::FVertexSemantic& semantic, size_t index, Graphics::EValueType type) {
    const FGenericVertexData* data = GetVertexData(semantic, index, type);
    AssertRelease(nullptr != data);
    RemoveVertexData(data);
}
//----------------------------------------------------------------------------
bool FGenericMesh::AreVertexEquals(size_t v0, size_t v1) const {
    Assert(v0 < _vertexCount);
    Assert(v1 < _vertexCount);
    if (v0 == v1)
        return true;

    for (const UGenericVertexData& vertices : _vertices) {
        Assert(_vertexCount == vertices->VertexCount());
        const size_t strideInBytes = vertices->StrideInBytes();

        const TMemoryView<const u8> rawData = vertices->MakeView();
        const TMemoryView<const u8> lhsData = rawData.SubRange(v0 * strideInBytes, strideInBytes);
        const TMemoryView<const u8> rhsData = rawData.SubRange(v1 * strideInBytes, strideInBytes);

        if (not Graphics::ValueEquals(vertices->Type(), lhsData, rhsData))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
hash_t FGenericMesh::VertexHash(size_t v, size_t seed/* = 0 */) const {
    Assert(v < _vertexCount);

    hash_t result = seed;

    for (const UGenericVertexData& vertices : _vertices) {
        Assert(_vertexCount == vertices->VertexCount());
        const TMemoryView<const u8>& rawData = vertices->MakeView();
        const size_t strideInBytes = vertices->StrideInBytes();

        const auto subpart = rawData.SubRange(v * strideInBytes, strideInBytes);
        const hash_t h = Graphics::ValueHash(vertices->Type(), subpart);

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

    for (const UGenericVertexData& vertices : _vertices) {
        Assert(_vertexCount == vertices->VertexCount());
        const size_t strideInBytes = vertices->StrideInBytes();

        const TMemoryView<u8> rawData = vertices->MakeView();

#if 1
        // no need for memory semantic, assumes pod and should be faster
        FPlatformMemory::MemcpyLarge(
            rawData.data() + dst * strideInBytes,
            rawData.data() + src * strideInBytes,
            strideInBytes );
#else
        const TMemoryView<u8> dstData = rawData.SubRange(dst * strideInBytes, strideInBytes);
        const TMemoryView<const u8> srcData = rawData.SubRange(src * strideInBytes, strideInBytes);
        Graphics::ValueCopy(vertices->Type(), dstData, srcData);
#endif
    }
}
//----------------------------------------------------------------------------
void FGenericMesh::VertexSwap(size_t v0, size_t v1) {
    Assert(v0 < _vertexCount);
    Assert(v1 < _vertexCount);
    if (v0 == v1)
        return;

    for (const UGenericVertexData& vertices : _vertices) {
        Assert(_vertexCount == vertices->VertexCount());
        const size_t strideInBytes = vertices->StrideInBytes();

        const TMemoryView<u8> rawData = vertices->MakeView();
        const TMemoryView<u8> lhsData = rawData.SubRange(v0 * strideInBytes, strideInBytes);
        const TMemoryView<u8> rhsData = rawData.SubRange(v1 * strideInBytes, strideInBytes);

        Graphics::ValueSwap(vertices->Type(), lhsData, rhsData);
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
        for (const UGenericVertexData& vertices : _vertices)
            vertices->Resize(vertexCount, keepData);
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
        for (const UGenericVertexData& vertices : _vertices)
            vertices->Reserve(vertexCount);
    }
}
//----------------------------------------------------------------------------
void FGenericMesh::CleanAndOptimize(size_t index) {
    if (!Position3f_IFP(index) && !Position4f_IFP(index))
        return;

    const size_t duplicated = MergeDuplicateVertices(*this);
    LOG(Info, L"[Mesh] removed {0}/{1} duplicate vertices = {2:f2}%", duplicated, _vertexCount, duplicated * 100.f / _vertexCount);

    const size_t unused = RemoveUnusedVertices(*this);
    LOG(Info, L"[Mesh] removed {0}/{1} unused vertices = {2:f2}%", unused, _vertexCount + unused, unused * 100.f / (_vertexCount + unused));

    const FAabb3f bounds = ComputeBounds(*this, index);
    const float minDistance = Min(bounds.Extents()) * 0.001f; // 0.1%
    const float minArea = minDistance * minDistance;

#if 0 // May merge needed vertices for interpolation : can't be in the default path
    const size_t close = MergeCloseVertices(*this, index, minDistance);
    LOG(Info, L"[Mesh] removed {0}/{1} close vertices (epsilon = {2}) = {3:f2}%", close, _vertexCount, minDistance, close * 100.f / _vertexCount);
#endif

    const size_t zero = RemoveZeroAreaTriangles(*this, index, minArea);
    LOG(Info, L"[Mesh] removed {0}/{1} zero area triangle indices (epsilon = {2}) = {3:f2}%", zero, _indexCount + zero, minArea, zero * 100.f / (_indexCount + zero));

    const size_t unused2 = RemoveUnusedVertices(*this);
    LOG(Info, L"[Mesh] removed {0}/{1} unused vertices = {2:f2}%", unused2, _vertexCount + unused2, unused2 * 100.f / (_vertexCount + unused2));

    const float VACMR_Before = VertexAverageCacheMissRate(*this);
    OptimizeIndicesAndVerticesOrder(*this);
    const float VACMR_After = VertexAverageCacheMissRate(*this);
    LOG(Info, L"[Mesh] optimized vertex average cache miss rate from {0:f3} to {1:f3} = {2:f2}%", VACMR_Before, VACMR_After, VACMR_After * 100.f / VACMR_Before);
}
//----------------------------------------------------------------------------
bool FGenericMesh::ExportIndices(const TMemoryView<u16>& dst) const {
    return ExportIndices(Graphics::EIndexElementSize::SixteenBits, dst.Cast<u8>());
}
//----------------------------------------------------------------------------
bool FGenericMesh::ExportIndices(const TMemoryView<u32>& dst) const {
    return ExportIndices(Graphics::EIndexElementSize::ThirtyTwoBits, dst.Cast<u8>());
}
//----------------------------------------------------------------------------
bool FGenericMesh::ExportIndices(Graphics::EIndexElementSize eltSize, const TMemoryView<u8>& dst) const  {
    Assert(dst.SizeInBytes() == size_t(eltSize) * _indexCount);

    if (Graphics::EIndexElementSize::SixteenBits == eltSize) {
        if (_indexCount > TNumericLimits<u16>::MaxValue())
            return false;

        const u32* psrc = _indices.MakeView().Cast<const u32>().data();
        u16* pdst = dst.Cast<u16>().data();
        u16* pend = pdst + _indexCount;

        while (pend != pdst)
            *pdst++ = checked_cast<u16>(*psrc++);
    }
    else {
        Assert(Graphics::EIndexElementSize::ThirtyTwoBits == eltSize);

        Copy(dst.Cast<u32>(), _indices.MakeView().Cast<const u32>());
    }

    return true;
}
//----------------------------------------------------------------------------
bool FGenericMesh::ExportVertices(const Graphics::FVertexDeclaration* vdecl, const TMemoryView<u8>& dst) const {
    Assert(vdecl);

    const size_t vertexSizeInBytes = vdecl->SizeInBytes();
    Assert(dst.SizeInBytes() == vertexSizeInBytes * _vertexCount);

    const TMemoryView<const Graphics::FValueField> subParts = vdecl->SubParts();
    STACKLOCAL_POD_STACK(const FGenericVertexData*, channels, subParts.size());

    for (const Graphics::FValueField& subPart : subParts) {
        bool found = false;

        for (const UGenericVertexData& vertices : _vertices) {
            Assert(vertices->VertexCount() == _vertexCount);

            if (subPart.Name() == vertices->Semantic() &&
                subPart.Index() == vertices->Index() &&
                subPart.IsPromotableFrom(vertices->Type()) ) {

                Assert(not channels.Contains(vertices.get()));
                channels.Push(vertices.get());
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
        const Graphics::FValueField& subPart = vdecl->SubPartByIndex(c);

        if (not subPart.PromoteArray(
            dst, vertexSizeInBytes,
            pchannel->Type(),
            pchannel->MakeView(),
            0, pchannel->StrideInBytes(),
            _vertexCount )) {
            AssertNotReached();
        }
    }

    return true;
}
//----------------------------------------------------------------------------
void FGenericMesh::Clear() {
#ifdef WITH_PPE_ASSERT
    Assert((_indexCount % 3) == 0);
    for (const UGenericVertexData& vertices : _vertices) {
        Assert(vertices->VertexCount() == _vertexCount);
        Assert(vertices->VertexCount() * vertices->StrideInBytes() == vertices->SizeInBytes());
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
    _indices.clear_ReleaseMemory();
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

    FPlatformMemory::MemcpyLarge(&storage[dst * strideInBytes], &storage[src * strideInBytes], strideInBytes);
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
        return TGenericVertexSubPart< _TYPE >( GetVertexDataIFP(Graphics::FVertexSemantic::_NAME, index, TGenericVertexSubPart< _TYPE >::Type) ); \
    }
//----------------------------------------------------------------------------
GENERICVERTEX_SEMANTIC_DEF(Position,    3, float3)
GENERICVERTEX_SEMANTIC_DEF(Position,    4, float4)
GENERICVERTEX_SEMANTIC_DEF(TexCoord,    2, float2)
GENERICVERTEX_SEMANTIC_DEF(TexCoord,    3, float3)
GENERICVERTEX_SEMANTIC_DEF(TexCoord,    4, float4)
GENERICVERTEX_SEMANTIC_DEF(Color,       4, float4)
GENERICVERTEX_SEMANTIC_DEF(Normal,      3, float3)
GENERICVERTEX_SEMANTIC_DEF(Normal,      4, float4)
GENERICVERTEX_SEMANTIC_DEF(Tangent,     3, float3)
GENERICVERTEX_SEMANTIC_DEF(Tangent,     4, float4)
GENERICVERTEX_SEMANTIC_DEF(Binormal,    3, float3)
//----------------------------------------------------------------------------
#undef GENERICVERTEX_SEMANTIC_DEF
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace PPE
