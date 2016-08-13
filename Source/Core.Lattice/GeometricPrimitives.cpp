#include "stdafx.h"

#include "GeometricPrimitives.h"

#include "GenericMesh.h"
#include "GenericMeshHelpers.h"

#include "Core/Container/HashMap.h"
#include "Core/Container/Pair.h"

#include "Core/Maths/ScalarMatrix.h"
#include "Core/Maths/ScalarMatrixHelpers.h"
#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/ScalarVectorHelpers.h"

namespace Core {
namespace Lattice {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void TransformIFP_(
    const MemoryView<float3>& normalizeds,
    const Positions3f& positions,
    const float4x4& transform,
    bool useTransform ) {

    if (positions) {
        const MemoryView<float3> transformeds = positions.Append(normalizeds.size());

        if (useTransform) {
            forrange(i, 0, transformeds.size())
                transformeds[i] = transform.Multiply_ZeroExtend(normalizeds[i]).Dehomogenize();
        }
        else {
            Copy(transformeds, normalizeds.AddConst());
        }
    }
    else if (useTransform) {
        for (float3& p : normalizeds)
            p = transform.Multiply_ZeroExtend(p).Dehomogenize();
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
static void Cube_(
    GenericMesh& mesh,
    const Positions3f& positions,
    const TexCoords3f& texcoords,
    const float4x4& transform,
    bool useTransform ) {
    Assert(positions || texcoords);

    const size_t offset = mesh.VertexCount();
    const size_t indexCount = 12 * 3;
    const size_t vertexCount = 24;

    mesh.Reserve(indexCount, vertexCount, true);

    const MemoryView<float3> normalizeds = (texcoords ? texcoords : positions).Append(vertexCount);

    size_t i = 0;

    normalizeds[i++] = float3(-1.0f,  1.0f, -1.0f);
    normalizeds[i++] = float3( 1.0f,  1.0f, -1.0f);
    normalizeds[i++] = float3( 1.0f,  1.0f,  1.0f);
    normalizeds[i++] = float3(-1.0f,  1.0f,  1.0f);

    normalizeds[i++] = float3(-1.0f, -1.0f, -1.0f);
    normalizeds[i++] = float3( 1.0f, -1.0f, -1.0f);
    normalizeds[i++] = float3( 1.0f, -1.0f,  1.0f);
    normalizeds[i++] = float3(-1.0f, -1.0f,  1.0f);

    normalizeds[i++] = float3(-1.0f, -1.0f,  1.0f);
    normalizeds[i++] = float3(-1.0f, -1.0f, -1.0f);
    normalizeds[i++] = float3(-1.0f,  1.0f, -1.0f);
    normalizeds[i++] = float3(-1.0f,  1.0f,  1.0f);

    normalizeds[i++] = float3( 1.0f, -1.0f,  1.0f);
    normalizeds[i++] = float3( 1.0f, -1.0f, -1.0f);
    normalizeds[i++] = float3( 1.0f,  1.0f, -1.0f);
    normalizeds[i++] = float3( 1.0f,  1.0f,  1.0f);

    normalizeds[i++] = float3(-1.0f, -1.0f, -1.0f);
    normalizeds[i++] = float3( 1.0f, -1.0f, -1.0f);
    normalizeds[i++] = float3( 1.0f,  1.0f, -1.0f);
    normalizeds[i++] = float3(-1.0f,  1.0f, -1.0f);

    normalizeds[i++] = float3(-1.0f, -1.0f,  1.0f);
    normalizeds[i++] = float3( 1.0f, -1.0f,  1.0f);
    normalizeds[i++] = float3( 1.0f,  1.0f,  1.0f);
    normalizeds[i++] = float3(-1.0f,  1.0f,  1.0f);

    Assert(vertexCount == i);

    TransformIFP_(normalizeds, positions, transform, useTransform);

    mesh.AddTriangle( 3, 1, 0, offset);
    mesh.AddTriangle( 2, 1, 3, offset);

    mesh.AddTriangle( 6, 4, 5, offset);
    mesh.AddTriangle( 7, 4, 6, offset);

    mesh.AddTriangle(11, 9, 8, offset);
    mesh.AddTriangle(10, 9,11, offset);

    mesh.AddTriangle(14,12,13, offset);
    mesh.AddTriangle(15,12,14, offset);

    mesh.AddTriangle(19,17,16, offset);
    mesh.AddTriangle(18,17,19, offset);

    mesh.AddTriangle(22,20,21, offset);
    mesh.AddTriangle(23,20,22, offset);
}
} //!namespace
//----------------------------------------------------------------------------
void Cube(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords) {
    Cube_(mesh, positions, texcoords, float4x4::Identity(), false);
}
//----------------------------------------------------------------------------
void Cube(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords, const float4x4& transform) {
    Cube_(mesh, positions, texcoords, transform, true);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
static void Pyramid_(
    GenericMesh& mesh,
    const Positions3f& positions,
    const TexCoords3f& texcoords,
    const float4x4& transform,
    bool useTransform ) {
    Assert(positions || texcoords);

    const size_t offset = mesh.VertexCount();
    const size_t indexCount = 4 * 3;
    const size_t vertexCount = 5;

    mesh.Reserve(indexCount, vertexCount, true);

    const MemoryView<float3> normalizeds = (texcoords ? texcoords : positions).Append(vertexCount);

    size_t i = 0;

    normalizeds[i++] = float3( 0.0f, 1.0f,  0.0f);  // 0 top
    normalizeds[i++] = float3( 0.0f, 0.0f, -1.0f);  // 1 front
    normalizeds[i++] = float3( 1.0f, 0.0f,  0.0f);  // 2 right
    normalizeds[i++] = float3( 0.0f, 0.0f,  1.0f);  // 3 back
    normalizeds[i++] = float3(-1.0f, 0.0f,  0.0f);  // 4 left

    Assert(vertexCount == i);

    TransformIFP_(normalizeds, positions, transform, useTransform);

    mesh.AddTriangle(0, 1, 2, offset); // top front-right face
    mesh.AddTriangle(0, 2, 3, offset); // top back-right face
    mesh.AddTriangle(0, 3, 4, offset); // top back-left face
    mesh.AddTriangle(0, 4, 1, offset); // top front-left face
}
} //!namespace
//----------------------------------------------------------------------------
void Pyramid(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords) {
    Pyramid_(mesh, positions, texcoords, float4x4::Identity(), false);
}
//----------------------------------------------------------------------------
void Pyramid(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords, const float4x4& transform) {
    Pyramid_(mesh, positions, texcoords, transform, true);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
static void Octahedron_(
    GenericMesh& mesh,
    const Positions3f& positions,
    const TexCoords3f& texcoords,
    const float4x4& transform,
    bool useTransform ) {
    Assert(positions || texcoords);

    const size_t offset = mesh.VertexCount();
    const size_t indexCount = 8 * 3;
    const size_t vertexCount = 6;

    mesh.Reserve(indexCount, vertexCount, true);

    const MemoryView<float3> normalizeds = (texcoords ? texcoords : positions).Append(vertexCount);

    size_t i = 0;

    normalizeds[i++] = float3( 0.0f,  1.0f,  0.0f); // 0 top
    normalizeds[i++] = float3( 0.0f,  0.0f, -1.0f); // 1 front
    normalizeds[i++] = float3( 1.0f,  0.0f,  0.0f); // 2 right
    normalizeds[i++] = float3( 0.0f,  0.0f,  1.0f); // 3 back
    normalizeds[i++] = float3(-1.0f,  0.0f,  0.0f); // 4 left
    normalizeds[i++] = float3( 0.0f, -1.0f,  0.0f); // 5 bottom

    Assert(vertexCount == i);

    TransformIFP_(normalizeds, positions, transform, useTransform);

    mesh.AddTriangle(0, 1, 2, offset); // top front-right face
    mesh.AddTriangle(0, 2, 3, offset); // top back-right face
    mesh.AddTriangle(0, 3, 4, offset); // top back-left face
    mesh.AddTriangle(0, 4, 1, offset); // top front-left face
    mesh.AddTriangle(5, 1, 4, offset); // bottom front-left face
    mesh.AddTriangle(5, 4, 3, offset); // bottom back-left face
    mesh.AddTriangle(5, 3, 2, offset); // bottom back-right face
    mesh.AddTriangle(5, 2, 1, offset); // bottom front-right face
}
} //!namespace
//----------------------------------------------------------------------------
void Octahedron(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords) {
    Octahedron_(mesh, positions, texcoords, float4x4::Identity(), false);
}
//----------------------------------------------------------------------------
void Octahedron(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords, const float4x4& transform) {
    Octahedron_(mesh, positions, texcoords, transform, true);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
static void Icosahedron_(
    GenericMesh& mesh,
    const Positions3f& positions,
    const TexCoords3f& texcoords,
    const float4x4& transform,
    bool useTransform ) {
    Assert(positions || texcoords);

    const size_t offset = mesh.VertexCount();
    const size_t indexCount = 20 * 3;
    const size_t vertexCount = 12;

    mesh.Reserve(indexCount, vertexCount, true);

    const MemoryView<float3> normalizeds = (texcoords ? texcoords : positions).Append(vertexCount);

    size_t i = 0;

    const float X = 0.525731112119133606f;
    const float Z = 0.850650808352039932f;

    normalizeds[i++] = float3(  -X,0.0f,   Z);
    normalizeds[i++] = float3(   X,0.0f,   Z);
    normalizeds[i++] = float3(  -X,0.0f,  -Z);
    normalizeds[i++] = float3(   X,0.0f,  -Z);
    normalizeds[i++] = float3(0.0f,   Z,   X);
    normalizeds[i++] = float3(0.0f,   Z,  -X);
    normalizeds[i++] = float3(0.0f,  -Z,   X);
    normalizeds[i++] = float3(0.0f,  -Z,  -X);
    normalizeds[i++] = float3(   Z,   X,0.0f);
    normalizeds[i++] = float3(  -Z,   X,0.0f);
    normalizeds[i++] = float3(   Z,  -X,0.0f);
    normalizeds[i++] = float3(  -Z,  -X,0.0f);

    Assert(vertexCount == i);

    TransformIFP_(normalizeds, positions, transform, useTransform);

    mesh.AddTriangle( 0, 4, 1, offset);
    mesh.AddTriangle( 0, 9, 4, offset);
    mesh.AddTriangle( 9, 5, 4, offset);
    mesh.AddTriangle( 4, 5, 8, offset);
    mesh.AddTriangle( 4, 8, 1, offset);
    mesh.AddTriangle( 8,10, 1, offset);
    mesh.AddTriangle( 8, 3,10, offset);
    mesh.AddTriangle( 5, 3, 8, offset);
    mesh.AddTriangle( 5, 2, 3, offset);
    mesh.AddTriangle( 2, 7, 3, offset);
    mesh.AddTriangle( 7,10, 3, offset);
    mesh.AddTriangle( 7, 6,10, offset);
    mesh.AddTriangle( 7,11, 6, offset);
    mesh.AddTriangle(11, 0, 6, offset);
    mesh.AddTriangle( 0, 1, 6, offset);
    mesh.AddTriangle( 6, 1,10, offset);
    mesh.AddTriangle( 9, 0,11, offset);
    mesh.AddTriangle( 9,11, 2, offset);
    mesh.AddTriangle( 9, 2, 5, offset);
    mesh.AddTriangle( 7, 2,11, offset);
}
} //!namespace
//----------------------------------------------------------------------------
void Icosahedron(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords) {
    Icosahedron_(mesh, positions, texcoords, float4x4::Identity(), false);
}
//----------------------------------------------------------------------------
void Icosahedron(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords, const float4x4& transform) {
    Icosahedron_(mesh, positions, texcoords, transform, true);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
static void ContellatedTetraHedron_(
    GenericMesh& mesh,
    const Positions3f& positions,
    const TexCoords3f& texcoords,
    const float4x4& transform,
    bool useTransform ) {
    Assert(positions || texcoords);

    const size_t offset = mesh.VertexCount();
    const size_t indexCount = 20 * 3;
    const size_t vertexCount = 4 * 3;

    mesh.Reserve(indexCount, vertexCount, true);

    const MemoryView<float3> normalizeds = (texcoords ? texcoords : positions).Append(vertexCount);

    size_t i = 0;

    // top
    normalizeds[i++] = float3( 0.0f,-1.0f, 1.0f);
    normalizeds[i++] = float3( 1.0f, 0.0f, 1.0f);
    normalizeds[i++] = float3( 0.0f, 1.0f, 1.0f);
    normalizeds[i++] = float3(-1.0f, 0.0f, 1.0f);

    // middle
    normalizeds[i++] = float3(-1.0f,-1.0f, 0.0f);
    normalizeds[i++] = float3( 1.0f,-1.0f, 0.0f);
    normalizeds[i++] = float3( 1.0f, 1.0f, 0.0f);
    normalizeds[i++] = float3(-1.0f, 1.0f, 0.0f);

    // bottom
    normalizeds[i++] = float3( 0.0f,-1.0f,-1.0f);
    normalizeds[i++] = float3( 1.0f, 0.0f,-1.0f);
    normalizeds[i++] = float3( 0.0f, 1.0f,-1.0f);
    normalizeds[i++] = float3(-1.0f, 0.0f,-1.0f);

    Assert(vertexCount == i);

    TransformIFP_(normalizeds, positions, transform, useTransform);

    mesh.AddTriangle( 0, 1, 2, offset);
    mesh.AddTriangle( 0, 2, 3, offset);

    mesh.AddTriangle( 0, 3, 4, offset);
    mesh.AddTriangle( 1, 0, 5, offset);
    mesh.AddTriangle( 2, 1, 6, offset);
    mesh.AddTriangle( 3, 2, 7, offset);

    mesh.AddTriangle( 8,10, 9, offset);
    mesh.AddTriangle( 8,11,10, offset);

    mesh.AddTriangle(11, 8, 4, offset);
    mesh.AddTriangle(10,11, 7, offset);
    mesh.AddTriangle( 9,10, 6, offset);
    mesh.AddTriangle( 8, 9, 5, offset);

    mesh.AddTriangle( 4, 3, 7, offset);
    mesh.AddTriangle( 4, 7,11, offset);

    mesh.AddTriangle( 7, 2, 6, offset);
    mesh.AddTriangle( 7, 6,10, offset);

    mesh.AddTriangle( 6, 1, 5, offset);
    mesh.AddTriangle( 6, 5, 9, offset);

    mesh.AddTriangle( 5, 0, 4, offset);
    mesh.AddTriangle( 5, 4, 8, offset);
}
} //!namespace
//----------------------------------------------------------------------------
void ContellatedTetraHedron(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords) {
    ContellatedTetraHedron_(mesh, positions, texcoords, float4x4::Identity(), false);
}
//----------------------------------------------------------------------------
void ContellatedTetraHedron(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords, const float4x4& transform) {
    ContellatedTetraHedron_(mesh, positions, texcoords, transform, true);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
static void HemiContellatedTetraHedron_(
    GenericMesh& mesh,
    const Positions3f& positions,
    const TexCoords3f& texcoords,
    const float4x4& transform,
    bool useTransform ) {
    Assert(positions || texcoords);

    const size_t offset = mesh.VertexCount();
    const size_t indexCount = 10 * 3;
    const size_t vertexCount = 4 * 2;

    mesh.Reserve(indexCount, vertexCount, true);

    const MemoryView<float3> normalizeds = (texcoords ? texcoords : positions).Append(vertexCount);

    size_t i = 0;

    // top
    normalizeds[i++] = float3( 0.0f,-1.0f, 1.0f);
    normalizeds[i++] = float3( 1.0f, 0.0f, 1.0f);
    normalizeds[i++] = float3( 0.0f, 1.0f, 1.0f);
    normalizeds[i++] = float3(-1.0f, 0.0f, 1.0f);

    // middle
    normalizeds[i++] = float3(-1.0f,-1.0f, 0.0f);
    normalizeds[i++] = float3( 1.0f,-1.0f, 0.0f);
    normalizeds[i++] = float3( 1.0f, 1.0f, 0.0f);
    normalizeds[i++] = float3(-1.0f, 1.0f, 0.0f);

    Assert(vertexCount == i);

    TransformIFP_(normalizeds, positions, transform, useTransform);

    mesh.AddTriangle(0,1,2, offset);
    mesh.AddTriangle(0,2,3, offset);

    mesh.AddTriangle(0,3,4, offset);
    mesh.AddTriangle(1,0,5, offset);
    mesh.AddTriangle(2,1,6, offset);
    mesh.AddTriangle(3,2,7, offset);

    mesh.AddTriangle(4,3,7, offset);
    mesh.AddTriangle(7,2,6, offset);
    mesh.AddTriangle(6,1,5, offset);
    mesh.AddTriangle(5,0,4, offset);
}
} //!namespace
//----------------------------------------------------------------------------
void HemiContellatedTetraHedron(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords) {
    HemiContellatedTetraHedron_(mesh, positions, texcoords, float4x4::Identity(), false);
}
//----------------------------------------------------------------------------
void HemiContellatedTetraHedron(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords, const float4x4& transform) {
    HemiContellatedTetraHedron_(mesh, positions, texcoords, transform, true);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
void Geosphere_(
    GenericMesh& mesh,
    const Positions3f& positions,
    const TexCoords3f& texcoords,
    size_t divisions,
    const float4x4& transform,
    bool useTransform ) {
    Assert(divisions);
    Assert(mesh.empty());

    Icosahedron_(mesh, positions, texcoords, float4x4::Identity(), false);

    Normals3f normals = mesh.Normal3f(0);

    ComputeNormals(mesh, positions, normals);
    PNTriangles(mesh, positions, normals, divisions);

    if (texcoords) {
        for (float3& uvw : texcoords.MakeView())
            uvw = Normalize3(uvw);
    }

    if (useTransform) {
        if (positions) {
            for (float3& pos : positions.MakeView())
                pos = transform.Multiply_OneExtend(pos).Dehomogenize();
        }
        else {
            Assert(texcoords);
            for (float3& uvw : texcoords.MakeView())
                uvw = transform.Multiply_OneExtend(uvw).Dehomogenize();
        }
    }

    ComputeNormals(mesh, positions, normals);
}
} //!namespace
//----------------------------------------------------------------------------
void Geosphere(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords, size_t divisions) {
    Geosphere_(mesh, positions, texcoords, divisions, float4x4::Identity(), false);
}
//----------------------------------------------------------------------------
void Geosphere(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords, size_t divisions, const float4x4& transform) {
    Geosphere_(mesh, positions, texcoords, divisions, transform, true);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
void HemiGeosphere_(
    GenericMesh& mesh,
    const Positions3f& positions,
    const TexCoords3f& texcoords,
    size_t divisions,
    const float4x4& transform,
    bool useTransform ) {
    Assert(divisions);
    Assert(mesh.empty());

    Pyramid_(mesh, positions, texcoords, float4x4::Identity(), false);

    Normals3f normals = mesh.Normal3f(0);

    ComputeNormals(mesh, positions, normals);
    PNTriangles(mesh, positions, normals, divisions);

    if (texcoords) {
        for (float3& uvw : texcoords.MakeView())
            uvw = Normalize3(uvw);
    }

    if (useTransform) {
        if (positions) {
            for (float3& pos : positions.MakeView())
                pos = transform.Multiply_OneExtend(pos).Dehomogenize();
        }
        else {
            Assert(texcoords);
            for (float3& uvw : texcoords.MakeView())
                uvw = transform.Multiply_OneExtend(uvw).Dehomogenize();
        }
    }

    ComputeNormals(mesh, positions, normals);
}
} //!namespace
//----------------------------------------------------------------------------
void HemiGeosphere(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords, size_t divisions) {
    HemiGeosphere_(mesh, positions, texcoords, divisions, float4x4::Identity(), false);
}
//----------------------------------------------------------------------------
void HemiGeosphere(GenericMesh& mesh, const Positions3f& positions, const TexCoords3f& texcoords, size_t divisions, const float4x4& transform) {
    HemiGeosphere_(mesh, positions, texcoords, divisions, transform, true);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace Core
