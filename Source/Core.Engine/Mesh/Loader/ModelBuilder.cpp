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
static PMaterial CreateMaterial_(const ModelBuilder::Material& materialMB) {
    Assert(materialMB.Name.size());

    VECTOR(Material, Graphics::BindName) tags;
    ASSOCIATIVE_VECTOR(Material, Graphics::BindName, Filename) textures;
    ASSOCIATIVE_VECTOR(Material, Graphics::BindName, PMaterialParameter) parameters;

    tags.reserve(Meta::BitSetsCount(materialMB.Mode));
    if (materialMB.HasFlag(ModelBuilder::Material::Ambient))
        tags.push_back(MaterialConstNames::Ambient());
    if (materialMB.HasFlag(ModelBuilder::Material::BumpMapping))
        tags.push_back(MaterialConstNames::BumpMapping());
    if (materialMB.HasFlag(ModelBuilder::Material::CastShadows))
        tags.push_back(MaterialConstNames::CastShadows());
    if (materialMB.HasFlag(ModelBuilder::Material::Color))
        tags.push_back(MaterialConstNames::Color());
    if (materialMB.HasFlag(ModelBuilder::Material::Emissive))
        tags.push_back(MaterialConstNames::Emissive());
    if (materialMB.HasFlag(ModelBuilder::Material::Fresnel))
        tags.push_back(MaterialConstNames::Fresnel());
    if (materialMB.HasFlag(ModelBuilder::Material::Glass))
        tags.push_back(MaterialConstNames::Glass());
    if (materialMB.HasFlag(ModelBuilder::Material::Highlight))
        tags.push_back(MaterialConstNames::Highlight());
    if (materialMB.HasFlag(ModelBuilder::Material::Reflection))
        tags.push_back(MaterialConstNames::Reflection());
    if (materialMB.HasFlag(ModelBuilder::Material::Refraction))
        tags.push_back(MaterialConstNames::Refraction());
    if (materialMB.HasFlag(ModelBuilder::Material::SeparateAlpha))
        tags.push_back(MaterialConstNames::SeparateAlpha());
    if (materialMB.HasFlag(ModelBuilder::Material::Transparency))
        tags.push_back(MaterialConstNames::Transparency());

    textures.reserve(7);
    if (!materialMB.AlphaMap.empty())
        textures.Insert_AssertUnique(MaterialConstNames::AlphaMap(), materialMB.AlphaMap);
    if (!materialMB.AmbientMap.empty())
        textures.Insert_AssertUnique(MaterialConstNames::AmbientMap(), materialMB.AmbientMap);
    if (!materialMB.DiffuseMap.empty())
        textures.Insert_AssertUnique(MaterialConstNames::DiffuseMap(), materialMB.DiffuseMap);
    if (!materialMB.DisplacementMap.empty())
        textures.Insert_AssertUnique(MaterialConstNames::DisplacementMap(), materialMB.DisplacementMap);
    if (!materialMB.EmissiveMap.empty())
        textures.Insert_AssertUnique(MaterialConstNames::EmissiveMap(), materialMB.EmissiveMap);
    if (!materialMB.NormalMap.empty())
        textures.Insert_AssertUnique(MaterialConstNames::NormalMap(), materialMB.NormalMap);
    if (!materialMB.ReflectionMap.empty())
        textures.Insert_AssertUnique(MaterialConstNames::ReflectionMap(), materialMB.ReflectionMap);
    if (!materialMB.SpecularColorMap.empty())
        textures.Insert_AssertUnique(MaterialConstNames::SpecularColorMap(), materialMB.SpecularColorMap);
    if (!materialMB.SpecularPowerMap.empty())
        textures.Insert_AssertUnique(MaterialConstNames::SpecularPowerMap(), materialMB.SpecularPowerMap);

    // TODO : MaterialParameterConstantFactory to merge constants by value

    parameters.reserve(4);
    if (materialMB.AmbientColor.a() >= 0)
        parameters.Insert_AssertUnique(MaterialConstNames::AmbientColor(), new MaterialParameterConstant<float4>(materialMB.AmbientColor.Data()));
    if (materialMB.DiffuseColor.a() >= 0)
        parameters.Insert_AssertUnique(MaterialConstNames::DiffuseColor(), new MaterialParameterConstant<float4>(materialMB.DiffuseColor.Data()));
    if (materialMB.EmissiveColor.a() >= 0)
        parameters.Insert_AssertUnique(MaterialConstNames::EmissiveColor(), new MaterialParameterConstant<float4>(materialMB.EmissiveColor.Data()));
    if (materialMB.SpecularColor.a() >= 0)
        parameters.Insert_AssertUnique(MaterialConstNames::SpecularColor(), new MaterialParameterConstant<float4>(materialMB.SpecularColor.Data()));

    if (materialMB.NormalDepth >= 0)
        parameters.Insert_AssertUnique(MaterialConstNames::NormalDepth(), new MaterialParameterConstant<float>(materialMB.NormalDepth));
    if (materialMB.RefractiveIndex >= 0)
        parameters.Insert_AssertUnique(MaterialConstNames::RefractiveIndex(), new MaterialParameterConstant<float>(materialMB.RefractiveIndex));
    if (materialMB.SpecularExponent >= 0)
        parameters.Insert_AssertUnique(MaterialConstNames::SpecularExponent(), new MaterialParameterConstant<float>(materialMB.SpecularExponent));

    LOG(Info, L"[Model] Loaded material '{0}' with diffuse map '{1}'", materialMB.Name, materialMB.DiffuseMap);

    return new Material("Standard", materialMB.Name, std::move(tags), std::move(textures), std::move(parameters));
}
//----------------------------------------------------------------------------
static PModelBone CreateBone_(const ModelBuilder::Bone& boneMB, const AABB3f& boundingBox) {
    const MeshName name(boneMB.Name);
    return new ModelBone(name, name, boneMB.Transform, boundingBox);
}
//----------------------------------------------------------------------------
static const Graphics::VertexDeclaration *GetVertexDeclaration_(const ModelBuilder::Group::Flags mode, bool useNormalMap, bool useVertexColor) {
    using namespace Graphics::Vertex;

    if (ModelBuilder::Group::Texcoords_Normals == (mode & ModelBuilder::Group::Texcoords_Normals)) {

        if (useNormalMap && useVertexColor)
            return Position0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N::Declaration;
        else if (useNormalMap)
            return Position0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N::Declaration;
        else if (useVertexColor)
            return Position0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N::Declaration;
        else
            return Position0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N::Declaration;

    }
    else if (ModelBuilder::Group::Texcoords == (mode & ModelBuilder::Group::Texcoords)) {
        AssertRelease(!useNormalMap); // impossible without normals

        if (useVertexColor)
            return Position0_Float3__Color0_UByte4N__TexCoord0_Half2::Declaration;
        else
            return Position0_Float3__TexCoord0_Half2::Declaration;

    }
    else if (ModelBuilder::Group::Normals == (mode & ModelBuilder::Group::Normals)) {
        AssertRelease(!useNormalMap); // impossible without texcoords

        if (useVertexColor)
            return Position0_Float3__Color0_UByte4N__Normal0_UX10Y10Z10W2N::Declaration;
        else
            return Position0_Float3__Normal0_UX10Y10Z10W2N::Declaration;
    }
    else {
        AssertRelease(!useNormalMap); // impossible without texcoords or normals

        if (useVertexColor)
            return Position0_Float3__Color0_UByte4N::Declaration;
        else
            return Position0_Float3::Declaration;
    }
}
//----------------------------------------------------------------------------
static const Graphics::VertexDeclaration *GetVertexDeclaration_(
    const ModelBuilder::Group& groupMB, 
    const ModelBuilder::Material& materialMB, 
    bool useVertexColor) {
    const bool useNormalMap = !materialMB.NormalMap.empty();
    return GetVertexDeclaration_(ModelBuilder::Group::Flags(groupMB.Mode), useNormalMap, useVertexColor);
}
//----------------------------------------------------------------------------
static void WriteGroupModelMesh_(
    size_t *pIndexOffset,
    size_t *pVertexOffset,
    AABB3f& boundingBox,
    MeshRawData& indexData,
    MeshRawData& vertexData,
    const ModelBuilder& mb,
    const ModelBuilder::Group& groupMB,
    const Graphics::VertexDeclaration *vertexDeclaration ) {
    Assert(pIndexOffset);
    Assert(pVertexOffset);

    const bool useQuads = groupMB.HasFlag(ModelBuilder::Group::Quad);
    const size_t vertexCountPerFace = useQuads ? 4 : 3;

    const size_t vertexCount = groupMB.FaceCount * vertexCountPerFace;
    const size_t vertexOffsetInBytes = *pVertexOffset * vertexDeclaration->SizeInBytes();
    const size_t vertexCountInBytes = vertexCount * vertexDeclaration->SizeInBytes();

    const MemoryView<u32> indices = indexData.MakeView().SubRange(*pIndexOffset * sizeof(u32), vertexCount * sizeof(u32)).Cast<u32>();

    const size_t firstIndex = *pIndexOffset;

    *pIndexOffset += vertexCount;
    *pVertexOffset += vertexCount;

    GenericVertex genericVertex(vertexDeclaration);
    genericVertex.SetDestination(vertexData.MakeView().SubRange(vertexOffsetInBytes, vertexCountInBytes));

    const GenericVertex::SubPart position0_3f = genericVertex.Position3f(0);
    const GenericVertex::SubPart position0_4f = genericVertex.Position4f(0);
    const GenericVertex::SubPart color0 = genericVertex.Color4f(0);
    const GenericVertex::SubPart texcoord0 = genericVertex.TexCoord2f(0);
    const GenericVertex::SubPart normal0 = genericVertex.Normal3f(0);
    Assert(position0_4f || position0_3f);

    size_t indexCount = 0;
    for (size_t f = 0; f < groupMB.FaceCount; ++f) {
        const ModelBuilder::Face& face = mb.Faces()[groupMB.FaceStart + f];

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
ModelBuilder::Face::Face(const PositionIndex (&pos)[3]) { 
    PositionIndex::Assign(P, pos); NormalIndex::Uninitialized(N); TexcoordIndex::Uninitialized(T); 
}
//----------------------------------------------------------------------------
ModelBuilder::Face::Face(const PositionIndex (&pos)[3], const TexcoordIndex (&texcoords)[3]) { 
    PositionIndex::Assign(P, pos); NormalIndex::Uninitialized(N); TexcoordIndex::Assign(T, texcoords); 
}
//----------------------------------------------------------------------------
ModelBuilder::Face::Face(const PositionIndex (&pos)[3], const TexcoordIndex (&texcoords)[3], const NormalIndex (&normals)[3]) { 
    PositionIndex::Assign(P, pos); NormalIndex::Assign(N, normals); TexcoordIndex::Assign(T, texcoords); 
}
//----------------------------------------------------------------------------
ModelBuilder::Face::Face(const PositionIndex (&pos)[3], const NormalIndex (&normals)[3]) { 
    PositionIndex::Assign(P, pos); NormalIndex::Assign(N, normals); TexcoordIndex::Uninitialized(T); 
}
//----------------------------------------------------------------------------
ModelBuilder::Face::Face(const PositionIndex (&pos)[4]) { 
    PositionIndex::Assign(P, pos); NormalIndex::Uninitialized(N); TexcoordIndex::Uninitialized(T); 
}
//----------------------------------------------------------------------------
ModelBuilder::Face::Face(const PositionIndex (&pos)[4], const TexcoordIndex (&texcoords)[4]) { 
    PositionIndex::Assign(P, pos); NormalIndex::Uninitialized(N); TexcoordIndex::Assign(T, texcoords); 
}
//----------------------------------------------------------------------------
ModelBuilder::Face::Face(const PositionIndex (&pos)[4], const TexcoordIndex (&texcoords)[4], const NormalIndex (&normals)[4]) { 
    PositionIndex::Assign(P, pos); NormalIndex::Assign(N, normals); TexcoordIndex::Assign(T, texcoords); 
}
//----------------------------------------------------------------------------
ModelBuilder::Face::Face(const PositionIndex (&pos)[4], const NormalIndex (&normals)[4]) { 
    PositionIndex::Assign(P, pos); NormalIndex::Assign(N, normals); TexcoordIndex::Uninitialized(T); 
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ModelBuilder::ModelBuilder() 
:   _name()
#ifdef WITH_CORE_ASSERT
,   _openGroup(false)
,   _openMaterial(false)
#endif
{}
//----------------------------------------------------------------------------
ModelBuilder::~ModelBuilder() {
    Assert(!_openGroup);
    Assert(!_openMaterial);
}
//----------------------------------------------------------------------------
void ModelBuilder::SetName(String&& name) {
    Assert(name.size());
    _name = std::move(name);
}
//----------------------------------------------------------------------------
void ModelBuilder::AddPosition(const float3& xyz) {
    _positions.push_back(float4(xyz, 1.0f));
}
//----------------------------------------------------------------------------
void ModelBuilder::AddPosition(const float4& xyzw) {
    _positions.push_back(xyzw);
}
//----------------------------------------------------------------------------
void ModelBuilder::AddColor(const ColorRGBAF& rgba) {
    Assert(_colors.size() == _positions.size() - 1); // each position should have its color if set
    _colors.push_back(rgba);
}
//----------------------------------------------------------------------------
void ModelBuilder::AddTexCoord(const float2& uv) {
    _texcoords.push_back(float3(uv, 0.0f));
}
//----------------------------------------------------------------------------
void ModelBuilder::AddTexCoord(const float3& uvw) {
    _texcoords.push_back(uvw);
}
//----------------------------------------------------------------------------
void ModelBuilder::AddNormal(const float3& value) {
    _normals.push_back(value);
}
//----------------------------------------------------------------------------
void ModelBuilder::AddTriangle(const PositionIndex (&pos)[3]) {
    OpenedGroup_().SetMode_CheckCoherency(Group::Triangle);

    _faces.emplace_back(pos);
}
//----------------------------------------------------------------------------
void ModelBuilder::AddTriangle(const PositionIndex (&pos)[3], const TexcoordIndex (&texcoords)[3]) {
    OpenedGroup_().SetMode_CheckCoherency(Group::Triangle_Texcoords);

    _faces.emplace_back(pos, texcoords);
}
//----------------------------------------------------------------------------
void ModelBuilder::AddTriangle(const PositionIndex (&pos)[3], const TexcoordIndex (&texcoords)[3], const NormalIndex (&normals)[3]) {
    OpenedGroup_().SetMode_CheckCoherency(Group::Triangle_Texcoords_Normals);

    _faces.emplace_back(pos, texcoords, normals);
}
//----------------------------------------------------------------------------
void ModelBuilder::AddTriangle(const PositionIndex (&pos)[3], const NormalIndex (&normals)[3]) {
    OpenedGroup_().SetMode_CheckCoherency(Group::Triangle_Normals);

    _faces.emplace_back(pos, normals);
}
//----------------------------------------------------------------------------
void ModelBuilder::AddQuad(const PositionIndex (&pos)[4]) {
    OpenedGroup_().SetMode_CheckCoherency(Group::Quad);

    _faces.emplace_back(pos);
}
//----------------------------------------------------------------------------
void ModelBuilder::AddQuad(const PositionIndex (&pos)[4], const TexcoordIndex (&texcoords)[4]) {
    OpenedGroup_().SetMode_CheckCoherency(Group::Quad_Texcoords);

    _faces.emplace_back(pos, texcoords);
}
//----------------------------------------------------------------------------
void ModelBuilder::AddQuad(const PositionIndex (&pos)[4], const TexcoordIndex (&texcoords)[4], const NormalIndex (&normals)[4]) {
    OpenedGroup_().SetMode_CheckCoherency(Group::Quad_Texcoords_Normals);

    _faces.emplace_back(pos, texcoords, normals);
}
//----------------------------------------------------------------------------
void ModelBuilder::AddQuad(const PositionIndex (&pos)[4], const NormalIndex (&normals)[4]) {
    OpenedGroup_().SetMode_CheckCoherency(Group::Quad_Normals);

    _faces.emplace_back(pos, normals);
}
//----------------------------------------------------------------------------
void ModelBuilder::AddBone(String&& name, const float4x4& transform) {
    Assert(!name.empty());
    Assert(IsHomogeneous(transform));

    _bones.emplace_back(std::move(name), transform);
}
//----------------------------------------------------------------------------
ModelBuilder::Group *ModelBuilder::OpenGroup(String&& name) {
    Assert(!_openGroup);
    Assert(!_openMaterial);
#ifdef WITH_CORE_ASSERT
    _openGroup = true;
#endif
    _groups.emplace_back(std::move(name));
    Group *const group = &_groups.back();
    group->FaceStart = checked_cast<u32>(_faces.size());
    return &_groups.back();
}
//----------------------------------------------------------------------------
void ModelBuilder::CloseGroup(Group *group) {
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
ModelBuilder::Material *ModelBuilder::OpenMaterial(String&& name) {
    Assert(!_openGroup);
    Assert(!_openMaterial);
#ifdef WITH_CORE_ASSERT
    _openMaterial = true;
#endif
    _materials.emplace_back(std::move(name));
    return &_materials.back();
}
//----------------------------------------------------------------------------
void ModelBuilder::CloseMaterial(Material *material) {
    Assert(!_openGroup);
    Assert(_openMaterial);
    Assert(&_materials.back() == material);
#ifdef WITH_CORE_ASSERT
    _openMaterial = false;
#endif
}
//----------------------------------------------------------------------------
bool ModelBuilder::MaterialIndexFromName(size_t *pIndex, const StringSlice& name) const {
    Assert(pIndex);
    Assert(!name.empty());

    const size_t materialCount = _materials.size();
    for (size_t i = 0; i < materialCount; ++i) {
        const Material& materialMB = _materials[i];
        if (materialMB.Name.size() == name.size() &&
            0 == CompareN(materialMB.Name.c_str(), name.Pointer(), name.size())) {
            *pIndex = i;
            return true;
        }
    }

    return false;
}
//----------------------------------------------------------------------------
PModel ModelBuilder::CreateModel() {
    Assert(_positions.size());
    Assert(_colors.empty() || _colors.size() == _positions.size());

    struct MeshStat {
        size_t IndexCount;
        size_t VertexCount;
        size_t SubPartCount;
        Graphics::PrimitiveType PrimitiveType;
        MeshStat() : IndexCount(0), VertexCount(0), SubPartCount(0) {}
    };
    const bool useVertexColor = (_positions.size() == _colors.size());
    const auto groupMeshIndices = MALLOCA_VIEW(size_t, _groups.size());
    VECTOR_THREAD_LOCAL(MeshGeneration, MeshStat) meshStats;
    VECTOR_THREAD_LOCAL(MeshGeneration, Graphics::PCVertexDeclaration) vertexDeclarations;
    for (size_t i = 0; i < _groups.size(); ++i) {
        const Group& groupMB = _groups[i];
        Assert(groupMB.Bone < _bones.size());
        Assert(groupMB.Material < _materials.size());
        Assert(groupMB.FaceStart + groupMB.FaceCount <= _faces.size());

        Material& materialMB = _materials[groupMB.Material];
        materialMB.SetFlag(Material::_InUse);

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

        const bool useQuad = groupMB.HasFlag(Group::Quad);
        const size_t vertexCountPerFace = (useQuad ? 4 : 3);

        MeshStat& meshStat = meshStats[meshIndex];
        meshStat.IndexCount += groupMB.FaceCount * vertexCountPerFace;
        meshStat.VertexCount += groupMB.FaceCount * vertexCountPerFace;
        ++meshStat.SubPartCount;

        AssertRelease(!useQuad); // No support for quad list
        meshStat.PrimitiveType = Graphics::PrimitiveType::TriangleList;
    }

    VECTOR_THREAD_LOCAL(MeshGeneration, PMaterial) materials;
    materials.resize(_materials.size());
    for (size_t i = 0; i < _materials.size(); ++i) {
        const Material& materialMB = _materials[i];
        if (materialMB.HasFlag(Material::_InUse))
            materials[i] = CreateMaterial_(materialMB);
    }

    struct MeshData {
        size_t IndexOffset;
        size_t VertexOffset;
        MeshRawData Indices;
        MeshRawData Vertices;
        MeshData() : IndexOffset(0), VertexOffset(0) {}
    };
    VECTOR_THREAD_LOCAL(MeshGeneration, MeshData) meshDatas;
    meshDatas.resize(meshStats.size());
    for (size_t i = 0; i < meshStats.size(); ++i) {
        MeshData& meshData = meshDatas[i];
        MeshStat& meshStat = meshStats[i];
        meshData.Indices.Resize_DiscardData(sizeof(u32) * meshStat.IndexCount);
        meshData.Vertices.Resize_DiscardData(vertexDeclarations[i]->SizeInBytes() * meshStat.VertexCount);
    }

    VECTOR_THREAD_LOCAL(MeshGeneration, AABB3f) boneAABBs;
    boneAABBs.resize(_bones.size());
    VECTOR_THREAD_LOCAL(MeshGeneration, AABB3f) groupAABBs;
    groupAABBs.resize(_groups.size());

    for (size_t i = 0; i < _groups.size(); ++i) {
        const Group& groupMB = _groups[i];
        const size_t meshIndex = groupMeshIndices[i];
        MeshData& meshData = meshDatas[meshIndex];

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
        MeshData& meshData = meshDatas[i];
        MeshStat& meshStat = meshStats[i];

        const MemoryView<u32> indices = meshData.Indices.MakeView().Cast<u32>();

        GenericVertex genericVertex(vertexDeclarations[i].get());
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
            const Group& groupMB = _groups[j];
            if (groupMeshIndices[j] != i)
                continue;

            const bool useQuad = groupMB.HasFlag(Group::Quad);
            const size_t vertexCountPerFace = (useQuad ? 4 : 3);

            const size_t firstIndex = indexOffset;
            const size_t indexCount = groupMB.FaceCount * vertexCountPerFace;
            indexOffset += indexCount;

            OptimizeIndicesOrder(indices.SubRange(firstIndex, indexCount), genericVertex.VertexCountWritten());

            const PMaterial& material = materials[groupMB.Material];

            LOG(Info, L"[Model] {0}/{1}: material = '{2}'", _name, groupMB.Name, material->Description() );

            subParts.emplace_back(new ModelMeshSubPart(
                MeshName(groupMB.Name),
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

        LOG(Warning, L"[Model] {0}/{1}: Optimized mesh average cache miss rate from {2}% to {3}%", 
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

        modelMeshes.emplace_back(new ModelMesh(
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

    return new Model(MeshName(_name), modelBoundingBox, std::move(bones), std::move(modelMeshes));
}
//----------------------------------------------------------------------------
void ModelBuilder::Clear() {
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
