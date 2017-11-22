#include "stdafx.h"

#include "Engine.h"

#include "ModelBuilder.h"

#include "Core/Maths/ScalarBoundingBox.h"
#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/ScalarMatrixHelpers.h"
#include "Core/Meta/BitField.h"

#include "Core.Engine/Material/Material.h"
#include "Core.Engine/Material/MaterialConstNames.h"
#include "Core.Engine/Material/Parameters/MaterialParameterConstant.h"

#include "Core.Engine/Mesh/Model.h"
#include "Core.Engine/Mesh/ModelBone.h"
#include "Core.Engine/Mesh/ModelMesh.h"
#include "Core.Engine/Mesh/ModelMeshSubPart.h"

#include "Core.Engine/Mesh/Geometry/GenericVertex.h"
#include "Core.Engine/Mesh/Geometry/GenericVertexExport.h"
#include "Core.Engine/Mesh/Geometry/GenericVertexOptimizer.h"

#include "Core.Graphics/Device/Geometry/IndexElementSize.h"
#include "Core.Graphics/Device/Geometry/PrimitiveType.h"
#include "Core.Graphics/Device/Geometry/VertexDeclaration.h"
#include "Core.Graphics/Device/Geometry/VertexTypes.h"

#include "Core/Diagnostic/Logger.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static PMaterial CreateMaterial_(const FModelBuilder::FMaterial& materialMB) {
    Assert(materialMB.Name.size());

    VECTOR(FMaterial, Graphics::FBindName) tags;
    ASSOCIATIVE_VECTOR(FMaterial, Graphics::FBindName, FFilename) textures;
    ASSOCIATIVE_VECTOR(FMaterial, Graphics::FBindName, PMaterialParameter) parameters;

    tags.reserve(Meta::popcnt(materialMB.Mode));
    if (materialMB.HasFlag(FModelBuilder::FMaterial::Ambient))
        tags.push_back(FMaterialConstNames::Ambient());
    if (materialMB.HasFlag(FModelBuilder::FMaterial::BumpMapping))
        tags.push_back(FMaterialConstNames::BumpMapping());
    if (materialMB.HasFlag(FModelBuilder::FMaterial::CastShadows))
        tags.push_back(FMaterialConstNames::CastShadows());
    if (materialMB.HasFlag(FModelBuilder::FMaterial::Color))
        tags.push_back(FMaterialConstNames::Color());
    if (materialMB.HasFlag(FModelBuilder::FMaterial::Emissive))
        tags.push_back(FMaterialConstNames::Emissive());
    if (materialMB.HasFlag(FModelBuilder::FMaterial::Fresnel))
        tags.push_back(FMaterialConstNames::Fresnel());
    if (materialMB.HasFlag(FModelBuilder::FMaterial::Glass))
        tags.push_back(FMaterialConstNames::Glass());
    if (materialMB.HasFlag(FModelBuilder::FMaterial::Highlight))
        tags.push_back(FMaterialConstNames::Highlight());
    if (materialMB.HasFlag(FModelBuilder::FMaterial::Reflection))
        tags.push_back(FMaterialConstNames::Reflection());
    if (materialMB.HasFlag(FModelBuilder::FMaterial::Refraction))
        tags.push_back(FMaterialConstNames::Refraction());
    if (materialMB.HasFlag(FModelBuilder::FMaterial::SeparateAlpha))
        tags.push_back(FMaterialConstNames::SeparateAlpha());
    if (materialMB.HasFlag(FModelBuilder::FMaterial::Transparency))
        tags.push_back(FMaterialConstNames::Transparency());

    textures.reserve(7);
    if (!materialMB.AlphaMap.empty())
        textures.Insert_AssertUnique(FMaterialConstNames::AlphaMap(), materialMB.AlphaMap);
    if (!materialMB.AmbientMap.empty())
        textures.Insert_AssertUnique(FMaterialConstNames::AmbientMap(), materialMB.AmbientMap);
    if (!materialMB.DiffuseMap.empty())
        textures.Insert_AssertUnique(FMaterialConstNames::DiffuseMap(), materialMB.DiffuseMap);
    if (!materialMB.DisplacementMap.empty())
        textures.Insert_AssertUnique(FMaterialConstNames::DisplacementMap(), materialMB.DisplacementMap);
    if (!materialMB.EmissiveMap.empty())
        textures.Insert_AssertUnique(FMaterialConstNames::EmissiveMap(), materialMB.EmissiveMap);
    if (!materialMB.NormalMap.empty())
        textures.Insert_AssertUnique(FMaterialConstNames::NormalMap(), materialMB.NormalMap);
    if (!materialMB.ReflectionMap.empty())
        textures.Insert_AssertUnique(FMaterialConstNames::ReflectionMap(), materialMB.ReflectionMap);
    if (!materialMB.SpecularColorMap.empty())
        textures.Insert_AssertUnique(FMaterialConstNames::SpecularColorMap(), materialMB.SpecularColorMap);
    if (!materialMB.SpecularPowerMap.empty())
        textures.Insert_AssertUnique(FMaterialConstNames::SpecularPowerMap(), materialMB.SpecularPowerMap);

    // TODO : MaterialParameterConstantFactory to merge constants by value

    parameters.reserve(4);
    if (materialMB.AmbientColor.a() >= 0)
        parameters.Insert_AssertUnique(FMaterialConstNames::AmbientColor(), new TMaterialParameterConstant<float4>(materialMB.AmbientColor.Data()));
    if (materialMB.DiffuseColor.a() >= 0)
        parameters.Insert_AssertUnique(FMaterialConstNames::DiffuseColor(), new TMaterialParameterConstant<float4>(materialMB.DiffuseColor.Data()));
    if (materialMB.EmissiveColor.a() >= 0)
        parameters.Insert_AssertUnique(FMaterialConstNames::EmissiveColor(), new TMaterialParameterConstant<float4>(materialMB.EmissiveColor.Data()));
    if (materialMB.SpecularColor.a() >= 0)
        parameters.Insert_AssertUnique(FMaterialConstNames::SpecularColor(), new TMaterialParameterConstant<float4>(materialMB.SpecularColor.Data()));

    if (materialMB.NormalDepth >= 0)
        parameters.Insert_AssertUnique(FMaterialConstNames::NormalDepth(), new TMaterialParameterConstant<float>(materialMB.NormalDepth));
    if (materialMB.RefractiveIndex >= 0)
        parameters.Insert_AssertUnique(FMaterialConstNames::RefractiveIndex(), new TMaterialParameterConstant<float>(materialMB.RefractiveIndex));
    if (materialMB.SpecularExponent >= 0)
        parameters.Insert_AssertUnique(FMaterialConstNames::SpecularExponent(), new TMaterialParameterConstant<float>(materialMB.SpecularExponent));

    LOG(Info, L"[FModel] Loaded material '{0}' with diffuse map '{1}'", materialMB.Name, materialMB.DiffuseMap);

    return new FMaterial("Standard", materialMB.Name, std::move(tags), std::move(textures), std::move(parameters));
}
//----------------------------------------------------------------------------
static PModelBone CreateBone_(const FModelBuilder::FBone& boneMB, const AABB3f& boundingBox) {
    const FMeshName name(boneMB.Name);
    return new FModelBone(name, name, boneMB.Transform, boundingBox);
}
//----------------------------------------------------------------------------
static const Graphics::FVertexDeclaration *GetVertexDeclaration_(const FModelBuilder::FGroup::EFlags mode, bool useNormalMap, bool useVertexColor) {
    using namespace Graphics::Vertex;

    if (FModelBuilder::FGroup::Texcoords_Normals == (mode & FModelBuilder::FGroup::Texcoords_Normals)) {

        if (useNormalMap && useVertexColor)
            return FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N::Declaration;
        else if (useNormalMap)
            return FPosition0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N::Declaration;
        else if (useVertexColor)
            return FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N::Declaration;
        else
            return FPosition0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N::Declaration;

    }
    else if (FModelBuilder::FGroup::Texcoords == (mode & FModelBuilder::FGroup::Texcoords)) {
        AssertRelease(!useNormalMap); // impossible without normals

        if (useVertexColor)
            return FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2::Declaration;
        else
            return FPosition0_Float3__TexCoord0_Half2::Declaration;

    }
    else if (FModelBuilder::FGroup::Normals == (mode & FModelBuilder::FGroup::Normals)) {
        AssertRelease(!useNormalMap); // impossible without texcoords

        if (useVertexColor)
            return FPosition0_Float3__Color0_UByte4N__Normal0_UX10Y10Z10W2N::Declaration;
        else
            return FPosition0_Float3__Normal0_UX10Y10Z10W2N::Declaration;
    }
    else {
        AssertRelease(!useNormalMap); // impossible without texcoords or normals

        if (useVertexColor)
            return FPosition0_Float3__Color0_UByte4N::Declaration;
        else
            return FPosition0_Float3::Declaration;
    }
}
//----------------------------------------------------------------------------
static const Graphics::FVertexDeclaration *GetVertexDeclaration_(
    const FModelBuilder::FGroup& groupMB,
    const FModelBuilder::FMaterial& materialMB,
    bool useVertexColor) {
    const bool useNormalMap = !materialMB.NormalMap.empty();
    return GetVertexDeclaration_(FModelBuilder::FGroup::EFlags(groupMB.Mode), useNormalMap, useVertexColor);
}
//----------------------------------------------------------------------------
static void WriteGroupModelMesh_(
    size_t *pIndexOffset,
    size_t *pVertexOffset,
    AABB3f& boundingBox,
    MeshRawData& indexData,
    MeshRawData& vertexData,
    const FModelBuilder& mb,
    const FModelBuilder::FGroup& groupMB,
    const Graphics::FVertexDeclaration *vertexDeclaration ) {
    Assert(pIndexOffset);
    Assert(pVertexOffset);

    const bool useQuads = groupMB.HasFlag(FModelBuilder::FGroup::Quad);
    const size_t vertexCountPerFace = useQuads ? 4 : 3;

    const size_t vertexCount = groupMB.FaceCount * vertexCountPerFace;
    const size_t vertexOffsetInBytes = *pVertexOffset * vertexDeclaration->SizeInBytes();
    const size_t vertexCountInBytes = vertexCount * vertexDeclaration->SizeInBytes();

    const TMemoryView<u32> indices = indexData.MakeView().SubRange(*pIndexOffset * sizeof(u32), vertexCount * sizeof(u32)).Cast<u32>();

    const size_t firstIndex = *pIndexOffset;

    *pIndexOffset += vertexCount;
    *pVertexOffset += vertexCount;

    FGenericVertex genericVertex(vertexDeclaration);
    genericVertex.SetDestination(vertexData.MakeView().SubRange(vertexOffsetInBytes, vertexCountInBytes));

    const FGenericVertex::FSubPart position0_3f = genericVertex.Position3f(0);
    const FGenericVertex::FSubPart position0_4f = genericVertex.Position4f(0);
    const FGenericVertex::FSubPart color0 = genericVertex.Color4f(0);
    const FGenericVertex::FSubPart texcoord0 = genericVertex.TexCoord2f(0);
    const FGenericVertex::FSubPart normal0 = genericVertex.Normal3f(0);
    Assert(position0_4f || position0_3f);

    size_t indexCount = 0;
    for (size_t f = 0; f < groupMB.FaceCount; ++f) {
        const FModelBuilder::EFace& face = mb.Faces()[groupMB.FaceStart + f];

        for (size_t v = 0; v < vertexCountPerFace; ++v) {
            const float4& pos_4f = mb.Positions()[face.P[v]];
            const float3 pos_3f = pos_4f.xyz();

            genericVertex.ZeroMemory_CurrentVertex();

            if (position0_4f)
                position0_4f.WriteValue(genericVertex, pos_4f);
            else
                position0_3f.WriteValue(genericVertex, pos_3f);

            if (color0)     color0.WriteValue(genericVertex, mb.Colors()[face.P[v]]);
            if (texcoord0)  texcoord0.WriteValue(genericVertex, mb.Texcoords()[face.T[v]].xy());
            if (normal0)    normal0.WriteValue(genericVertex, mb.Normals()[face.N[v]]);

            genericVertex.NextVertex();

            boundingBox.Add(pos_3f);

            indices[indexCount] = checked_cast<u32>(firstIndex + indexCount);
            ++indexCount;
        }
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FModelBuilder::EFace::EFace(const FPositionIndex (&pos)[3]) {
    FPositionIndex::Assign(P, pos); FNormalIndex::Uninitialized(N); FTexcoordIndex::Uninitialized(T);
}
//----------------------------------------------------------------------------
FModelBuilder::EFace::EFace(const FPositionIndex (&pos)[3], const FTexcoordIndex (&texcoords)[3]) {
    FPositionIndex::Assign(P, pos); FNormalIndex::Uninitialized(N); FTexcoordIndex::Assign(T, texcoords);
}
//----------------------------------------------------------------------------
FModelBuilder::EFace::EFace(const FPositionIndex (&pos)[3], const FTexcoordIndex (&texcoords)[3], const FNormalIndex (&normals)[3]) {
    FPositionIndex::Assign(P, pos); FNormalIndex::Assign(N, normals); FTexcoordIndex::Assign(T, texcoords);
}
//----------------------------------------------------------------------------
FModelBuilder::EFace::EFace(const FPositionIndex (&pos)[3], const FNormalIndex (&normals)[3]) {
    FPositionIndex::Assign(P, pos); FNormalIndex::Assign(N, normals); FTexcoordIndex::Uninitialized(T);
}
//----------------------------------------------------------------------------
FModelBuilder::EFace::EFace(const FPositionIndex (&pos)[4]) {
    FPositionIndex::Assign(P, pos); FNormalIndex::Uninitialized(N); FTexcoordIndex::Uninitialized(T);
}
//----------------------------------------------------------------------------
FModelBuilder::EFace::EFace(const FPositionIndex (&pos)[4], const FTexcoordIndex (&texcoords)[4]) {
    FPositionIndex::Assign(P, pos); FNormalIndex::Uninitialized(N); FTexcoordIndex::Assign(T, texcoords);
}
//----------------------------------------------------------------------------
FModelBuilder::EFace::EFace(const FPositionIndex (&pos)[4], const FTexcoordIndex (&texcoords)[4], const FNormalIndex (&normals)[4]) {
    FPositionIndex::Assign(P, pos); FNormalIndex::Assign(N, normals); FTexcoordIndex::Assign(T, texcoords);
}
//----------------------------------------------------------------------------
FModelBuilder::EFace::EFace(const FPositionIndex (&pos)[4], const FNormalIndex (&normals)[4]) {
    FPositionIndex::Assign(P, pos); FNormalIndex::Assign(N, normals); FTexcoordIndex::Uninitialized(T);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FModelBuilder::FModelBuilder()
:   _name()
#ifdef WITH_CORE_ASSERT
,   _openGroup(false)
,   _openMaterial(false)
#endif
{}
//----------------------------------------------------------------------------
FModelBuilder::~FModelBuilder() {
    Assert(!_openGroup);
    Assert(!_openMaterial);
}
//----------------------------------------------------------------------------
void FModelBuilder::SetName(FString&& name) {
    Assert(name.size());
    _name = std::move(name);
}
//----------------------------------------------------------------------------
void FModelBuilder::AddPosition(const float3& xyz) {
    _positions.push_back(float4(xyz, 1.0f));
}
//----------------------------------------------------------------------------
void FModelBuilder::AddPosition(const float4& xyzw) {
    _positions.push_back(xyzw);
}
//----------------------------------------------------------------------------
void FModelBuilder::AddColor(const ColorRGBAF& rgba) {
    Assert(_colors.size() == _positions.size() - 1); // each position should have its color if set
    _colors.push_back(rgba);
}
//----------------------------------------------------------------------------
void FModelBuilder::AddTexCoord(const float2& uv) {
    _texcoords.push_back(float3(uv, 0.0f));
}
//----------------------------------------------------------------------------
void FModelBuilder::AddTexCoord(const float3& uvw) {
    _texcoords.push_back(uvw);
}
//----------------------------------------------------------------------------
void FModelBuilder::AddNormal(const float3& value) {
    _normals.push_back(value);
}
//----------------------------------------------------------------------------
void FModelBuilder::AddTriangle(const FPositionIndex (&pos)[3]) {
    OpenedGroup_().SetMode_CheckCoherency(FGroup::FTriangle);

    _faces.emplace_back(pos);
}
//----------------------------------------------------------------------------
void FModelBuilder::AddTriangle(const FPositionIndex (&pos)[3], const FTexcoordIndex (&texcoords)[3]) {
    OpenedGroup_().SetMode_CheckCoherency(FGroup::Triangle_Texcoords);

    _faces.emplace_back(pos, texcoords);
}
//----------------------------------------------------------------------------
void FModelBuilder::AddTriangle(const FPositionIndex (&pos)[3], const FTexcoordIndex (&texcoords)[3], const FNormalIndex (&normals)[3]) {
    OpenedGroup_().SetMode_CheckCoherency(FGroup::Triangle_Texcoords_Normals);

    _faces.emplace_back(pos, texcoords, normals);
}
//----------------------------------------------------------------------------
void FModelBuilder::AddTriangle(const FPositionIndex (&pos)[3], const FNormalIndex (&normals)[3]) {
    OpenedGroup_().SetMode_CheckCoherency(FGroup::Triangle_Normals);

    _faces.emplace_back(pos, normals);
}
//----------------------------------------------------------------------------
void FModelBuilder::AddQuad(const FPositionIndex (&pos)[4]) {
    OpenedGroup_().SetMode_CheckCoherency(FGroup::Quad);

    _faces.emplace_back(pos);
}
//----------------------------------------------------------------------------
void FModelBuilder::AddQuad(const FPositionIndex (&pos)[4], const FTexcoordIndex (&texcoords)[4]) {
    OpenedGroup_().SetMode_CheckCoherency(FGroup::Quad_Texcoords);

    _faces.emplace_back(pos, texcoords);
}
//----------------------------------------------------------------------------
void FModelBuilder::AddQuad(const FPositionIndex (&pos)[4], const FTexcoordIndex (&texcoords)[4], const FNormalIndex (&normals)[4]) {
    OpenedGroup_().SetMode_CheckCoherency(FGroup::Quad_Texcoords_Normals);

    _faces.emplace_back(pos, texcoords, normals);
}
//----------------------------------------------------------------------------
void FModelBuilder::AddQuad(const FPositionIndex (&pos)[4], const FNormalIndex (&normals)[4]) {
    OpenedGroup_().SetMode_CheckCoherency(FGroup::Quad_Normals);

    _faces.emplace_back(pos, normals);
}
//----------------------------------------------------------------------------
void FModelBuilder::AddBone(FString&& name, const float4x4& transform) {
    Assert(!name.empty());
    Assert(IsHomogeneous(transform));

    _bones.emplace_back(std::move(name), transform);
}
//----------------------------------------------------------------------------
FModelBuilder::FGroup *FModelBuilder::OpenGroup(FString&& name) {
    Assert(!_openGroup);
    Assert(!_openMaterial);
#ifdef WITH_CORE_ASSERT
    _openGroup = true;
#endif
    _groups.emplace_back(std::move(name));
    FGroup *const group = &_groups.back();
    group->FaceStart = checked_cast<u32>(_faces.size());
    return &_groups.back();
}
//----------------------------------------------------------------------------
void FModelBuilder::CloseGroup(FGroup *group) {
    Assert(_openGroup);
    Assert(!_openMaterial);
    Assert(&_groups.back() == group);
#ifdef WITH_CORE_ASSERT
    _openGroup = false;
#endif
    Assert(_faces.size() >= group->FaceStart);
    group->FaceCount = checked_cast<u32>(_faces.size() - group->FaceStart);
}
//----------------------------------------------------------------------------
FModelBuilder::FMaterial *FModelBuilder::OpenMaterial(FString&& name) {
    Assert(!_openGroup);
    Assert(!_openMaterial);
#ifdef WITH_CORE_ASSERT
    _openMaterial = true;
#endif
    _materials.emplace_back(std::move(name));
    return &_materials.back();
}
//----------------------------------------------------------------------------
void FModelBuilder::CloseMaterial(FMaterial *material) {
    Assert(!_openGroup);
    Assert(_openMaterial);
    Assert(&_materials.back() == material);
#ifdef WITH_CORE_ASSERT
    _openMaterial = false;
#endif
}
//----------------------------------------------------------------------------
bool FModelBuilder::MaterialIndexFromName(size_t *pIndex, const FStringView& name) const {
    Assert(pIndex);
    Assert(!name.empty());

    const size_t materialCount = _materials.size();
    for (size_t i = 0; i < materialCount; ++i) {
        const FMaterial& materialMB = _materials[i];
        if (materialMB.Name.size() == name.size() &&
            0 == CompareN(materialMB.Name.c_str(), name.Pointer(), name.size())) {
            *pIndex = i;
            return true;
        }
    }

    return false;
}
//----------------------------------------------------------------------------
PModel FModelBuilder::CreateModel() {
    Assert(_positions.size());
    Assert(_colors.empty() || _colors.size() == _positions.size());

    struct FMeshStat {
        size_t IndexCount;
        size_t VertexCount;
        size_t SubPartCount;
        Graphics::EPrimitiveType EPrimitiveType;
        FMeshStat() : IndexCount(0), VertexCount(0), SubPartCount(0) {}
    };
    const bool useVertexColor = (_positions.size() == _colors.size());
    const auto groupMeshIndices = MALLOCA_VIEW(size_t, _groups.size());
    VECTOR_THREAD_LOCAL(MeshGeneration, FMeshStat) meshStats;
    VECTOR_THREAD_LOCAL(MeshGeneration, Graphics::PCVertexDeclaration) vertexDeclarations;
    for (size_t i = 0; i < _groups.size(); ++i) {
        const FGroup& groupMB = _groups[i];
        Assert(groupMB.Bone < _bones.size());
        Assert(groupMB.Material < _materials.size());
        Assert(groupMB.FaceStart + groupMB.FaceCount <= _faces.size());

        FMaterial& materialMB = _materials[groupMB.Material];
        materialMB.SetFlag(FMaterial::_InUse);

        const Graphics::PCVertexDeclaration vertexDeclaration = GetVertexDeclaration_(
            groupMB,
            _materials[groupMB.Material],
            useVertexColor );

        size_t meshIndex = 0;
        if (!FindElementIndexIFP(&meshIndex, vertexDeclarations, vertexDeclaration)) {
            meshIndex = meshStats.size();
            meshStats.emplace_back();
            vertexDeclarations.push_back(vertexDeclaration);
        }

        groupMeshIndices[i] = meshIndex;

        const bool useQuad = groupMB.HasFlag(FGroup::Quad);
        const size_t vertexCountPerFace = (useQuad ? 4 : 3);

        FMeshStat& meshStat = meshStats[meshIndex];
        meshStat.IndexCount += groupMB.FaceCount * vertexCountPerFace;
        meshStat.VertexCount += groupMB.FaceCount * vertexCountPerFace;
        ++meshStat.SubPartCount;

        AssertRelease(!useQuad); // No support for quad list
        meshStat.PrimitiveType = Graphics::EPrimitiveType::TriangleList;
    }

    VECTOR_THREAD_LOCAL(MeshGeneration, PMaterial) materials;
    materials.resize(_materials.size());
    for (size_t i = 0; i < _materials.size(); ++i) {
        const FMaterial& materialMB = _materials[i];
        if (materialMB.HasFlag(FMaterial::_InUse))
            materials[i] = CreateMaterial_(materialMB);
    }

    struct FMeshData {
        size_t IndexOffset;
        size_t VertexOffset;
        MeshRawData Indices;
        MeshRawData Vertices;
        FMeshData() : IndexOffset(0), VertexOffset(0) {}
    };
    VECTOR_THREAD_LOCAL(MeshGeneration, FMeshData) meshDatas;
    meshDatas.resize(meshStats.size());
    for (size_t i = 0; i < meshStats.size(); ++i) {
        FMeshData& meshData = meshDatas[i];
        FMeshStat& meshStat = meshStats[i];
        meshData.Indices.Resize_DiscardData(sizeof(u32) * meshStat.IndexCount);
        meshData.Vertices.Resize_DiscardData(vertexDeclarations[i]->SizeInBytes() * meshStat.VertexCount);
    }

    VECTOR_THREAD_LOCAL(MeshGeneration, AABB3f) boneAABBs;
    boneAABBs.resize(_bones.size());
    VECTOR_THREAD_LOCAL(MeshGeneration, AABB3f) groupAABBs;
    groupAABBs.resize(_groups.size());

    for (size_t i = 0; i < _groups.size(); ++i) {
        const FGroup& groupMB = _groups[i];
        const size_t meshIndex = groupMeshIndices[i];
        FMeshData& meshData = meshDatas[meshIndex];

        AABB3f& boundingBox = groupAABBs[i];
        WriteGroupModelMesh_(   &meshData.IndexOffset, &meshData.VertexOffset,
                                boundingBox,
                                meshData.Indices, meshData.Vertices,
                                *this, groupMB,
                                vertexDeclarations[meshIndex].get() );

        boneAABBs[groupMB.Bone].Add(boundingBox);
    }

    VECTOR(Mesh, PModelMesh) modelMeshes;
    modelMeshes.reserve(meshDatas.size());
    for (size_t i = 0; i < meshDatas.size(); ++i) {
        FMeshData& meshData = meshDatas[i];
        FMeshStat& meshStat = meshStats[i];

        const TMemoryView<u32> indices = meshData.Indices.MakeView().Cast<u32>();

        FGenericVertex genericVertex(vertexDeclarations[i].get());
        genericVertex.SetDestination(meshData.Vertices.MakeView());
        genericVertex.SeekVertex(meshStat.VertexCount);

        MergeDuplicateVertices(genericVertex, indices);
        if (genericVertex.VertexCountWritten() != meshStat.VertexCount) {
            meshStat.VertexCount = genericVertex.VertexCountWritten();
            meshData.Vertices.Resize_KeepData(meshStat.VertexCount * genericVertex.VertexDeclaration()->SizeInBytes());
            genericVertex.SetDestination(meshData.Vertices.MakeView());
            genericVertex.SeekVertex(meshStat.VertexCount);
        }

        const float ACMR0 = VertexAverageCacheMissRate(indices);

        if (genericVertex.Tangent3f(0) || genericVertex.Tangent4f(0))
            ComputeTangentSpace(genericVertex, indices.Cast<const u32>());

        VECTOR(Mesh, PModelMeshSubPart) subParts;
        subParts.reserve(meshStat.SubPartCount);
        size_t indexOffset = 0;
        for (size_t j = 0; j < _groups.size(); ++j) {
            const FGroup& groupMB = _groups[j];
            if (groupMeshIndices[j] != i)
                continue;

            const bool useQuad = groupMB.HasFlag(FGroup::Quad);
            const size_t vertexCountPerFace = (useQuad ? 4 : 3);

            const size_t firstIndex = indexOffset;
            const size_t indexCount = groupMB.FaceCount * vertexCountPerFace;
            indexOffset += indexCount;

            OptimizeIndicesOrder(indices.SubRange(firstIndex, indexCount), genericVertex.VertexCountWritten());

            const PMaterial& material = materials[groupMB.Material];

            LOG(Info, L"[FModel] {0}/{1}: material = '{2}'", _name, groupMB.Name, material->Description() );

            subParts.emplace_back(new FModelMeshSubPart(
                FMeshName(groupMB.Name),
                checked_cast<u32>(groupMB.Bone),
                checked_cast<u32>(0),
                checked_cast<u32>(firstIndex),
                checked_cast<u32>(indexCount),
                groupAABBs[j],
                material ));
        }
        Assert(subParts.size() == meshStat.SubPartCount);

        OptimizeVerticesOrder(genericVertex, indices);

        const float ACMR1 = VertexAverageCacheMissRate(indices);

        LOG(Warning, L"[FModel] {0}/{1}: Optimized mesh average cache miss rate from {2}% to {3}%",
            _name, genericVertex.VertexDeclaration()->ToString(), ACMR0, ACMR1 );

        const Graphics::IndexElementSize indexElementSize = (meshStat.VertexCount <= UINT16_MAX
            ? Graphics::IndexElementSize::SixteenBits
            : Graphics::IndexElementSize::ThirtyTwoBits );

        if (Graphics::IndexElementSize::SixteenBits == indexElementSize) {
            MeshRawData indices16bits;
            indices16bits.Resize_DiscardData(meshStat.IndexCount * sizeof(u16));

            u32 *const pIndex32 = reinterpret_cast<u32 *>(meshData.Indices.Pointer());
            u16 *const pIndex16 = reinterpret_cast<u16 *>(indices16bits.Pointer());
            for (size_t j = 0; j < meshStat.IndexCount; ++j)
                pIndex16[j] = checked_cast<u16>(pIndex32[j]);

            meshData.Indices = std::move(indices16bits);
        }

        modelMeshes.emplace_back(new FModelMesh(
            checked_cast<u32>(meshStat.IndexCount),
            checked_cast<u32>(meshStat.VertexCount),
            meshStat.PrimitiveType,
            indexElementSize,
            vertexDeclarations[i].get(),
            std::move(meshData.Indices),
            std::move(meshData.Vertices),
            std::move(subParts) ));
    }
    Assert(modelMeshes.size() == meshDatas.size());


    const size_t boneCount = _bones.size();
    VECTOR(Mesh, PModelBone) bones;
    bones.reserve(boneCount);
    AABB3f modelBoundingBox;
    for (size_t b = 0; b < boneCount; ++b) {
        const AABB3f& boneAABB = boneAABBs[b];
        modelBoundingBox.Add(boneAABB);
        bones.push_back(CreateBone_(_bones[b], boneAABB));
    }

    return new FModel(FMeshName(_name), modelBoundingBox, std::move(bones), std::move(modelMeshes));
}
//----------------------------------------------------------------------------
void FModelBuilder::Clear() {
    Assert(!_openGroup);
    Assert(!_openMaterial);

    _name.clear();

    _positions.clear();
    _colors.clear();
    _texcoords.clear();
    _normals.clear();

    _bones.clear();
    _faces.clear();
    _groups.clear();
    _materials.clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
