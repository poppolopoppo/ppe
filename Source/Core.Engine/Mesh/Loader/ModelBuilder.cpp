#pragma once

#include "Engine.h"

#include "ModelBuilder.h"

#include "Core/Maths/Geometry/ScalarBoundingBox.h"
#include "Core/Maths/Geometry/ScalarVector.h"

#include "Core.Engine/Mesh/Model.h"
#include "Core.Engine/Mesh/ModelMesh.h"
#include "Core.Engine/Mesh/ModelMeshSubPart.h"
#include "Core.Engine/Mesh/ModelSubPart.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ModelBuilder::ModelBuilder() {}
//----------------------------------------------------------------------------
ModelBuilder::~ModelBuilder() {}
//----------------------------------------------------------------------------
void ModelBuilder::SetName(const char *name) {
    Assert(name);
    _name = name;
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
    OpenedGroup().SetMode_CheckCoherency(Group::Triangle);

    Assert(_positions.size() > pos[0]);
    Assert(_positions.size() > pos[1]);
    Assert(_positions.size() > pos[2]);

    _faces.emplace_back(pos);
}
//----------------------------------------------------------------------------
void ModelBuilder::AddTriangle(const PositionIndex (&pos)[3], const TexcoordIndex (&texcoords)[3]) {
    OpenedGroup().SetMode_CheckCoherency(Group::Triangle_Texcoords);

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
    OpenedGroup().SetMode_CheckCoherency(Group::Triangle_Texcoords_Normals);

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
    OpenedGroup().SetMode_CheckCoherency(Group::Triangle_Normals);

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
    OpenedGroup().SetMode_CheckCoherency(Group::Quad);

    Assert(_positions.size() > pos[0]);
    Assert(_positions.size() > pos[1]);
    Assert(_positions.size() > pos[2]);
    Assert(_positions.size() > pos[3]);

    _faces.emplace_back(pos);
}
//----------------------------------------------------------------------------
void ModelBuilder::AddQuad(const PositionIndex (&pos)[4], const TexcoordIndex (&texcoords)[4]) {
    OpenedGroup().SetMode_CheckCoherency(Group::Quad_Texcoords);

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
    OpenedGroup().SetMode_CheckCoherency(Group::Quad_Texcoords_Normals);

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
    OpenedGroup().SetMode_CheckCoherency(Group::Quad_Normals);

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
ModelBuilder::Group *ModelBuilder::OpenGroup(const char *name) {
    Assert(!_openGroup);
    Assert(!_openMaterial);
#ifdef WITH_CORE_ASSERT
    _openMaterial = true;
#endif
    _groups.emplace_back(name);
    Group *const group = &_groups.back();
    group->Start = _faces.size();
    return &_groups.back();
}
//----------------------------------------------------------------------------
void ModelBuilder::CloseGroup(Group *group) {
    Assert(_openGroup);
    Assert(!_openMaterial);
    Assert(group == &_groups.back());
#ifdef WITH_CORE_ASSERT
    _openGroup = false;
#endif
    Assert(_faces.size() >= group->Start);
    group->Count = _faces.size() - group->Start;
}
//----------------------------------------------------------------------------
ModelBuilder::Material *ModelBuilder::OpenMaterial(const char *name) {
    Assert(!_openGroup);
    Assert(!_openMaterial);
#ifdef WITH_CORE_ASSERT
    _openMaterial = true;
#endif
    _materials.emplace_back(name);
    return &_materials.back();
}
//----------------------------------------------------------------------------
void ModelBuilder::CloseMaterial(Material *material) {
    Assert(!_openGroup);
    Assert(_openMaterial);
    Assert(material == &_materials.back());
#ifdef WITH_CORE_ASSERT
    _openMaterial = false;
#endif
}
//----------------------------------------------------------------------------
PModel ModelBuilder::CreateModel() {


}
//----------------------------------------------------------------------------
void ModelBuilder::Clear() {
    Assert(!_openGroup);
    Assert(!_openMaterial);

    _positions.clear();
    _colors.clear();
    _texcoords.clear();
    _normals.clear();

    _faces.clear();
    _groups.clear();
    _materials.clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
