﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Mesh/GeometricPrimitives.h"

#include "Mesh/GenericMesh.h"
#include "Mesh/GenericMeshHelpers.h"

#include "Container/HashMap.h"

#include "Maths/ScalarMatrix.h"
#include "Maths/ScalarMatrixHelpers.h"
#include "Maths/ScalarVector.h"
#include "Maths/ScalarVectorHelpers.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void TransformIFP_(
    const TMemoryView<float3>& normalizeds,
    const FPositions3f& positions,
    const float4x4& transform,
    bool useTransform ) {

    if (positions) {
        const TMemoryView<float3> transformed = positions.Append(normalizeds.size());

        if (useTransform) {
            forrange(i, 0, transformed.size())
                transformed[i] = TransformVector3(transform, normalizeds[i]);
        }
        else {
            Copy(transformed, normalizeds);
        }
    }
    else if (useTransform) {
        for (float3& p : normalizeds)
            p = TransformVector3(transform, p);
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
static void Cube_(
    FGenericMesh& mesh,
    const FPositions3f& positions,
    const FTexcoords3f& texcoords,
    const float4x4& transform,
    bool useTransform ) {
    Assert(positions || texcoords);

    const size_t offset = mesh.VertexCount();
    const size_t indexCount = 12 * 3;
    const size_t vertexCount = 24;

    mesh.Reserve(indexCount, vertexCount, true);

    const TMemoryView<float3> normalizeds = (texcoords ? texcoords : positions).Append(vertexCount);

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

    mesh.AddTriangle(3, 1, 0, offset);
    mesh.AddTriangle(2, 1, 3, offset);

    mesh.AddTriangle(6, 4, 5, offset);
    mesh.AddTriangle(7, 4, 6, offset);

    mesh.AddTriangle(11, 9, 8, offset);
    mesh.AddTriangle(10, 9, 11, offset);

    mesh.AddTriangle(14, 12, 13, offset);
    mesh.AddTriangle(15, 12, 14, offset);

    mesh.AddTriangle(19, 17, 16, offset);
    mesh.AddTriangle(18, 17, 19, offset);

    mesh.AddTriangle(22, 20, 21, offset);
    mesh.AddTriangle(23, 20, 22, offset);
}
} //!namespace
//----------------------------------------------------------------------------
void Cube(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords) {
    Cube_(mesh, positions, texcoords, float4x4::Identity(), false);
}
//----------------------------------------------------------------------------
void Cube(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords, const float4x4& transform) {
    Cube_(mesh, positions, texcoords, transform, true);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
static void Pyramid_(
    FGenericMesh& mesh,
    const FPositions3f& positions,
    const FTexcoords3f& texcoords,
    const float4x4& transform,
    bool useTransform ) {
    Assert(positions || texcoords);

    const size_t offset = mesh.VertexCount();
    const size_t indexCount = 4 * 3;
    const size_t vertexCount = 5;

    mesh.Reserve(indexCount, vertexCount, true);

    const TMemoryView<float3> normalizeds = (texcoords ? texcoords : positions).Append(vertexCount);

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
void Pyramid(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords) {
    Pyramid_(mesh, positions, texcoords, float4x4::Identity(), false);
}
//----------------------------------------------------------------------------
void Pyramid(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords, const float4x4& transform) {
    Pyramid_(mesh, positions, texcoords, transform, true);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
static void Octahedron_(
    FGenericMesh& mesh,
    const FPositions3f& positions,
    const FTexcoords3f& texcoords,
    const float4x4& transform,
    bool useTransform ) {
    Assert(positions || texcoords);

    const size_t offset = mesh.VertexCount();
    const size_t indexCount = 8 * 3;
    const size_t vertexCount = 6;

    mesh.Reserve(indexCount, vertexCount, true);

    const TMemoryView<float3> normalizeds = (texcoords ? texcoords : positions).Append(vertexCount);

    size_t i = 0;

    normalizeds[i++] = float3( 0.0f,  1.0f,  0.0f); // 0 top
    normalizeds[i++] = float3( 0.0f,  0.0f, -1.0f); // 1 front
    normalizeds[i++] = float3( 1.0f,  0.0f,  0.0f); // 2 right
    normalizeds[i++] = float3( 0.0f,  0.0f,  1.0f); // 3 back
    normalizeds[i++] = float3(-1.0f,  0.0f,  0.0f); // 4 left
    normalizeds[i++] = float3( 0.0f, -1.0f,  0.0f); // 5 bottom

    Assert(vertexCount == i);

    TransformIFP_(normalizeds, positions, transform, useTransform);

    mesh.AddTriangle(2, 1, 0, offset); // top front-right face
    mesh.AddTriangle(3, 2, 0, offset); // top back-right face
    mesh.AddTriangle(4, 3, 0, offset); // top back-left face
    mesh.AddTriangle(1, 4, 0, offset); // top front-left face
    mesh.AddTriangle(4, 1, 5, offset); // bottom front-left face
    mesh.AddTriangle(3, 4, 5, offset); // bottom back-left face
    mesh.AddTriangle(2, 3, 5, offset); // bottom back-right face
    mesh.AddTriangle(1, 2, 5, offset); // bottom front-right face
}
} //!namespace
//----------------------------------------------------------------------------
void Octahedron(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords) {
    Octahedron_(mesh, positions, texcoords, float4x4::Identity(), false);
}
//----------------------------------------------------------------------------
void Octahedron(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords, const float4x4& transform) {
    Octahedron_(mesh, positions, texcoords, transform, true);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
static void Icosahedron_(
    FGenericMesh& mesh,
    const FPositions3f& positions,
    const FTexcoords3f& texcoords,
    const float4x4& transform,
    bool useTransform ) {
    Assert(positions || texcoords);

    const size_t offset = mesh.VertexCount();
    const size_t indexCount = 20 * 3;
    const size_t vertexCount = 12;

    mesh.Reserve(indexCount, vertexCount, true);

    const TMemoryView<float3> normalizeds = (texcoords ? texcoords : positions).Append(vertexCount);

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

    mesh.AddTriangle(1, 4, 0, offset);
    mesh.AddTriangle(4, 9, 0, offset);
    mesh.AddTriangle(4, 5, 9, offset);
    mesh.AddTriangle(8, 5, 4, offset);
    mesh.AddTriangle(1, 8, 4, offset);
    mesh.AddTriangle(1, 10, 8, offset);
    mesh.AddTriangle(10, 3, 8, offset);
    mesh.AddTriangle(8, 3, 5, offset);
    mesh.AddTriangle(3, 2, 5, offset);
    mesh.AddTriangle(3, 7, 2, offset);
    mesh.AddTriangle(3, 10, 7, offset);
    mesh.AddTriangle(10, 6, 7, offset);
    mesh.AddTriangle(6, 11, 7, offset);
    mesh.AddTriangle(6, 0, 11, offset);
    mesh.AddTriangle(6, 1, 0, offset);
    mesh.AddTriangle(10, 1, 6, offset);
    mesh.AddTriangle(11, 0, 9, offset);
    mesh.AddTriangle(2, 11, 9, offset);
    mesh.AddTriangle(5, 2, 9, offset);
    mesh.AddTriangle(11, 2, 7, offset);
}
} //!namespace
//----------------------------------------------------------------------------
void Icosahedron(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords) {
    Icosahedron_(mesh, positions, texcoords, float4x4::Identity(), false);
}
//----------------------------------------------------------------------------
void Icosahedron(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords, const float4x4& transform) {
    Icosahedron_(mesh, positions, texcoords, transform, true);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
static void ContellatedTetraHedron_(
    FGenericMesh& mesh,
    const FPositions3f& positions,
    const FTexcoords3f& texcoords,
    const float4x4& transform,
    bool useTransform ) {
    Assert(positions || texcoords);

    const size_t offset = mesh.VertexCount();
    const size_t indexCount = 20 * 3;
    const size_t vertexCount = 4 * 3;

    mesh.Reserve(indexCount, vertexCount, true);

    const TMemoryView<float3> normalizeds = (texcoords ? texcoords : positions).Append(vertexCount);

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

    mesh.AddTriangle(0, 1, 2, offset);
    mesh.AddTriangle(0, 2, 3, offset);

    mesh.AddTriangle(0, 3, 4, offset);
    mesh.AddTriangle(1, 0, 5, offset);
    mesh.AddTriangle(2, 1, 6, offset);
    mesh.AddTriangle(3, 2, 7, offset);

    mesh.AddTriangle(8, 10, 9, offset);
    mesh.AddTriangle(8, 11, 10, offset);

    mesh.AddTriangle(11, 8, 4, offset);
    mesh.AddTriangle(10, 11, 7, offset);
    mesh.AddTriangle(9, 10, 6, offset);
    mesh.AddTriangle(8, 9, 5, offset);

    mesh.AddTriangle(4, 3, 7, offset);
    mesh.AddTriangle(4, 7, 11, offset);

    mesh.AddTriangle(7, 2, 6, offset);
    mesh.AddTriangle(7, 6, 10, offset);

    mesh.AddTriangle(6, 1, 5, offset);
    mesh.AddTriangle(6, 5, 9, offset);

    mesh.AddTriangle(5, 0, 4, offset);
    mesh.AddTriangle(5, 4, 8, offset);
}
} //!namespace
//----------------------------------------------------------------------------
void ContellatedTetraHedron(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords) {
    ContellatedTetraHedron_(mesh, positions, texcoords, float4x4::Identity(), false);
}
//----------------------------------------------------------------------------
void ContellatedTetraHedron(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords, const float4x4& transform) {
    ContellatedTetraHedron_(mesh, positions, texcoords, transform, true);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
static void HemiContellatedTetraHedron_(
    FGenericMesh& mesh,
    const FPositions3f& positions,
    const FTexcoords3f& texcoords,
    const float4x4& transform,
    bool useTransform ) {
    Assert(positions || texcoords);

    const size_t offset = mesh.VertexCount();
    const size_t indexCount = 10 * 3;
    const size_t vertexCount = 4 * 2;

    mesh.Reserve(indexCount, vertexCount, true);

    const TMemoryView<float3> normalizeds = (texcoords ? texcoords : positions).Append(vertexCount);

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

    mesh.AddTriangle(0, 1, 2, offset);
    mesh.AddTriangle(0, 2, 3, offset);

    mesh.AddTriangle(0, 3, 4, offset);
    mesh.AddTriangle(1, 0, 5, offset);
    mesh.AddTriangle(2, 1, 6, offset);
    mesh.AddTriangle(3, 2, 7, offset);

    mesh.AddTriangle(4, 3, 7, offset);
    mesh.AddTriangle(7, 2, 6, offset);
    mesh.AddTriangle(6, 1, 5, offset);
    mesh.AddTriangle(5, 0, 4, offset);
}
} //!namespace
//----------------------------------------------------------------------------
void HemiContellatedTetraHedron(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords) {
    HemiContellatedTetraHedron_(mesh, positions, texcoords, float4x4::Identity(), false);
}
//----------------------------------------------------------------------------
void HemiContellatedTetraHedron(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords, const float4x4& transform) {
    HemiContellatedTetraHedron_(mesh, positions, texcoords, transform, true);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
void Geosphere_(
    FGenericMesh& mesh,
    const FPositions3f& positions,
    const FTexcoords3f& texcoords,
    size_t divisions,
    const float4x4& transform,
    bool useTransform ) {
    Assert(divisions);
    Assert(mesh.empty());

    Icosahedron_(mesh, positions, texcoords, float4x4::Identity(), false);

    FNormals3f normals = mesh.Normal3f(0);

    ComputeNormals(mesh, positions, normals);
    PNTriangles(mesh, positions, normals, divisions);

    if (texcoords) {
        for (float3& uvw : texcoords.MakeView())
            uvw = Normalize(uvw);
    }

    if (useTransform) {
        if (positions) {
            for (float3& pos : positions.MakeView())
                pos = TransformPosition3(transform, pos);
        }
        else {
            Assert(texcoords);
            for (float3& uvw : texcoords.MakeView())
                uvw = TransformPosition3(transform, uvw);
        }
    }

    ComputeNormals(mesh, positions, normals);
}
} //!namespace
//----------------------------------------------------------------------------
void Geosphere(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords, size_t divisions) {
    Geosphere_(mesh, positions, texcoords, divisions, float4x4::Identity(), false);
}
//----------------------------------------------------------------------------
void Geosphere(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords, size_t divisions, const float4x4& transform) {
    Geosphere_(mesh, positions, texcoords, divisions, transform, true);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
void HemiGeosphere_(
    FGenericMesh& mesh,
    const FPositions3f& positions,
    const FTexcoords3f& texcoords,
    size_t divisions,
    const float4x4& transform,
    bool useTransform ) {
    Assert(divisions);
    Assert(mesh.empty());

    Pyramid_(mesh, positions, texcoords, float4x4::Identity(), false);

    FNormals3f normals = mesh.Normal3f(0);

    ComputeNormals(mesh, positions, normals);
    PNTriangles(mesh, positions, normals, divisions);

    if (texcoords) {
        for (float3& uvw : texcoords.MakeView())
            uvw = Normalize(uvw);
    }

    if (useTransform) {
        if (positions) {
            for (float3& pos : positions.MakeView())
                pos = TransformPosition3(transform, pos);
        }
        else {
            Assert(texcoords);
            for (float3& uvw : texcoords.MakeView())
                uvw = TransformPosition3(transform, uvw);
        }
    }

    ComputeNormals(mesh, positions, normals);
}
} //!namespace
//----------------------------------------------------------------------------
void HemiGeosphere(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords, size_t divisions) {
    HemiGeosphere_(mesh, positions, texcoords, divisions, float4x4::Identity(), false);
}
//----------------------------------------------------------------------------
void HemiGeosphere(FGenericMesh& mesh, const FPositions3f& positions, const FTexcoords3f& texcoords, size_t divisions, const float4x4& transform) {
    HemiGeosphere_(mesh, positions, texcoords, divisions, transform, true);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
