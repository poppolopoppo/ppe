#include "stdafx.h"

#include "Engine.h"

#include "ModelBuilder.h"

#include "Core/Maths/Geometry/ScalarBoundingBox.h"
#include "Core/Maths/Geometry/ScalarVector.h"
#include "Core/Maths/Transform/ScalarMatrixHelpers.h"
#include "Core/Meta/BitField.h"

#include "Core.Engine/Material/Material.h"
#include "Core.Engine/Material/MaterialConstNames.h"
#include "Core.Engine/Material/Parameters/MaterialParameterBlock.h"

#include "Core.Engine/Mesh/Model.h"
#include "Core.Engine/Mesh/ModelBone.h"
#include "Core.Engine/Mesh/ModelMesh.h"
#include "Core.Engine/Mesh/ModelMeshSubPart.h"

#include "Core.Engine/Mesh/Geometry/GenericVertex.h"
#include "Core.Engine/Mesh/Geometry/GenericVertexOptimizer.h"

#include "Core.Graphics/Device/Geometry/IndexElementSize.h"
#include "Core.Graphics/Device/Geometry/PrimitiveType.h"
#include "Core.Graphics/Device/Geometry/VertexDeclaration.h"
#include "Core.Graphics/Device/Geometry/VertexTypes.h"

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
    ASSOCIATIVE_VECTOR(Material, Graphics::BindName, PAbstractMaterialParameter) parameters;

    tags.reserve(Meta::BitSetsCount(materialMB.Mode));
    if (materialMB.HasFlag(ModelBuilder::Material::Ambient))
        tags.push_back(MaterialConstNames::Ambient());
    if (materialMB.HasFlag(ModelBuilder::Material::Color))
        tags.push_back(MaterialConstNames::Color());
    if (materialMB.HasFlag(ModelBuilder::Material::Emissive))
        tags.push_back(MaterialConstNames::Emissive());
    if (materialMB.HasFlag(ModelBuilder::Material::Highlight))
        tags.push_back(MaterialConstNames::Highlight());
    if (materialMB.HasFlag(ModelBuilder::Material::Reflection))
        tags.push_back(MaterialConstNames::Reflection());
    if (materialMB.HasFlag(ModelBuilder::Material::Transparency))
        tags.push_back(MaterialConstNames::Transparency());
    if (materialMB.HasFlag(ModelBuilder::Material::Glass))
        tags.push_back(MaterialConstNames::Glass());
    if (materialMB.HasFlag(ModelBuilder::Material::Fresnel))
        tags.push_back(MaterialConstNames::Fresnel());
    if (materialMB.HasFlag(ModelBuilder::Material::CastShadows))
        tags.push_back(MaterialConstNames::CastShadows());

    textures.reserve(7);
    if (!materialMB.AlphaMap.empty())
        textures.Insert_AssertUnique(MaterialConstNames::AlphaMap(), materialMB.AlphaMap);
    if (!materialMB.AmbientMap.empty())
        textures.Insert_AssertUnique(MaterialConstNames::AmbientMap(), materialMB.AmbientMap);
    if (!materialMB.DiffuseMap.empty())
        textures.Insert_AssertUnique(MaterialConstNames::DiffuseMap(), materialMB.DiffuseMap);
    if (!materialMB.DisplacementMap.empty())
        textures.Insert_AssertUnique(MaterialConstNames::DisplacementMap(), materialMB.DisplacementMap);
    if (!materialMB.NormalMap.empty())
        textures.Insert_AssertUnique(MaterialConstNames::NormalMap(), materialMB.NormalMap);
    if (!materialMB.SpecularColorMap.empty())
        textures.Insert_AssertUnique(MaterialConstNames::SpecularColorMap(), materialMB.SpecularColorMap);
    if (!materialMB.SpecularPowerMap.empty())
        textures.Insert_AssertUnique(MaterialConstNames::SpecularPowerMap(), materialMB.SpecularPowerMap);

    parameters.reserve(4);
    if (materialMB.AmbientColor.a() > 0)
        parameters.Insert_AssertUnique(MaterialConstNames::AmbientColor(), new MaterialParameterBlock<float4>(materialMB.AmbientColor.Data()));
    if (materialMB.DiffuseColor.a() > 0)
        parameters.Insert_AssertUnique(MaterialConstNames::DiffuseColor(), new MaterialParameterBlock<float4>(materialMB.DiffuseColor.Data()));
    if (materialMB.EmissiveColor.a() > 0)
        parameters.Insert_AssertUnique(MaterialConstNames::EmissiveColor(), new MaterialParameterBlock<float4>(materialMB.EmissiveColor.Data()));
    if (materialMB.SpecularColor.a() > 0)
        parameters.Insert_AssertUnique(MaterialConstNames::SpecularColor(), new MaterialParameterBlock<float4>(materialMB.SpecularColor.Data()));

    Assert(tags.capacity() == tags.size());
#if 1 // realloc vector ?
    textures.Vector().shrink_to_fit();
    parameters.Vector().shrink_to_fit();
#endif

    return new Material(materialMB.Name, std::move(tags), std::move(textures), std::move(parameters));
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
            return Position0_Float3__TexCoord0_Half2__Normal0_UByte4N::Declaration;

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
static PModelMesh CreateModelMesh_(
    const ModelBuilder& mb, 
    const MemoryView<PMaterial>& materials,
    const MemoryView<AABB3f>& boneAABBs,
    const ModelBuilder::Group& groupMB) {

    const bool useQuads = groupMB.HasFlag(ModelBuilder::Group::Quad);
    const size_t vertexCountPerFace = useQuads ? 4 : 3;

    const bool useVertexColor = (mb.Colors().size() == mb.Positions().size());

    size_t vertexCount = 0;
    bool useNormalMap = false;
    for (size_t s = 0; s < groupMB.SubPartCount; ++s) {
        const ModelBuilder::SubPart& subPartMB = mb.SubParts()[groupMB.SubPartStart + s];
        vertexCount += subPartMB.FaceCount * vertexCountPerFace; // will be merged afterwards
        const ModelBuilder::Material& materialMB = mb.Materials()[subPartMB.Material];
        useNormalMap |= (!materialMB.NormalMap.empty());
    }

    const Graphics::VertexDeclaration *vertexDeclaration = GetVertexDeclaration_(ModelBuilder::Group::Flags(groupMB.Mode), useNormalMap, useVertexColor);
    Assert(vertexDeclaration);

    GenericVertex genericVertex(vertexDeclaration);

    const GenericVertex::SubPart position0_3f = genericVertex.Position3f(0);
    const GenericVertex::SubPart position0_4f = genericVertex.Position4f(0);
    const GenericVertex::SubPart color0 = genericVertex.Color4f(0);
    const GenericVertex::SubPart texcoord0 = genericVertex.TexCoord2f(0);
    const GenericVertex::SubPart normal0 = genericVertex.Normal3f(0);
    Assert(position0_4f || position0_3f);

    MeshRawData vertices;
    vertices.Resize_DiscardData(vertexDeclaration->SizeInBytes() * vertexCount);
    genericVertex.SetDestination(vertices.MakeView());

    RAWSTORAGE_THREAD_LOCAL(Mesh, u32) indices;
    indices.Resize_DiscardData(vertexCount);

    VECTOR_THREAD_LOCAL(Mesh, AABB3f) subPartAABBs;
    subPartAABBs.resize(groupMB.SubPartCount);

    size_t indexCount = 0;
    for (size_t s = 0; s < groupMB.SubPartCount; ++s) {
        const ModelBuilder::SubPart& subPartMB = mb.SubParts()[groupMB.SubPartStart + s];

        AABB3f& subPartAABB = subPartAABBs[s];
        for (size_t f = 0; f < subPartMB.FaceCount; ++f) {
            const ModelBuilder::Face& face = mb.Faces()[subPartMB.FaceStart + f];

            for (size_t v = 0; v < vertexCountPerFace; ++v) {
                const float4& pos_4f = mb.Positions()[face.P[v]];
                const float3 pos_3f = pos_4f.xyz();

                subPartAABB.Add(pos_3f);

                genericVertex.ZeroMemory_CurrentVertex();

                if (position0_4f)
                    position0_4f.AssignValue(genericVertex, pos_4f);
                else
                    position0_3f.AssignValue(genericVertex, pos_3f);

                if (color0) color0.AssignValue(genericVertex, mb.Colors()[face.P[v]]);
                if (texcoord0) texcoord0.AssignValue(genericVertex, mb.Texcoords()[face.T[v]]);
                if (normal0) normal0.AssignValue(genericVertex, mb.Normals()[face.N[v]]);

                if (!genericVertex.NextVertex())
                    AssertNotReached();

                indices[indexCount] = checked_cast<u32>(indexCount);
                ++indexCount;
            }
        }

        AABB3f& boneAABB = boneAABBs[subPartMB.Bone];
        boneAABB.Add(subPartAABB);
    }

    MergeDuplicateVertices(genericVertex, indices.MakeView());
    OptimizeIndicesAndVerticesOrder(genericVertex, indices.MakeView());

    if (genericVertex.VertexCountWritten() != vertexCount) {
        vertexCount = genericVertex.VertexCountWritten();
        vertices.Resize_KeepData(vertexDeclaration->SizeInBytes() * vertexCount);
    }

    VECTOR(Mesh, PModelMeshSubPart) meshSubParts;
    meshSubParts.reserve(groupMB.SubPartCount);
    for (size_t s = 0; s < groupMB.SubPartCount; ++s) {
        const ModelBuilder::SubPart& subPartMB = mb.SubParts()[groupMB.SubPartStart + s];

        u32 baseVertex = checked_cast<u32>(vertexCount);
        if (useQuads)
            for (size_t f = 0; f < subPartMB.FaceCount; ++f) {
                const size_t i = f * 4;
                baseVertex = std::min(baseVertex, indices[i + 0]);
                baseVertex = std::min(baseVertex, indices[i + 1]);
                baseVertex = std::min(baseVertex, indices[i + 2]);
                baseVertex = std::min(baseVertex, indices[i + 3]);
            }
        else
            for (size_t f = 0; f < subPartMB.FaceCount; ++f) {
                const size_t i = f * 3;
                baseVertex = std::min(baseVertex, indices[i + 0]);
                baseVertex = std::min(baseVertex, indices[i + 1]);
                baseVertex = std::min(baseVertex, indices[i + 2]);
            }

        const AABB3f& subPartAABB = subPartAABBs[s];

        meshSubParts.emplace_back(new ModelMeshSubPart( materials[subPartMB.Material].get(), 
                                                        checked_cast<u32>(subPartMB.Bone),
                                                        checked_cast<u32>(baseVertex),
                                                        checked_cast<u32>(subPartMB.FaceStart * vertexCountPerFace),
                                                        checked_cast<u32>(subPartMB.FaceCount * vertexCountPerFace),
                                                        subPartAABB ));
    }

    const Graphics::IndexElementSize indexElementSize = (vertexCount <= UINT16_MAX
        ? Graphics::IndexElementSize::SixteenBits 
        : Graphics::IndexElementSize::ThirtyTwoBits );

    MeshRawData indices16Or32bits;
    if (Graphics::IndexElementSize::SixteenBits == indexElementSize) {
        indices16Or32bits.Resize_DiscardData(indices.size() * sizeof(u16));
        u16 *pIndex = reinterpret_cast<u16 *>(indices16Or32bits.Pointer());
        for (size_t i = 0; i < indexCount; ++i, ++pIndex)
            pIndex[i] = checked_cast<u16>(indices[i]);
    }
    else {
        indices16Or32bits.Resize_DiscardData(indices.size() * sizeof(u32));
        Assert(indices.SizeInBytes() == indices16Or32bits.SizeInBytes());
        memcpy(indices16Or32bits.Pointer(), indices.Pointer(), indices.SizeInBytes());
    }

    AssertRelease(!useQuads); // no primitive type for quads on graphics::device, triangulate quads ?
    const Graphics::PrimitiveType primitiveType = Graphics::PrimitiveType::TriangleList;

    return new ModelMesh(   checked_cast<u32>(indexCount),
                            checked_cast<u32>(vertexCount),
                            primitiveType,
                            indexElementSize,
                            vertexDeclaration,
                            std::move(indices16Or32bits),
                            std::move(vertices),
                            std::move(meshSubParts) );
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
,   _openSubPart(false)
#endif
{}
//----------------------------------------------------------------------------
ModelBuilder::~ModelBuilder() {
    Assert(!_openGroup);
    Assert(!_openMaterial);
    Assert(!_openSubPart);
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
    Assert(_openSubPart);
    OpenedGroup_().SetMode_CheckCoherency(Group::Triangle);

    Assert(_positions.size() > pos[0]);
    Assert(_positions.size() > pos[1]);
    Assert(_positions.size() > pos[2]);

    _faces.emplace_back(pos);
}
//----------------------------------------------------------------------------
void ModelBuilder::AddTriangle(const PositionIndex (&pos)[3], const TexcoordIndex (&texcoords)[3]) {
    Assert(_openSubPart);
    OpenedGroup_().SetMode_CheckCoherency(Group::Triangle_Texcoords);

    Assert(_positions.size() > pos[0]);
    Assert(_positions.size() > pos[1]);
    Assert(_positions.size() > pos[2]);

    Assert(_texcoords.size() > texcoords[0]);
    Assert(_texcoords.size() > texcoords[1]);
    Assert(_texcoords.size() > texcoords[2]);

    _faces.emplace_back(pos, texcoords);
}
//----------------------------------------------------------------------------
void ModelBuilder::AddTriangle(const PositionIndex (&pos)[3], const TexcoordIndex (&texcoords)[3], const NormalIndex (&normals)[3]) {
    Assert(_openSubPart);
    OpenedGroup_().SetMode_CheckCoherency(Group::Triangle_Texcoords_Normals);

    Assert(_positions.size() > pos[0]);
    Assert(_positions.size() > pos[1]);
    Assert(_positions.size() > pos[2]);

    Assert(_texcoords.size() > texcoords[0]);
    Assert(_texcoords.size() > texcoords[1]);
    Assert(_texcoords.size() > texcoords[2]);

    Assert(_normals.size() > normals[0]);
    Assert(_normals.size() > normals[1]);
    Assert(_normals.size() > normals[2]);

    _faces.emplace_back(pos, texcoords, normals);
}
//----------------------------------------------------------------------------
void ModelBuilder::AddTriangle(const PositionIndex (&pos)[3], const NormalIndex (&normals)[3]) {
    Assert(_openSubPart);
    OpenedGroup_().SetMode_CheckCoherency(Group::Triangle_Normals);

    Assert(_positions.size() > pos[0]);
    Assert(_positions.size() > pos[1]);
    Assert(_positions.size() > pos[2]);

    Assert(_normals.size() > normals[0]);
    Assert(_normals.size() > normals[1]);
    Assert(_normals.size() > normals[2]);

    _faces.emplace_back(pos, normals);
}
//----------------------------------------------------------------------------
void ModelBuilder::AddQuad(const PositionIndex (&pos)[4]) {
    Assert(_openSubPart);
    OpenedGroup_().SetMode_CheckCoherency(Group::Quad);

    Assert(_positions.size() > pos[0]);
    Assert(_positions.size() > pos[1]);
    Assert(_positions.size() > pos[2]);
    Assert(_positions.size() > pos[3]);

    _faces.emplace_back(pos);
}
//----------------------------------------------------------------------------
void ModelBuilder::AddQuad(const PositionIndex (&pos)[4], const TexcoordIndex (&texcoords)[4]) {
    Assert(_openSubPart);
    OpenedGroup_().SetMode_CheckCoherency(Group::Quad_Texcoords);

    Assert(_positions.size() > pos[0]);
    Assert(_positions.size() > pos[1]);
    Assert(_positions.size() > pos[2]);
    Assert(_positions.size() > pos[3]);

    Assert(_texcoords.size() > texcoords[0]);
    Assert(_texcoords.size() > texcoords[1]);
    Assert(_texcoords.size() > texcoords[2]);
    Assert(_texcoords.size() > texcoords[3]);

    _faces.emplace_back(pos, texcoords);
}
//----------------------------------------------------------------------------
void ModelBuilder::AddQuad(const PositionIndex (&pos)[4], const TexcoordIndex (&texcoords)[4], const NormalIndex (&normals)[4]) {
    Assert(_openSubPart);
    OpenedGroup_().SetMode_CheckCoherency(Group::Quad_Texcoords_Normals);

    Assert(_positions.size() > pos[0]);
    Assert(_positions.size() > pos[1]);
    Assert(_positions.size() > pos[2]);
    Assert(_positions.size() > pos[3]);

    Assert(_texcoords.size() > texcoords[0]);
    Assert(_texcoords.size() > texcoords[1]);
    Assert(_texcoords.size() > texcoords[2]);
    Assert(_texcoords.size() > texcoords[3]);

    Assert(_normals.size() > normals[0]);
    Assert(_normals.size() > normals[1]);
    Assert(_normals.size() > normals[2]);
    Assert(_normals.size() > normals[3]);

    _faces.emplace_back(pos, texcoords, normals);
}
//----------------------------------------------------------------------------
void ModelBuilder::AddQuad(const PositionIndex (&pos)[4], const NormalIndex (&normals)[4]) {
    Assert(_openSubPart);
    OpenedGroup_().SetMode_CheckCoherency(Group::Quad_Normals);

    Assert(_positions.size() > pos[0]);
    Assert(_positions.size() > pos[1]);
    Assert(_positions.size() > pos[2]);
    Assert(_positions.size() > pos[3]);

    Assert(_normals.size() > normals[0]);
    Assert(_normals.size() > normals[1]);
    Assert(_normals.size() > normals[2]);
    Assert(_normals.size() > normals[3]);

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
    Assert(!_openSubPart);
#ifdef WITH_CORE_ASSERT
    _openMaterial = true;
#endif
    _groups.emplace_back(std::move(name));
    Group *const group = &_groups.back();
    group->SubPartStart = checked_cast<u32>(_subParts.size());
    return &_groups.back();
}
//----------------------------------------------------------------------------
void ModelBuilder::CloseGroup(Group *group) {
    Assert(_openGroup);
    Assert(!_openMaterial);
    Assert(!_openSubPart);
    Assert(&_groups.back() == group);
#ifdef WITH_CORE_ASSERT
    _openGroup = false;
#endif
    Assert(_subParts.size() >= group->SubPartStart);
    group->SubPartCount = checked_cast<u32>(_faces.size() - group->SubPartStart);
}
//----------------------------------------------------------------------------
ModelBuilder::Material *ModelBuilder::OpenMaterial(String&& name) {
    Assert(!_openGroup);
    Assert(!_openMaterial);
    Assert(!_openSubPart);
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
    Assert(!_openSubPart);
    Assert(&_materials.back() == material);
#ifdef WITH_CORE_ASSERT
    _openMaterial = false;
#endif
}
//----------------------------------------------------------------------------
ModelBuilder::SubPart *ModelBuilder::OpenSubPart() {
    Assert(_openGroup);
    Assert(!_openMaterial);
    Assert(!_openSubPart);
#ifdef WITH_CORE_ASSERT
    _openSubPart = true;
#endif
    _subParts.emplace_back();
    _subParts.back().FaceStart = checked_cast<u32>(_faces.size());
    return &_subParts.back();
}
//----------------------------------------------------------------------------
void ModelBuilder::CloseSubPart(SubPart *subPart) {
    Assert(_openGroup);
    Assert(!_openMaterial);
    Assert(_openSubPart);
    Assert(&_subParts.back() == subPart);
#ifdef WITH_CORE_ASSERT
    _openSubPart = false;
#endif
    Assert(_faces.size() >= subPart->FaceStart);
    Assert(_bones.size() > subPart->Bone);
    Assert(_materials.size() > subPart->Material);
    subPart->FaceCount = checked_cast<u32>(_faces.size() - subPart->FaceStart);
}
//----------------------------------------------------------------------------
PModel ModelBuilder::CreateModel() {
    Assert(_colors.empty() || _colors.size() == _positions.size());

    VECTOR(Mesh, PMaterial) materials;
    materials.reserve(_materials.size());
    for (const Material& m : _materials)
        materials.push_back(CreateMaterial_(m));

    VECTOR_THREAD_LOCAL(Mesh, AABB3f) boneAABBs;
    boneAABBs.resize(_bones.size());

    VECTOR(Mesh, PModelMesh) meshes;
    meshes.reserve(_groups.size());
    for (const Group& g : _groups)
        meshes.push_back(CreateModelMesh_(*this, MakeView(materials), MakeView(boneAABBs), g));

    const size_t boneCount = _bones.size();
    VECTOR(Mesh, PModelBone) bones;
    bones.reserve(boneCount);
    AABB3f boundingBox;
    for (size_t b = 0; b < boneCount; ++b) {
        const AABB3f& boneAABB = boneAABBs[b];
        boundingBox.Add(boneAABB);
        bones.push_back(CreateBone_(_bones[b], boneAABB));
    }

    const MeshName name(_name);
    return new Model(name, boundingBox, std::move(bones), std::move(meshes));
}
//----------------------------------------------------------------------------
void ModelBuilder::Clear() {
    Assert(!_openGroup);
    Assert(!_openMaterial);
    Assert(!_openSubPart);

    _name.clear();

    _positions.clear();
    _colors.clear();
    _texcoords.clear();
    _normals.clear();

    _bones.clear();
    _faces.clear();
    _groups.clear();
    _materials.clear();
    _subParts.clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
