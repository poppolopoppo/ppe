// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Mesh/GenericMeshHelpers.h"

#include "Mesh/GenericMesh.h"

#include "HAL/PlatformMemory.h"

#include "Diagnostic/Logger.h"

#include "Container/BitSet.h"
#include "Container/Hash.h"
#include "Container/HashSet.h"
#include "Container/Stack.h"
#include "Container/Vector.h"

#include "Maths/MathHelpers.h"
#include "Maths/PNTriangle.h"
#include "Maths/ScalarBoundingBox.h"
#include "Maths/ScalarMatrix.h"
#include "Maths/ScalarMatrixHelpers.h"
#include "Maths/ScalarVector.h"
#include "Maths/ScalarVectorHelpers.h"
#include "Maths/Transform.h"
#include "Maths/Quaternion.h"
#include "Maths/QuaternionHelpers.h"
#include "Memory/UniqueView.h"

#include "Thread/Task/TaskHelpers.h"

#include <algorithm>

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
// http://fgiesen.wordpress.com/2013/12/14/simple-lossless-index-buffer-compression/
// https://github.com/rygorous/simple-ib-compress/blob/master/main.cpp
struct FVertexCache {
    static const u32 Size = 32;
    static const u32 MaxValence = 15;

    struct FEntry {
        u32 CachePos;           // its position in the cache (UINT32_MAX if not in)
        u32 Score;              // its score (higher=better)
        u32 TrianglesLeft;      // # of not-yet-used tris
        u32 *TriangleList;      // list of triangle indices
        u32 OpenPos;            // position in "open vertex" list
    };

    struct FTriangle {
        u32 Score;              // current score (UINT32_MAX if already done)
        u32 Indices[3];         // vertex indices
    };
};
//----------------------------------------------------------------------------
static float3 ComputeNormal_(const float3& p0, const float3& p1, const float3& p2) {
#if 0
    return Cross(Normalize(p1 - p0), Normalize(p2 - p1));
#elif 0
    const float3 p10 = SafeNormalize(p1 - p0);
    const float3 p21 = SafeNormalize(p2 - p1);
    const float3 p02 = SafeNormalize(p0 - p2);

    const float3 c012 = Cross(p10, p02);
    const float3 c102 = Cross(p10, p21);
    const float3 c201 = Cross(p02, p21);

    const float d012 = LengthSq(c012);
    const float d102 = LengthSq(c102);
    const float d201 = LengthSq(c201);

    return (d012 > d102)
        ? (d012 > d201 ? c012 : c201)
        : (d102 > d201 ? c102 : c201);
#else
    // Newell's method: sum cross product of all consecutive edges
    // https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal

    auto crossEdge = [](const double3& a, const double3& b) -> double3 {
        return (double3(a - b).yzx * double3(a + b).zxy);
    };

    const double3 d0{ p0 };
    const double3 d1{ p1 };
    const double3 d2{ p2 };

    const double3 normal = crossEdge(d0, d1) + crossEdge(d1, d2) + crossEdge(d2, d0);
    return float3(normal);
#endif
}
//----------------------------------------------------------------------------
static void ComputeTriangleBasis_(
    float3 *pTangent,
    float3 *pBinormal,
    const float3& pos0, const float3& pos1, const float3& pos2,
    const float2& tex0, const float2& tex1, const float2& tex2 ) {
    // triangle 0, 1, 2
    const float3 pos10 = pos1 - pos0;
    const float3 pos20 = pos2 - pos0;

    const float2 tex10 = tex1 - tex0;
    const float2 tex20 = tex2 - tex0;

    const float f012 = (tex10.x * tex20.y -
                        tex10.y * tex20.x );

    // triangle 1, 0, 2
    const float3 pos01 = pos0 - pos1;
    const float3 pos21 = pos2 - pos1;

    const float2 tex01 = tex0 - tex1;
    const float2 tex21 = tex2 - tex1;

    const float f102 = (tex01.x * tex21.y -
                        tex01.y * tex21.x );

    // triangle 2, 0, 1
    const float3 pos02 = pos0 - pos2;
    const float3 pos12 = pos1 - pos2;

    const float2 tex02 = tex0 - tex2;
    const float2 tex12 = tex1 - tex2;

    const float f201 = (tex02.x * tex12.y -
                        tex02.y * tex12.x );

    if (Abs(f012) > Abs(f102)) {
        if (Abs(f012) > Abs(f201)) {
            const float f = (Abs(f012) < EpsilonSQ) ? 1.0f : 1.0f/f012;
            *pTangent = ((pos10 * tex20.y) - (pos20 * tex10.y)) * f;
            *pBinormal = ((pos20 * tex10.x) - (pos10 * tex20.x)) * f;
        }
        else {
            const float f = (Abs(f201) < EpsilonSQ) ? 1.0f : 1.0f/f201;
            *pTangent = ((pos01 * tex21.y) - (pos21 * tex01.y)) * f;
            *pBinormal = ((pos21 * tex01.x) - (pos01 * tex21.x)) * f;
        }
    }
    else {
        if (Abs(f102) > Abs(f201)) {
            const float f = (Abs(f102) < EpsilonSQ) ? 1.0f : 1.0f/f102;
            *pTangent = ((pos01 * tex21.y) - (pos21 * tex01.y)) * f;
            *pBinormal = ((pos21 * tex01.x) - (pos01 * tex21.x)) * f;
        }
        else {
            const float f = (Abs(f201) < EpsilonSQ) ? 1.0f : 1.0f/f201;
            *pTangent = ((pos02 * tex12.y) - (pos12 * tex02.y)) * f;
            *pBinormal = ((pos12 * tex02.x) - (pos02 * tex12.x)) * f;
        }
    }
}
//----------------------------------------------------------------------------
}//!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
template <size_t _Dim>
TScalarBoundingBox<float, _Dim> ComputeSubPartBounds_(
    const TGenericVertexSubPart<TScalarVector<float, _Dim>>& subPart) {
    TScalarBoundingBox<float, _Dim> bounds;
    bounds.AddRange(subPart.MakeView().begin(), subPart.MakeView().end());
    return bounds;
}
} //!namespace
//----------------------------------------------------------------------------
FAabb3f ComputeBounds(const FGenericMesh& mesh, size_t index) {
    FAabb3f bounds;

    const TGenericVertexSubPart<float3> sp_position3f = mesh.Position3f_IFP(index);
    const TGenericVertexSubPart<float4> sp_position4f = mesh.Position4f_IFP(index);
    if (!sp_position3f && !sp_position4f) {
        AssertNotReached();
        return bounds;
    }

    if (sp_position3f)
        bounds = ComputeSubPartBounds(sp_position3f);
    else
        bounds = ComputeSubPartBounds(sp_position4f).Shuffle<0,1,2>();

    return bounds;
}
//----------------------------------------------------------------------------
FAabb3f ComputeSubPartBounds(const TGenericVertexSubPart<float3>& subPart) {
    return ComputeSubPartBounds_<3>(subPart);
}
//----------------------------------------------------------------------------
FAabb4f ComputeSubPartBounds(const TGenericVertexSubPart<float4>& subPart) {
    return ComputeSubPartBounds_<4>(subPart);
}
//----------------------------------------------------------------------------
namespace {
static void ComputeNormals_(
    const FGenericMesh& mesh,
    const FPositions3f& sp_position3f,
    const FPositions4f& sp_position4f,
    const FNormals3f& sp_normal3f ) {
    Assert((!sp_position3f) ^ (!sp_position4f));

    const u32 indexCount = mesh.IndexCount();
    const u32 vertexCount = mesh.VertexCount();

    Assert(!sp_position3f || sp_position3f.size() == vertexCount);
    Assert(!sp_position4f || sp_position4f.size() == vertexCount);

    const TMemoryView<const u32> indices = mesh.Indices();
    const TMemoryView<float3> normals3f = sp_normal3f.Resize(vertexCount);

    const float3 zero = float3::Zero;
    for (float3& n : normals3f)
        n = zero;

    if (sp_position4f) {
        Assert(!sp_position3f);
        const TMemoryView<const float4> positions4f = sp_position4f.MakeView();
        Assert(positions4f.size() == mesh.VertexCount());

        for (u32 t = 0; t < indexCount; t += 3) {
            const u32 i0 = indices[t + 0];
            const u32 i1 = indices[t + 1];
            const u32 i2 = indices[t + 2];

            const float3 n = ComputeNormal_(
                positions4f[i0].xyz,
                positions4f[i1].xyz,
                positions4f[i2].xyz );

            normals3f[i0] += n;
            normals3f[i1] += n;
            normals3f[i2] += n;
        }
    }
    else {
        Assert(!sp_position4f);
        const TMemoryView<const float3> positions3f = sp_position3f.MakeView();

        for (u32 t = 0; t < indexCount; t += 3) {
            const u32 i0 = indices[t + 0];
            const u32 i1 = indices[t + 1];
            const u32 i2 = indices[t + 2];

            const float3 n = ComputeNormal_(
                positions3f[i0],
                positions3f[i1],
                positions3f[i2] );

            normals3f[i0] += n;
            normals3f[i1] += n;
            normals3f[i2] += n;
        }
    }

    for (float3& n : normals3f)
        n = Normalize(n);
}
} //!namespace
//----------------------------------------------------------------------------
bool ComputeNormals(FGenericMesh& mesh, size_t index) {
    const TGenericVertexSubPart<float3> sp_position3f = mesh.Position3f_IFP(index);
    const TGenericVertexSubPart<float4> sp_position4f = mesh.Position4f_IFP(index);
    if (!sp_position3f && !sp_position4f)
        return false;

    TGenericVertexSubPart<float3> sp_normal3f = mesh.Normal3f(index);
    Assert(sp_normal3f);

    ComputeNormals_(mesh, sp_position3f, sp_position4f, sp_normal3f);

    return true;
}
//----------------------------------------------------------------------------
void ComputeNormals(const FGenericMesh& mesh, const FPositions3f& positions, const FNormals3f& normals) {
    Assert(positions);
    Assert(normals);

    ComputeNormals_(mesh, positions, FPositions4f(), normals);
}
//----------------------------------------------------------------------------
void ComputeNormals(const FGenericMesh& mesh, const FPositions4f& positions, const FNormals3f& normals) {
    Assert(positions);
    Assert(normals);

    ComputeNormals_(mesh, FPositions3f(), positions, normals);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
static void ComputeTangentSpace_(
    const FGenericMesh& mesh,
    const FPositions3f& sp_position3f,
    const FPositions4f& sp_position4f,
    const FTexcoords2f& sp_texcoord2f,
    const FNormals3f& sp_normal3f,
    const FTangents3f& sp_tangent3f,
    const FTangents4f& sp_tangent4f,
    const FBinormals3f& sp_binormal3f ) {
    Assert((!sp_position3f) ^ (!sp_position4f));
    Assert((!sp_tangent4f) ^ (!sp_tangent3f));

    const u32 indexCount = mesh.IndexCount();
    const u32 vertexCount = mesh.VertexCount();

    Assert(!sp_position3f || sp_position3f.size() == vertexCount);
    Assert(!sp_position4f || sp_position4f.size() == vertexCount);
    Assert(sp_texcoord2f.size() == vertexCount);
    Assert(sp_normal3f.size() == vertexCount);

    const TMemoryView<const u32> indices = mesh.Indices();

    const TMemoryView<const float3> positions3f = sp_position3f.MakeView();
    const TMemoryView<const float4> positions4f = sp_position4f.MakeView();
    const TMemoryView<const float2> texcoords2f = sp_texcoord2f.MakeView();
    const TMemoryView<const float3> normals3f = sp_normal3f.MakeView();

    VECTOR(MeshBuilder, float3) tmpTangents3f;
    VECTOR(MeshBuilder, float3) tmpBinormals3f;

    TMemoryView<float3> tangents3f;
    TMemoryView<float4> tangents4f;
    TMemoryView<float3> binormals3f;
    if (sp_tangent4f) {
        tmpTangents3f.resize_AssumeEmpty(vertexCount);
        tmpBinormals3f.reserve_AssumeEmpty(vertexCount);

        tangents3f = tmpTangents3f.MakeView();
        binormals3f = tmpBinormals3f.MakeView();

        tangents4f = sp_tangent4f.Resize(vertexCount);
    }
    else {
        tangents3f = sp_tangent3f.Resize(vertexCount);
        binormals3f = sp_binormal3f.Resize(vertexCount);
    }

    const float3 zero = float3::Zero;
    for (u32 i = 0; i < vertexCount; ++i)
        tangents3f[i] = binormals3f[i] = zero;

    for (u32 i = 0; i < indexCount; i += 3) {
        const u32 i0 = indices[i + 0];
        const u32 i1 = indices[i + 1];
        const u32 i2 = indices[i + 2];

        float3 tangent;
        float3 binormal;

        if (sp_position3f) {
            ComputeTriangleBasis_(
                &tangent, &binormal,
                positions3f[i0], positions3f[i1], positions3f[i2],
                texcoords2f[i0], texcoords2f[i1], texcoords2f[i2] );
        }
        else {
            ComputeTriangleBasis_(
                &tangent, &binormal,
                positions4f[i0].xyz, positions4f[i1].xyz, positions4f[i2].xyz,
                texcoords2f[i0], texcoords2f[i1], texcoords2f[i2] );
        }

        tangents3f[i0] += tangent;
        tangents3f[i1] += tangent;
        tangents3f[i2] += tangent;

        binormals3f[i0] += binormal;
        binormals3f[i1] += binormal;
        binormals3f[i2] += binormal;
    }

    ParallelFor(0, vertexCount, [&](size_t i) {
        float3& tangent = tangents3f[i];
        float3& binormal = binormals3f[i];

        const float3& normal = normals3f[i];
        if (float3(0) == tangent && float3(0) == binormal) {
            tangent = float3(1, 0, 0);
            binormal = Cross(normal, tangent);
        }
        else if (float3(0) == tangent) {
            Assert(float3(0) != binormal);
            binormal = SafeNormalize(binormal);
            tangent = Cross(normal, binormal);
        }
        else if (float3(0) == binormal) {
            Assert(float3(0) != tangent);
            tangent = SafeNormalize(tangent);
            binormal = Cross(normal, tangent);
        }
        else {
            tangent = SafeNormalize(tangent);
            binormal = SafeNormalize(binormal);
        }

        //Gram-Schmidt orthogonalization
        tangent = SafeNormalize(tangent - normal * Dot(normal, tangent));

        //Right handed TBN space ?
        const bool leftHanded = (Dot(Cross(tangent, binormal), normal) < 0);

        //Handedness packing IFP
        if (leftHanded) {
            binormal = -binormal;

            if (sp_tangent4f)
                tangents4f[i] = float4(tangent, leftHanded ? 1.0f : 0.0f);
        }
    });
}
} //!namespace
//----------------------------------------------------------------------------
bool ComputeTangentSpace(FGenericMesh& mesh, size_t index, bool packHandedness/* = false */) {
    const TGenericVertexSubPart<float3> sp_position3f = mesh.Position3f_IFP(index);
    const TGenericVertexSubPart<float4> sp_position4f = mesh.Position4f_IFP(index);
    if (!sp_position3f && !sp_position4f)
        return false;

    Assert(!sp_position3f ^ !sp_position4f);

    const TGenericVertexSubPart<float2> sp_texcoord2f = mesh.Texcoord2f_IFP(index);
    if (!sp_texcoord2f)
        return false;

    const TGenericVertexSubPart<float3> sp_normal3f = mesh.Normal3f_IFP(index);
    if (!sp_normal3f)
        return false;

    TGenericVertexSubPart<float3> sp_tangent3f;
    TGenericVertexSubPart<float4> sp_tangent4f;
    TGenericVertexSubPart<float3> sp_binormal3f;

    if (packHandedness) {
        sp_tangent4f = mesh.Tangent4f(index);

        Assert(sp_tangent4f);
    }
    else {
        sp_tangent3f = mesh.Tangent3f(index);
        sp_binormal3f = mesh.Binormal3f(index);

        Assert(sp_tangent3f);
        Assert(sp_binormal3f);
    }

    ComputeTangentSpace_(mesh, sp_position3f, sp_position4f, sp_texcoord2f, sp_normal3f, sp_tangent3f, sp_tangent4f, sp_binormal3f);

    return true;
}
//----------------------------------------------------------------------------
void ComputeTangentSpace(const FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords2f& uv, const FNormals3f& normals, const FTangents3f& tangents, const FBinormals3f& binormals) {
    ComputeTangentSpace_(mesh, positions, FPositions4f(), uv, normals, tangents, FTangents4f(), binormals);
}
//----------------------------------------------------------------------------
void ComputeTangentSpace(const FGenericMesh& mesh, const FPositions4f& positions, const FTexcoords2f& uv, const FNormals3f& normals, const FTangents3f& tangents, const FBinormals3f& binormals) {
    ComputeTangentSpace_(mesh, FPositions3f(), positions, uv, normals, tangents, FTangents4f(), binormals);
}
//----------------------------------------------------------------------------
void ComputeTangentSpace(const FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords2f& uv, const FNormals3f& normals, const FTangents4f& tangents) {
    ComputeTangentSpace_(mesh, positions, FPositions4f(), uv, normals, FTangents3f(), tangents, FBinormals3f());
}
//----------------------------------------------------------------------------
void ComputeTangentSpace(const FGenericMesh& mesh, const FPositions4f& positions, const FTexcoords2f& uv, const FNormals3f& normals, const FTangents4f& tangents) {
    ComputeTangentSpace_(mesh, FPositions3f(), positions, uv, normals, FTangents3f(), tangents, FBinormals3f());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool TangentSpaceToQuaternion(FGenericMesh& mesh, size_t index, bool removeTBN/* = true */) {
    const TGenericVertexSubPart<float3> sp_normal3f = mesh.Normal3f_IFP(index);
    if (!sp_normal3f)
        return false;

    if (const TGenericVertexSubPart<float4> sp_tangent4f = mesh.Tangent4f_IFP(index)) {
        const UGenericVertexData oldNormals = mesh.StealVertexData(sp_normal3f);
        TangentSpaceToQuaternion(mesh, sp_normal3f, sp_tangent4f, mesh.Normal4f(index));

        if (removeTBN) {
            mesh.RemoveSubPart(sp_tangent4f);
        }
    }
    else if (const TGenericVertexSubPart<float3> sp_tangent3f = mesh.Tangent3f_IFP(index)) {
        const TGenericVertexSubPart<float3> sp_binormal3f = mesh.Binormal3f_IFP(index);
        if (!sp_binormal3f)
            return false;

        const UGenericVertexData oldNormals = mesh.StealVertexData(sp_normal3f);
        TangentSpaceToQuaternion(mesh, sp_normal3f, sp_binormal3f, sp_tangent3f, mesh.Normal4f(index));

        if (removeTBN) {
            mesh.RemoveSubPart(sp_binormal3f);
            mesh.RemoveSubPart(sp_tangent3f);
        }
    }
    else {
        return false;
    }

    return true;
}
//----------------------------------------------------------------------------
void TangentSpaceToQuaternion(const FGenericMesh& mesh, const FNormals3f& normals, const FBinormals3f& binormals, const FTangents3f& tangents, const FNormals4f& quaternions) {
    const u32 vertexCount = mesh.VertexCount();
    quaternions.Resize(vertexCount, false);

    const TMemoryView<const float3> t = tangents.MakeView();
    const TMemoryView<const float3> b = binormals.MakeView();
    const TMemoryView<const float3> n = normals.MakeView();

    TMemoryView<float4> q = quaternions.MakeView();

    ParallelFor(0, vertexCount, [&](size_t v) {
        q[v] = PPE::TangentSpaceToQuaternion(t[v], b[v], n[v]).data;
    });
}
//----------------------------------------------------------------------------
void TangentSpaceToQuaternion(const FGenericMesh& mesh, const FNormals3f& normals, const FTangents4f& tangentsWithHandedness, const FNormals4f& quaternions) {
    const u32 vertexCount = mesh.VertexCount();

    const TMemoryView<const float4> t = tangentsWithHandedness.MakeView();
    const TMemoryView<const float3> n = normals.MakeView();

    TMemoryView<float4> q = quaternions.MakeView();

    ParallelFor(0, vertexCount, [&](size_t v) {
        const float4& tangent = t[v];
        const float3& normal = n[v];
        const float3 binormal = Normalize(Cross(tangent.xyz, normal)) *
            (tangent.w > 0 ? -1.f : 1.f);

        q[v] = PPE::TangentSpaceToQuaternion(tangent.xyz, binormal, normal).data;
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t MergeCloseVertices(FGenericMesh& mesh, size_t index, float minDistance/* = Epsilon */) {
    const u32 vertexCount = mesh.VertexCount();
    if (0 == vertexCount)
        return 0;

    const TGenericVertexSubPart<float3> sp_position3f = mesh.Position3f_IFP(index);
    const TGenericVertexSubPart<float4> sp_position4f = mesh.Position4f_IFP(index);
    PPE_LOG_CHECK(MeshBuilder, sp_position3f || sp_position4f);

    STACKLOCAL_POD_ARRAY(u32, tmp, vertexCount * 2);
    const TMemoryView<u32> sorted = tmp.SubRange(0 * vertexCount, vertexCount);
    const TMemoryView<u32> reindexation = tmp.SubRange(1 * vertexCount, vertexCount);

    forrange(i, 0, vertexCount)
        sorted[i] = u32(i);

    const TMemoryView<const float3> positions3f = sp_position3f.MakeView();
    const TMemoryView<const float4> positions4f = sp_position4f.MakeView();

    if (sp_position3f)
        std::sort(sorted.begin(), sorted.end(), [&positions3f](u32 lhs, u32 rhs) {
            return (positions3f[lhs].x < positions3f[rhs].x);
        });
    else
        std::sort(sorted.begin(), sorted.end(), [&positions4f](u32 lhs, u32 rhs) {
            return (positions4f[lhs].x < positions4f[rhs].x);
        });

    const float minDistanceSq = Sqr(minDistance);
    for (u32 a = 0; a < vertexCount; ++a) {
        const u32 ia = sorted[a];
        const float3 pa = (sp_position3f ? positions3f[ia] : float3(positions4f[ia].xyz));

        reindexation[ia] = ia;

        u32 b = a;
        while (b--) {
            const u32 ib = sorted[b];
            const float3 pb = (sp_position3f ? positions3f[ib] : float3(positions4f[ib].xyz));

            if (pb.x < pa.x - minDistance)
                break;

            if (DistanceSq(pa, pb) < minDistanceSq) {
                reindexation[ib] = ia;
            }
        }
    }

    for (u32& vertexIndex : mesh.Indices())
        vertexIndex = reindexation[vertexIndex];

    u32 mergedCount = 0;
    forrange(v, 0, vertexCount)
        if (reindexation[v] != v)
            mergedCount++;

    return mergedCount;
}
//----------------------------------------------------------------------------
size_t MergeDuplicateVertices(FGenericMesh& mesh) {
    const u32 vertexCount = mesh.VertexCount();
    if (0 == vertexCount)
        return 0;

    // Hash all vertices
    STACKLOCAL_POD_ARRAY(hash_t, hashes, vertexCount);
    std::fill(hashes.begin(), hashes.end(), PPE_HASH_VALUE_SEED);

    // Compute each vertex hash value subpart by subpart
    ParallelForEachRef(mesh.Vertices().begin(), mesh.Vertices().end(), [&](const UGenericVertexData& subpart) {
        Assert(subpart->VertexCount() == vertexCount);

        const TMemoryView<const u8> rawData = subpart->MakeView();
        const size_t sizeInBytes = EVertexFormat_SizeOf(subpart->Format());
        const size_t strideInBytes = subpart->StrideInBytes();
        Assert_NoAssume(sizeInBytes <= strideInBytes);

        forrange(v, 0, vertexCount) {
            const auto block = rawData.SubRange(v * strideInBytes, strideInBytes).CutBefore(sizeInBytes);
            hash_combine(hashes[v], hash_view(block));
        }
    });

    // Construct a hash set with all vertices
    struct TVertexIndexHasher_ {
        const TMemoryView<const hash_t> Hashes;
        hash_t operator ()(u32 v) const {
            return (Hashes[v]);
        }
    };
    struct TVertexIndexEqual_ {
        const TMemoryView<const hash_t> Hashes;
        const FGenericMesh* pMesh;
        bool operator ()(u32 lhs, u32 rhs) const {
            return (Hashes[lhs] == Hashes[rhs] &&
                    pMesh->AreVertexEquals(lhs, rhs) );
        }
    };
    THashSet<u32, TVertexIndexHasher_, TVertexIndexEqual_, ALLOCATOR(MeshBuilder)>
        verticesSet(TVertexIndexHasher_{ hashes }, TVertexIndexEqual_{ hashes, &mesh });
    verticesSet.reserve(vertexCount);

    // Merge and re-index all vertices
    STACKLOCAL_POD_ARRAY(u32, reindexation, vertexCount);
    forrange(v, 0, vertexCount) {
        const auto it = verticesSet.insert(v);
        Assert(it.second || *it.first < v);
        reindexation[v] = *it.first;
    }

    // No collisions <=> no duplicates
    if (verticesSet.size() == vertexCount)
        return 0;
    Assert(verticesSet.size() < vertexCount);

    // Remap all indices to new merged indices
    for (u32& index : mesh.Indices())
        index = reindexation[index];

    // Duplicate vertices are still there but not referenced, call RemoveUnusedVertices()

    return (vertexCount - verticesSet.size());
}
//----------------------------------------------------------------------------
size_t RemoveZeroAreaTriangles(FGenericMesh& mesh, size_t index, float minArea/* = Epsilon */) {
    if (0 == mesh.IndexCount() || 0 == mesh.VertexCount())
        return 0;

    const TGenericVertexSubPart<float3> sp_position3f = mesh.Position3f_IFP(index);
    const TGenericVertexSubPart<float4> sp_position4f = mesh.Position4f_IFP(index);
    PPE_LOG_CHECK(MeshBuilder, sp_position3f || sp_position4f);

    const TMemoryView<u32> indices = mesh.Indices();
    const float twiceMinArea = 2 * minArea;

    const TMemoryView<const float3> positions3f = sp_position3f.MakeView();
    const TMemoryView<const float4> positions4f = sp_position4f.MakeView();

    u32 indexCount = 0;
    for (size_t t = 0; t < indices.size(); t += 3)
    {
        const uint32_t i0 = indices[t + 0];
        const uint32_t i1 = indices[t + 1];
        const uint32_t i2 = indices[t + 2];

        float3 p0, p1, p2;
        if (sp_position3f) {
            p0 = positions3f[i0];
            p1 = positions3f[i1];
            p2 = positions3f[i2];
        }
        else {
            p0 = positions4f[i0].xyz;
            p1 = positions4f[i1].xyz;
            p2 = positions4f[i2].xyz;
        }

        float twiceArea = 0;
        twiceArea = Max(twiceArea, Length(Cross(p1 - p0, p2 - p0)));
        twiceArea = Max(twiceArea, Length(Cross(p0 - p1, p2 - p1)));
        twiceArea = Max(twiceArea, Length(Cross(p0 - p2, p1 - p2)));

        if (twiceArea > twiceMinArea)
        {
            indices[indexCount++] = i0;
            indices[indexCount++] = i1;
            indices[indexCount++] = i2;
        }
    }

    if (indexCount == indices.size())
        return 0;
    Assert(indexCount < indices.size());

    mesh.Resize(indexCount, mesh.VertexCount(), true);

    return (indices.size() - indexCount);
}
//----------------------------------------------------------------------------
size_t RemoveUnusedVertices(FGenericMesh& mesh) {
    if (mesh.VertexCount() == 0)
        return 0;


    STACKLOCAL_BITSET(inUse, mesh.VertexCount());
    inUse.ResetAll(false);

    u32 vertexCountUsed = 0;
    for (u32 vertexIndex : mesh.Indices()) {
        if (!inUse[vertexIndex]) {
            inUse.SetTrue(vertexIndex);
            vertexCountUsed++;
        }
    }

    if (vertexCountUsed == mesh.VertexCount())
        return 0;

    const u32 vertexCountBefore = mesh.VertexCount();
    Assert(vertexCountUsed < vertexCountBefore);

    STACKLOCAL_POD_ARRAY(u32, reindexation, mesh.VertexCount());

    u32 insertIndex = 0;
    forrange(vertexIndex, 0, mesh.VertexCount()) {
        if (inUse[vertexIndex]) {
            if (vertexIndex != insertIndex)
                mesh.VertexCopy(insertIndex, vertexIndex);

            reindexation[vertexIndex] = insertIndex++;
        }
#ifdef WITH_PPE_ASSERT
        else {
            reindexation[vertexIndex] = u32(-1);
        }
#endif
    }
    Assert(insertIndex == vertexCountUsed);

    for (u32& index : mesh.Indices()) {
        Assert(reindexation[index] != u32(-1));
        Assert(reindexation[index] <= index);
        index = reindexation[index];
    }

    mesh.Resize(mesh.IndexCount(), vertexCountUsed, true);

    return (vertexCountBefore - vertexCountUsed);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
// PN-Triangles tessellation :
static void PNTesselateRecursive_(
    u32* pBaseIndex,
    u32* pBaseVertex,
    const u32 vertexOffset,
    const TMemoryView<u32>& indices,
    const TMemoryView<float3>& vertices,
    const float3& uvw00, u32 i00,
    const float3& uvw11, u32 i11,
    const float3& uvw22, u32 i22,
    size_t n ) {
    if (n) {
        n--;

        const u32 j01 = (*pBaseVertex)++;
        const u32 j12 = (*pBaseVertex)++;
        const u32 j20 = (*pBaseVertex)++;

        const float3 uvw01 = (uvw00 + uvw11) * 0.5f;
        const float3 uvw12 = (uvw11 + uvw22) * 0.5f;
        const float3 uvw20 = (uvw22 + uvw00) * 0.5f;

        vertices[j01 - vertexOffset] = uvw01;
        vertices[j12 - vertexOffset] = uvw12;
        vertices[j20 - vertexOffset] = uvw20;

        PNTesselateRecursive_(pBaseIndex, pBaseVertex, vertexOffset, indices, vertices, uvw00,i00, uvw01,j01, uvw20,j20, n);
        PNTesselateRecursive_(pBaseIndex, pBaseVertex, vertexOffset, indices, vertices, uvw01,j01, uvw11,i11, uvw12,j12, n);
        PNTesselateRecursive_(pBaseIndex, pBaseVertex, vertexOffset, indices, vertices, uvw12,j12, uvw20,j20, uvw01,j01, n);
        PNTesselateRecursive_(pBaseIndex, pBaseVertex, vertexOffset, indices, vertices, uvw20,j20, uvw12,j12, uvw22,i22, n);
    }
    else {
        indices[(*pBaseIndex)++] = i00;
        indices[(*pBaseIndex)++] = i11;
        indices[(*pBaseIndex)++] = i22;
    }
}
//----------------------------------------------------------------------------
// The generated index order will be very inefficient, better call OptimizeIndices() afterwards
static void PNTriangles_(
    FGenericMesh& mesh,
    const FPositions3f& sp_position3f,
    const FNormals3f& sp_normal3f,
    size_t recursions ) {
    if (0 == recursions)
        return;

    TFixedSizeStack<FGenericVertexData*, 6> sp_lerps;
    for (const UGenericVertexData& vertices : mesh.Vertices()) {
        if (vertices.get() != sp_position3f.Data() &&
            vertices.get() != sp_normal3f.Data() ) {
            sp_lerps.Push(vertices.get());
        }
    }

    const u32 baseIndex = checked_cast<u32>(mesh.IndexCount());
    const u32 baseVertex = checked_cast<u32>(mesh.VertexCount());
    Assert(0 == (baseIndex % 3));

    // Space allocated per triangle perfect quad-tree, geometric series : (4^depth - 1)/3)
    const u32 triangleCount = (baseIndex / 3u);
    const u32 indicesPerTriangle = Pow(4u, checked_cast<unsigned>(recursions)) * 3u;
    const u32 verticesPerTriangle = (indicesPerTriangle / 3u - 1u);

    STACKLOCAL_POD_ARRAY(float3, barycentrics, verticesPerTriangle);
    STACKLOCAL_POD_ARRAY(u32, baseIndices, baseIndex);
    Copy(baseIndices, mesh.Indices());

    const u32 nextIndexCount = indicesPerTriangle * triangleCount;
    const u32 nextVertexCount = baseVertex + verticesPerTriangle * triangleCount;
    mesh.Resize(nextIndexCount, nextVertexCount);

    const TMemoryView<float3> positions3f = sp_position3f.MakeView();
    const TMemoryView<float3> normals3f = sp_normal3f.MakeView();
    Assert(positions3f.size() == nextVertexCount);
    Assert(normals3f.size() == nextVertexCount);

    FPNTriangle pn;

    u32 nextIndex = 0;
    u32 nextVertex = baseVertex;
    for (u32 i = 0; i < baseIndex; i += 3) {
        const u32 i0 = baseIndices[i + 0];
        const u32 i1 = baseIndices[i + 1];
        const u32 i2 = baseIndices[i + 2];

        ONLY_IF_ASSERT(const u32 startIndex = nextIndex);
        const u32 startVertex = nextVertex;

        PNTesselateRecursive_(
            &nextIndex,
            &nextVertex,
            startVertex,
            mesh.Indices(),
            barycentrics,
            float3(1,0,0), i0,
            float3(0,1,0), i1,
            float3(0,0,1), i2,
            recursions );

        FPNTriangle::FromTriangle(pn,
            positions3f[i0], normals3f[i0],
            positions3f[i1], normals3f[i1],
            positions3f[i2], normals3f[i2] );

        Assert_NoAssume(indicesPerTriangle == (nextIndex - startIndex));
        Assert(verticesPerTriangle == (nextVertex - startVertex));

        forrange(j, startVertex, nextVertex)
            positions3f[j] = pn.LerpPosition(barycentrics[j - startVertex]);

        forrange(j, startVertex, nextVertex)
            normals3f[j] = pn.LerpNormal(barycentrics[j - startVertex]);

        for (FGenericVertexData* pData : sp_lerps) {
            void (*lerpF)(const FRawMemory& dst, const FRawMemoryConst& a, const FRawMemoryConst& b, const FRawMemoryConst& c, const float3& uvw);
            switch (pData->Format()) {
            case RHI::EVertexFormat::Float:
                lerpF = [](const FRawMemory& dst, const FRawMemoryConst& a, const FRawMemoryConst& b, const FRawMemoryConst& c, const float3& uvw) {
                    dst.Cast<float>()[0] = BarycentricLerp(a.Cast<const float>()[0], b.Cast<const float>()[0], c.Cast<const float>()[0], uvw.x, uvw.y, uvw.z);
                };
                break;
            case RHI::EVertexFormat::Float2:
                lerpF = [](const FRawMemory& dst, const FRawMemoryConst& a, const FRawMemoryConst& b, const FRawMemoryConst& c, const float3& uvw) {
                    dst.Cast<float2>()[0] = BarycentricLerp(a.Cast<const float2>()[0], b.Cast<const float2>()[0], c.Cast<const float2>()[0], uvw.x, uvw.y, uvw.z);
                };
                break;
            case RHI::EVertexFormat::Float3:
                lerpF = [](const FRawMemory& dst, const FRawMemoryConst& a, const FRawMemoryConst& b, const FRawMemoryConst& c, const float3& uvw) {
                    dst.Cast<float3>()[0] = BarycentricLerp(a.Cast<const float3>()[0], b.Cast<const float3>()[0], c.Cast<const float3>()[0], uvw.x, uvw.y, uvw.z);
                };
                break;
            case RHI::EVertexFormat::Float4:
                lerpF = [](const FRawMemory& dst, const FRawMemoryConst& a, const FRawMemoryConst& b, const FRawMemoryConst& c, const float3& uvw) {
                    dst.Cast<float4>()[0] = BarycentricLerp(a.Cast<const float4>()[0], b.Cast<const float4>()[0], c.Cast<const float4>()[0], uvw.x, uvw.y, uvw.z);
                };
                break;
            default:
                AssertNotImplemented();
            }

            const FRawMemoryConst v0 = pData->Vertex(i0);
            const FRawMemoryConst v1 = pData->Vertex(i1);
            const FRawMemoryConst v2 = pData->Vertex(i2);
            forrange(tess, 0, barycentrics.size()) {
                lerpF(pData->SubRange(startVertex + tess, 1), v0, v1, v2, barycentrics[tess]);
            }
        }

        Assert_NoAssume(nextIndex == ((i+3)/3) * indicesPerTriangle);
        Assert_NoAssume(nextVertex == baseVertex + ((i+3)/3) * verticesPerTriangle);
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
bool PNTriangles(FGenericMesh& mesh, size_t index, size_t recursions) {
    const TGenericVertexSubPart<float3> sp_position3f = mesh.Position3f_IFP(index);
    if (!sp_position3f)
        return false;

    const TGenericVertexSubPart<float3> sp_normal3f = mesh.Normal3f_IFP(index);
    if (!sp_normal3f)
        return false;

    // You have to generate those after tessellation,
    // or these will linearly interpolated (instead of quadratically, like normals3f)
    // and you would get a wrong TBN basis.
    // Better call ComputeTangentSpace() after this pass.
    Assert(!mesh.Normal4f_IFP(index));
    Assert(!mesh.Tangent3f_IFP(index));
    Assert(!mesh.Tangent4f_IFP(index));
    Assert(!mesh.Binormal3f_IFP(index));

    PNTriangles_(mesh, sp_position3f, sp_normal3f, recursions);

    return true;
}
//----------------------------------------------------------------------------
void PNTriangles(FGenericMesh& mesh, const FPositions3f& positions, const FNormals3f& normals, size_t recursions) {
    PNTriangles_(mesh, positions, normals, recursions);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Transform(FGenericMesh& mesh, size_t index, const float4x4& transform) {
    Assert(not IsNANorINF(transform));

    if (const TGenericVertexSubPart<float3> sp_position3f = mesh.Position3f_IFP(index)) {
        ParallelForEachRef(sp_position3f.MakeView().begin(), sp_position3f.MakeView().end(), [&](float3& p) {
            p = TransformPosition3(transform, p);
        });
    }
    else if (const TGenericVertexSubPart<float4> sp_position4f = mesh.Position4f_IFP(index)) {
        ParallelForEachRef(sp_position4f.MakeView().begin(), sp_position4f.MakeView().end(), [&](float4& p) {
            p = transform.Multiply(p);
        });
    }

    if (const TGenericVertexSubPart<float3> sp_normal3f = mesh.Normal3f_IFP(index)) {
        ParallelForEachRef(sp_normal3f.MakeView().begin(), sp_normal3f.MakeView().end(), [&](float3& p) {
            p = Normalize(TransformVector3(transform, p));
        });
    }
    else if (const TGenericVertexSubPart<float4> sp_normal4f = mesh.Normal4f_IFP(index)) {
        float3 scale;
        float3 translation;
        FQuaternion rotation;
        Decompose(transform, scale, rotation, translation);
        ParallelForEachRef(sp_normal4f.MakeView().begin(), sp_normal4f.MakeView().end(), [&](float4& p) {
            p = (rotation * FQuaternion(p)).Normalize().data;
        });
    }

    if (const TGenericVertexSubPart<float3> sp_tangent3f = mesh.Tangent3f_IFP(index)) {
        ParallelForEachRef(sp_tangent3f.MakeView().begin(), sp_tangent3f.MakeView().end(), [&](float3& p) {
            p = Normalize(TransformVector3(transform, p));
        });
    }
    else if (const TGenericVertexSubPart<float4> sp_tangent4f = mesh.Tangent4f_IFP(index)) {
        ParallelForEachRef(sp_tangent4f.MakeView().begin(), sp_tangent4f.MakeView().end(), [&](float4& p) {
            p = float4(Normalize(TransformVector3<float>(transform, p.xyz)), p.w);
        });
    }

    if (const TGenericVertexSubPart<float3> sp_binormal3f = mesh.Binormal3f_IFP(index)) {
        ParallelForEachRef(sp_binormal3f.MakeView().begin(), sp_binormal3f.MakeView().end(), [&](float3& p) {
            p = Normalize(TransformVector3(transform, p));
        });
    }
}
//----------------------------------------------------------------------------
void Transform(FGenericMesh& mesh, size_t index, const FTransform& transform) {
    Assert(not IsNANorINF(transform));

    if (const TGenericVertexSubPart<float3> sp_position3f = mesh.Position3f_IFP(index)) {
        ParallelForEachRef(sp_position3f.MakeView().begin(), sp_position3f.MakeView().end(), [&](float3& p) {
            p = transform.TransformPosition(p);
        });
    }
    else if (const TGenericVertexSubPart<float4> sp_position4f = mesh.Position4f_IFP(index)) {
        ParallelForEachRef(sp_position4f.MakeView().begin(), sp_position4f.MakeView().end(), [&](float4& p) {
            p = transform.Transform(p);
        });
    }

    if (const TGenericVertexSubPart<float3> sp_normal3f = mesh.Normal3f_IFP(index)) {
        ParallelForEachRef(sp_normal3f.MakeView().begin(), sp_normal3f.MakeView().end(), [&](float3& p) {
            p = transform.TransformVectorNoScale(p);
        });
    }
    else if (const TGenericVertexSubPart<float4> sp_normal4f = mesh.Normal4f_IFP(index)) {
        ParallelForEachRef(sp_normal4f.MakeView().begin(), sp_normal4f.MakeView().end(), [&](float4& p) {
            p = (transform.Rotation() * FQuaternion(p)).data;
        });
    }

    if (const TGenericVertexSubPart<float3> sp_tangent3f = mesh.Tangent3f_IFP(index)) {
        ParallelForEachRef(sp_tangent3f.MakeView().begin(), sp_tangent3f.MakeView().end(), [&](float3& p) {
            p = transform.TransformVectorNoScale(p);
        });
    }
    else if (const TGenericVertexSubPart<float4> sp_tangent4f = mesh.Tangent4f_IFP(index)) {
        ParallelForEachRef(sp_tangent4f.MakeView().begin(), sp_tangent4f.MakeView().end(), [&](float4& p) {
            p = float4(transform.TransformVectorNoScale(p.xyz), p.w);
        });
    }

    if (const TGenericVertexSubPart<float3> sp_binormal3f = mesh.Binormal3f_IFP(index)) {
        ParallelForEachRef(sp_binormal3f.MakeView().begin(), sp_binormal3f.MakeView().end(), [&](float3& p) {
            p = transform.TransformVectorNoScale(p);
        });
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void OptimizeIndicesOrder(const TMemoryView<u32>& indices, size_t vertexCount) {
    Assert(indices.size() >= 3);
    Assert(0 == (indices.size() % 3));

    // prepare triangles
    STACKLOCAL_POD_ARRAY(FVertexCache::FEntry, entries, vertexCount);
    for (FVertexCache::FEntry& entry : entries) {
        entry.CachePos = UINT32_MAX;
        entry.Score = 0;
        entry.TrianglesLeft = 0;
        entry.TriangleList = nullptr;
        entry.OpenPos = UINT32_MAX;
    }

    // alloc space for entry triangle indices
    const u32 triangleCount = checked_cast<u32>(indices.size() / 3);
    STACKLOCAL_POD_ARRAY(FVertexCache::FTriangle, triangles, triangleCount);
    {
        const u32 *pIndex = &indices[0];
        for (u32 i = 0; i < triangleCount; ++i) {
            FVertexCache::FTriangle& triangle = triangles[i];
            triangle.Score = 0;
            for (u32 j = 0; j < 3; ++j) {
                const u32 index = *pIndex++;
                triangle.Indices[j] = index;
                ++entries[index].TrianglesLeft;
            }
        }
    }

    STACKLOCAL_POD_ARRAY(u32, adjacencies, triangleCount*3);
    {
        u32 *pTriList = adjacencies.Pointer();
        ONLY_IF_ASSERT(const u32 *pTriListEnd = adjacencies.Pointer() + adjacencies.size());

        for (u32 i = 0; i < vertexCount; ++i) {
            FVertexCache::FEntry& entry = entries[i];
            entry.TriangleList = pTriList;
            pTriList += entry.TrianglesLeft;
            Assert_NoAssume(pTriList <= pTriListEnd);
            entry.TrianglesLeft = 0;
        }

        for (u32 i = 0; i < triangleCount; ++i) {
            const FVertexCache::FTriangle& triangle = triangles[i];
            for (u32 j = 0; j < 3; ++j) {
                const u32 index = triangle.Indices[j];
                FVertexCache::FEntry& entry = entries[index];
                entry.TriangleList[entry.TrianglesLeft++] = u32(i);
            }
        }
    }

    // open vertices
    STACKLOCAL_POD_ARRAY(u32, openVertices, vertexCount);
    u32 openCount = 0;

    // the cache
    u32 cache[FVertexCache::Size + 3] = {UINT32_MAX};
    u32 pos2score[FVertexCache::Size];
    u32 val2score[FVertexCache::Size + 1];

    for (u32 i = 0; i < FVertexCache::Size; ++i) {
        const float score = (i < 3) ? 0.75f : std::pow(1.0f - (i - 3)/float(FVertexCache::Size - 3), 1.5f);
        pos2score[i] = static_cast<u32>(score * 65536.0f + 0.5f);
    }

    val2score[0] = 0;
    for(int i=1;i<16;i++)
    {
        const float score = 2.0f / std::sqrt(float(i));
        val2score[i] = static_cast<u32>(score * 65536.0f + 0.5f);
    }

    // outer loop: find triangle to start with
    u32 *pIndex = &indices[0];
    u32 seedPos = 0;

    for (;;) {
        struct seed_triangle_t {
            u32 seedScore = 0;
            u32 seedTriangle = UINT32_MAX;

            bool operator <(const seed_triangle_t& other) const {
                return (seedScore > other.seedScore);
            }
        };

        // if there are open vertices, search them for the seed triangle
        // which maximum score.
        seed_triangle_t bestTri = ParallelMapReduce(0, openCount,
            [&](size_t i) NOEXCEPT -> seed_triangle_t {
                const FVertexCache::FEntry& entry = entries[openVertices[i]];

                seed_triangle_t bestTri;
                for (u32 j = 0; j < entry.TrianglesLeft; ++j) {
                    const u32 index = entry.TriangleList[j];
                    const FVertexCache::FTriangle& triangle = triangles[index];

                    if (triangle.Score > bestTri.seedScore) {
                        bestTri.seedScore = triangle.Score;
                        bestTri.seedTriangle = index;
                    }
                }

                return bestTri;
            },
            [](const seed_triangle_t& lhs, const seed_triangle_t& rhs) NOEXCEPT -> seed_triangle_t {
                return (lhs.seedScore < rhs.seedScore ? rhs : lhs);
            });

        // if we haven't found a seed triangle yet, there are no open
        // vertices and we can pick any triangle
        if (UINT32_MAX == bestTri.seedTriangle) {
            while (seedPos < triangleCount && triangles[seedPos].Score == UINT32_MAX)
                ++seedPos;

            if (seedPos == triangleCount) // no triangle left, we're done!
                break;

            bestTri.seedTriangle = seedPos;
        }

        while (bestTri.seedTriangle != UINT32_MAX) {
            FVertexCache::FTriangle& triangle = triangles[bestTri.seedTriangle];

            // mark this triangle as used, remove it from the "remaining tris"
            // list of the vertices it uses, and add it to the index buffer.
            triangle.Score = UINT32_MAX;

            for (u32 j = 0; j < 3; ++j) {
                const u32 index = triangle.Indices[j];
                *pIndex++ = index;

                FVertexCache::FEntry& entry = entries[index];

                // find this triangles' entry
                u32 k = 0;
                while(entry.TriangleList[k] != bestTri.seedTriangle) {
                    Assert(k < entry.TrianglesLeft);
                    ++k;
                }

                // swap it to the end and decrement # of tris left
                if(--entry.TrianglesLeft) {
                    std::swap(entry.TriangleList[k], entry.TriangleList[entry.TrianglesLeft]);
                }
                else if(entry.OpenPos != UINT32_MAX) {
                    Assert(openCount);
                    std::swap(openVertices[entry.OpenPos], openVertices[--openCount]);
                }
            }

            // update cache status
            cache[FVertexCache::Size] = cache[FVertexCache::Size + 1] = cache[FVertexCache::Size + 2] = UINT32_MAX;

            for(u32 j = 0; j < 3 ; ++j) {
                const u32 index = triangle.Indices[j];
                cache[FVertexCache::Size + 2] = index;

                // find vertex index
                u32 pos = 0;
                while (index != cache[pos])
                    ++pos;

                // move to front
                for(int k = pos; k > 0; k--)
                    cache[k] = cache[k - 1];

                cache[0] = index;

                // remove sentinel if it wasn't used
                if(pos != FVertexCache::Size + 2)
                    cache[FVertexCache::Size + 2] = UINT32_MAX;
            }

            // update vertex scores
            for (u32 i = 0; i < FVertexCache::Size + 3; ++i) {
                const u32 index = cache[i];
                if (UINT32_MAX == index)
                    continue;

                FVertexCache::FEntry& entry = entries[index];

                entry.Score = val2score[std::min(entry.TrianglesLeft, u32(FVertexCache::MaxValence))];
                if (i < FVertexCache::Size) {
                    entry.CachePos = i;
                    entry.Score += pos2score[i];
                }
                else {
                    entry.CachePos = UINT32_MAX;

                    // also add to open vertices list if the vertex is indeed open
                    if (UINT32_MAX == entry.OpenPos && entry.TrianglesLeft) {
                        entry.OpenPos = openCount;
                        openVertices[openCount++] = index;
                    }
                }
            }

            // update triangle scores, find new best triangle
            bestTri.seedScore = 0;
            bestTri.seedTriangle = UINT32_MAX;

            for (u32 i = 0; i < FVertexCache::Size; ++i) {
                if (cache[i] == UINT32_MAX)
                    continue;

                const FVertexCache::FEntry& entry = entries[cache[i]];

                forrange(j, 0, entry.TrianglesLeft) {
                    const u32 index = entry.TriangleList[j];
                    FVertexCache::FTriangle& tri = triangles[index];

                    Assert(tri.Score != UINT32_MAX);

                    u32 score = 0;
                    for (u32 k = 0; k < 3; ++k)
                        score += entries[tri.Indices[k]].Score;

                    tri.Score = score;
                    if (tri.Score > bestTri.seedScore)
                        bestTri = seed_triangle_t{score, index};
                }
            }
        } //!while (bestTriangle != UINT32_MAX)
    } //!for (;;)
}
//----------------------------------------------------------------------------
void OptimizeIndicesOrder(FGenericMesh& mesh) {
    OptimizeIndicesOrder(mesh.Indices(), mesh.VertexCount());
}
//----------------------------------------------------------------------------
void OptimizeVerticesOrder(FGenericMesh& mesh) { // also removes unused vertices as a side effect
    const u32 vertexCount = mesh.VertexCount();

    STACKLOCAL_POD_ARRAY(u32, tmpData, vertexCount*3);
    FPlatformMemory::Memset(tmpData.Pointer(), 0xFF, tmpData.SizeInBytes());

    const TMemoryView<u32> old2new = tmpData.SubRange(0, vertexCount);
    const TMemoryView<u32> reindexation = tmpData.SubRange(vertexCount, vertexCount);
    const TMemoryView<u32> tmp = tmpData.SubRange(2*vertexCount, vertexCount);

    u32 sortedCount = 0;

    // Construct mapping and new VB
    for (u32& index : mesh.Indices()) {
        if (vertexCount <= old2new[index]) {
            reindexation[sortedCount] = index;
            old2new[index] = sortedCount++;
        }
        index = old2new[index];
    }

    // Handle unused vertices
    if (sortedCount < vertexCount) {
        u32 n = sortedCount;
        for (u32& index : old2new) {
            if (vertexCount <= index)
                index = n++;
        }
    }
    Assert(sortedCount <= vertexCount);

    // Reorder each vertex subpart
    const size_t maxVertexStride = *mesh.Vertices().Map([](const UGenericVertexData& v) {
        return v->StrideInBytes();
    }).MaxElement();
    STACKLOCAL_POD_ARRAY(u8, tmpVertex, maxVertexStride);

    for (const UGenericVertexData& subpart : mesh.Vertices()) {
        Copy(tmp, reindexation);

        const FRawMemory rawVertex = tmpVertex.CutBefore(subpart->StrideInBytes());

        forrange(i, 0, vertexCount) {
            subpart->ReadVertex(i, rawVertex);

            u32 j = i;
            while (true) {
                const u32 k = tmp[j];
                tmp[j] = j;
                if (k == i)
                    break;

                subpart->CopyVertex(j, k);
                j = k;
            }

            subpart->WriteVertex(j, rawVertex);
        }
    }

    // Remove unused vertices
    if (sortedCount < vertexCount)
        mesh.Resize(mesh.IndexCount(), sortedCount);
}
//----------------------------------------------------------------------------
void OptimizeIndicesAndVerticesOrder(FGenericMesh& mesh) {
    OptimizeIndicesOrder(mesh);
    OptimizeVerticesOrder(mesh);
}
//----------------------------------------------------------------------------
float VertexAverageCacheMissRate(const TMemoryView<const u32>& indices, bool fifo/* = true */, size_t cacheSize/* = 16 */) {
    Assert(cacheSize > 1);

    if (indices.empty())
        return 0.0f;

    const u32 indexCount = checked_cast<u32>(indices.size());

    u32 misses = 0;
    u32 wrPos = 0;

    STACKLOCAL_POD_ARRAY(u32, cache, cacheSize+1);

    // initialize cache (we simulate a FIFO here)
    for (u32 i = 0; i < cacheSize; ++i)
        cache[i] = UINT32_MAX;

    // simulate
    for (u32 i = 0; i < indexCount; ++i) {
        const u32 index = indices[i];
        cache[cacheSize] = index;

        // find in cache
        u32 cachePos = 0;
        while (cache[cachePos] != index)
            ++cachePos;

        misses += (cachePos == cacheSize);

        if (fifo) {
            cache[wrPos] = index;
            if (++wrPos == cacheSize)
                wrPos = 0;
        }
        else {
            // move to front
            for (u32 j = cachePos; j > 0; --j)
                cache[j] = cache[j - 1];

            cache[0] = index;
        }
    }

    const float ACMR = misses * 3.0f / indexCount;
    return ACMR;
}
//----------------------------------------------------------------------------
float VertexAverageCacheMissRate(const FGenericMesh& mesh, bool fifo/* = true */, size_t cacheSize/* = 16 */) {
    return VertexAverageCacheMissRate(mesh.Indices(), fifo, cacheSize);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
