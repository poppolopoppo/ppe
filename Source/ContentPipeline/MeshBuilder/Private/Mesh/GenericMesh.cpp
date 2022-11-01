#include "stdafx.h"

#include "Mesh/GenericMesh.h"

#include "Mesh/GenericMeshHelpers.h"

#include "RHI/EnumHelpers.h"
#include "RHI/VertexInputState.h"

#include "Container/Hash.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformMemory.h"
#include "Maths/ScalarBoundingBox.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
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
FGenericVertexData& FGenericMesh::VertexData(const RHI::FVertexID& semantic) const NOEXCEPT {
    FGenericVertexData* const vertices = VertexDataIFP(semantic);
    AssertRelease(nullptr != vertices);
    return (*vertices);
}
//----------------------------------------------------------------------------
FGenericVertexData* FGenericMesh::VertexDataIFP(const RHI::FVertexID& semantic) const NOEXCEPT {
    Assert(semantic.Valid());

    auto it = _vertices.find(semantic);
    if (_vertices.end() != it)
        return it->second.get();

    return nullptr;
}
//----------------------------------------------------------------------------
FGenericVertexData& FGenericMesh::AddVertexData(const RHI::FVertexID& semantic, RHI::EVertexFormat format) {
    Assert_NoAssume(nullptr == VertexDataIFP(semantic, format));

    UGenericVertexData vertex = MakeUnique<FGenericVertexData>(this, semantic, format);
    FGenericVertexData& const result = *vertex;

    _vertices.Emplace_AssertUnique(RHI::FVertexID(semantic), std::move(vertex));
    return result;
}
//----------------------------------------------------------------------------
FGenericVertexData& FGenericMesh::GetOrAddVertexData(const RHI::FVertexID& semantic, RHI::EVertexFormat format) {
    if (FGenericVertexData* result = VertexDataIFP(semantic)) {
        Assert_NoAssume(result->Format() == format);
        return *result;
    }

    return AddVertexData(semantic, format);
}
//----------------------------------------------------------------------------
void FGenericMesh::RemoveVertexData(const FGenericVertexData* data) {
    Assert(data);

    RemoveVertexData(data->Semantic());
}
//----------------------------------------------------------------------------
void FGenericMesh::RemoveVertexData(const RHI::FVertexID& semantic) {
    Assert(semantic.Valid());

    VerifyRelease(_vertices.Erase(semantic));
}
//----------------------------------------------------------------------------
bool FGenericMesh::AreVertexEquals(size_t v0, size_t v1) const {
    Assert(v0 < _vertexCount);
    Assert(v1 < _vertexCount);
    if (v0 == v1)
        return true;

    for (const UGenericVertexData& vertices : _vertices.Values()) {
        Assert_NoAssume(_vertexCount == vertices->VertexCount());
        const size_t strideInBytes = vertices->StrideInBytes();

        const FRawMemoryConst rawData = vertices->MakeView();
        const int cmp = FPlatformMemory::Memcmp(
            rawData.SubRange(v0 * strideInBytes, strideInBytes).data(),
            rawData.SubRange(v1 * strideInBytes, strideInBytes).data(),
            strideInBytes );

        if (cmp)
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
hash_t FGenericMesh::VertexHash(size_t v, hash_t seed/* = 0 */) const {
    Assert(v < _vertexCount);

    hash_t result = seed;

    for (const UGenericVertexData& vertices : _vertices.Values()) {
        Assert(_vertexCount == vertices->VertexCount());
        const FRawMemoryConst& rawData = vertices->MakeView();
        const size_t strideInBytes = vertices->StrideInBytes();

        hash_combine(result, vertices->Semantic(), vertices->Format(), vertices->VertexCount());

        const auto subPart = rawData.SubRange(v * strideInBytes, strideInBytes);
        result = FPlatformHash::HashMem(result, subPart.data(), subPart.SizeInBytes());
    }

    return result;
}
//----------------------------------------------------------------------------
void FGenericMesh::VertexCopy(size_t dst, size_t src) {
    Assert(dst < _vertexCount);
    Assert(src < _vertexCount);
    if (src == dst)
        return;

    for (const UGenericVertexData& vertices : _vertices.Values()) {
        Assert(_vertexCount == vertices->VertexCount());
        const size_t strideInBytes = vertices->StrideInBytes();
        const FRawMemory rawData = vertices->MakeView();

        FPlatformMemory::Memcpy(
            rawData.SubRange(dst * strideInBytes, strideInBytes).data(),
            rawData.SubRange(src * strideInBytes, strideInBytes).data(),
            strideInBytes );
    }
}
//----------------------------------------------------------------------------
void FGenericMesh::VertexSwap(size_t v0, size_t v1) {
    Assert(v0 < _vertexCount);
    Assert(v1 < _vertexCount);
    if (v0 == v1)
        return;

    for (const UGenericVertexData& vertices : _vertices.Values()) {
        Assert(_vertexCount == vertices->VertexCount());
        const size_t strideInBytes = vertices->StrideInBytes();
        const FRawMemory rawData = vertices->MakeView();

        FPlatformMemory::Memswap(
            rawData.SubRange(v0 * strideInBytes, strideInBytes).data(),
            rawData.SubRange(v1 * strideInBytes, strideInBytes).data(),
            strideInBytes );
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
        _indices.resize(indexCount, keepData);
    }
    if (_vertexCount != vertexCount) {
        _vertexCount = vertexCount;
        for (const UGenericVertexData& vertices : _vertices.Values())
            vertices->Resize(vertexCount, keepData);
    }

#if USE_PPE_ASSERT
    Assert_NoAssume(Validate());
#endif
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
        for (const UGenericVertexData& vertices : _vertices.Values())
            vertices->Reserve(vertexCount);
    }
}
//----------------------------------------------------------------------------
void FGenericMesh::CleanAndOptimize(size_t index) {
    if (!Position3f_IFP(index) && !Position4f_IFP(index))
        return;

    const size_t duplicated = MergeDuplicateVertices(*this);
    LOG(MeshBuilder, Info, L"removed {0}/{1} duplicate vertices = {2:f2}%", duplicated, _vertexCount, duplicated * 100.f / _vertexCount);

    const size_t unused = RemoveUnusedVertices(*this);
    LOG(MeshBuilder, Info, L"removed {0}/{1} unused vertices = {2:f2}%", unused, _vertexCount + unused, unused * 100.f / (_vertexCount + unused));

    const FAabb3f bounds = ComputeBounds(*this, index);
    const float minDistance = bounds.Extents().MinComponent() * 0.001f; // 0.01%
    const float minArea = minDistance * minDistance;

#if 0 // May merge needed vertices for interpolation : can't be in the default path
    const size_t close = MergeCloseVertices(*this, index, minDistance);
    LOG(MeshBuilder, Info, L"removed {0}/{1} close vertices (epsilon = {2}) = {3:f2}%", close, _vertexCount, minDistance, close * 100.f / _vertexCount);
#endif

    const size_t zero = RemoveZeroAreaTriangles(*this, index, minArea);
    LOG(MeshBuilder, Info, L"removed {0}/{1} zero area triangle indices (epsilon = {2}) = {3:f2}%", zero, _indexCount + zero, minArea, zero * 100.f / (_indexCount + zero));

    const size_t unused2 = RemoveUnusedVertices(*this);
    LOG(MeshBuilder, Info, L"removed {0}/{1} unused vertices = {2:f2}%", unused2, _vertexCount + unused2, unused2 * 100.f / (_vertexCount + unused2));

    const float VACMR_Before = VertexAverageCacheMissRate(*this);

    OptimizeIndicesAndVerticesOrder(*this);

    const float VACMR_After = VertexAverageCacheMissRate(*this);
    LOG(MeshBuilder, Info, L"optimized vertex average cache miss rate from {0:f3} to {1:f3} = {2:f2}%", VACMR_Before, VACMR_After, VACMR_After * 100.f / VACMR_Before);
}
//----------------------------------------------------------------------------
bool FGenericMesh::ExportIndices(const TMemoryView<u16>& dst) const {
    return ExportIndices(RHI::EIndexFormat::UShort, dst.Cast<u8>());
}
//----------------------------------------------------------------------------
bool FGenericMesh::ExportIndices(const TMemoryView<u32>& dst) const {
    return ExportIndices(RHI::EIndexFormat::UInt, dst.Cast<u8>());
}
//----------------------------------------------------------------------------
bool FGenericMesh::ExportIndices(RHI::EIndexFormat format, const FRawMemory& dst) const  {
    Assert_NoAssume(Validate());
    Assert_NoAssume(dst.SizeInBytes() == RHI::EIndexFormat_SizeOf(format) * _indexCount);

    switch (format) {
    case RHI::EIndexFormat::UInt:
        FPlatformMemory::Memcpy(dst.data(), Indices().data(), dst.SizeInBytes());
        break;

    case RHI::EIndexFormat::UShort:
        if (_vertexCount > TNumericLimits<u16>::MaxValue())
            return false;

        const u32* psrc = Indices().data();
        u16* pdst = dst.Cast<u16>().data();
        u16* const pend = pdst + _indexCount;

        while (pend != pdst)
            *pdst++ = checked_cast<u16>(*psrc++);
        break;

    default:
        AssertNotImplemented();
        return false;
    }

    return true;
}
//----------------------------------------------------------------------------
bool FGenericMesh::ExportVertices(const RHI::FVertexInputState& vdecl, const RHI::FVertexBufferID& vertexBuffer, const FRawMemory& dst) const {
    Assert(vertexBuffer.Valid());

    const RHI::FVertexBufferBinding& bufferBinding = vdecl.BufferBindings[vertexBuffer];
    Assert_NoAssume(dst.SizeInBytes() == bufferBinding.Stride * _vertexCount);

    for (const TPair<const RHI::FVertexID, RHI::FVertexInput>& it : vdecl.Vertices) {
        if (it.second.BufferBinding != bufferBinding.Index)
            continue;

        const FGenericVertexData* const pVertexData = VertexDataIFP(it.first);
        if (not pVertexData)
            return false;

        const size_t vertexStride = pVertexData->StrideInBytes();
        Assert_NoAssume(pVertexData->VertexCount() == _vertexCount);
        Assert_NoAssume(pVertexData->SizeInBytes() == vertexStride * _vertexCount);

        // same format => trivial copy, always succeed
        if (pVertexData->Format() == it.second.Format) {
            forrange(v, 0, _vertexCount) {
                const FRawMemoryConst src = pVertexData->Vertex(v);
                src.CopyTo(dst.SubRange(v * bufferBinding.Stride + it.second.Offset, vertexStride));
            }
        }
        // different format => try promotion, can fail
        else if (RHI::FVertexFormatPromote promote = RHI::EVertexFormat_Promote(it.second.Format, pVertexData->Format())) {
            forrange(v, 0, _vertexCount) {
                const FRawMemoryConst src = pVertexData->Vertex(v);
                if (not promote(dst.SubRange(v * bufferBinding.Stride + it.second.Offset, vertexStride), src))
                    return false;
            }
        }
        else {
            return false;
        }
    }

    return true;
}
//----------------------------------------------------------------------------
bool FGenericMesh::Validate() const NOEXCEPT {
    if (Indices().size() != _indexCount)
        return false;

    for (u32 index : Indices()) {
        if (index >= _vertexCount)
            return false;
    }

    for (const UGenericVertexData& vertices : _vertices.Values()) {
        if (vertices->VertexCount() != _vertexCount)
            return false;

        if (vertices->SizeInBytes() / vertices->StrideInBytes() != _vertexCount)
            return false;
    }
}
//----------------------------------------------------------------------------
void FGenericMesh::Clear() {
    Assert_NoAssume(Validate());

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
FGenericVertexData::FGenericVertexData(FGenericMesh& owner, const RHI::FVertexID& semantic, RHI::EVertexFormat format) NOEXCEPT
:   _owner(owner), _semantic(semantic), _format(format), _vertexCount(0) {
    Assert(_format != Default);
}
//----------------------------------------------------------------------------
FGenericVertexData::~FGenericVertexData() {}
//----------------------------------------------------------------------------
size_t FGenericVertexData::StrideInBytes() const NOEXCEPT {
    return RHI::EVertexFormat_SizeOf(_format);
}
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
void FGenericVertexData::CopyVertex(size_t dst, size_t src) {
    Assert(dst < _vertexCount);
    Assert(src < _vertexCount);

    if (dst == src)
        return;

    const size_t strideInBytes = StrideInBytes();
    const FRawMemory storage = _stream.MakeView();

    FPlatformMemory::MemcpyLarge(&storage[dst * strideInBytes], &storage[src * strideInBytes], strideInBytes);
}
//----------------------------------------------------------------------------
void FGenericVertexData::ReadVertex(size_t v, const FRawMemory& dst) const {
    Vertex(v).CopyTo(dst);
}
//----------------------------------------------------------------------------
void FGenericVertexData::WriteVertex(size_t v, const FRawMemoryConst& src) {
    src.CopyTo(Vertex(v));
}
//----------------------------------------------------------------------------
FRawMemory FGenericVertexData::SubRange(size_t start, size_t count) {
    Assert(start + count <= _vertexCount);

    const size_t strideInBytes = StrideInBytes();
    return _stream.MakeView().SubRange(start * strideInBytes, count * strideInBytes);
}
//----------------------------------------------------------------------------
FRawMemoryConst FGenericVertexData::SubRange(size_t start, size_t count) const {
    Assert(start + count <= _vertexCount);

    const size_t strideInBytes = StrideInBytes();
    return _stream.MakeView().SubRange(start * strideInBytes, count * strideInBytes);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------

RHI::VertexID::
#define GENERICVERTEX_SEMANTIC_DEF(_NAME, _DIM, _TYPE) \
    TGenericVertexSubPart< _TYPE > FGenericMesh::CONCAT(CONCAT(_NAME, _DIM), f)(size_t index) { \
        return TGenericVertexSubPart< _TYPE >( GetOrAddVertexData(RHI::VertexID::_NAME(index), TGenericVertexSubPart< _TYPE >::Format) ); \
    } \
    TGenericVertexSubPart< _TYPE > FGenericMesh::CONCAT(CONCAT(_NAME, _DIM), f_IFP)(size_t index) const { \
        return TGenericVertexSubPart< _TYPE >( VertexDataIFP(RHI::VertexID::_NAME(index), TGenericVertexSubPart< _TYPE >::Format) ); \
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
} //!namespace ContentPipeline
} //!namespace PPE
