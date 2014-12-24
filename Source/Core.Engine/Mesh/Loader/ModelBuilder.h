#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Color/Color.h"
#include "Core/Container/Vector.h"
#include "Core/IO/FS/Filename.h"
#include "Core/IO/String.h"
#include "Core/Maths/Geometry/ScalarVector_fwd.h"

namespace Core {
namespace Engine {
FWD_REFPTR(Model);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ModelBuilder {
public:
    template <typename _Tag, typename _N = u32>
    struct TypedIndex {
        _N Value;
        TypedIndex() : Value(0) {}
        TypedIndex(_N value) : Value(value) {}
        TypedIndex& operator =(_N value) { Value = value; return this; }
        STATIC_CONST_INTEGRAL(u32, InvalidValue, 0xFEE1DEAD);
        operator _N () const { Assert(InvalidValue != Value); return Value; }
#ifdef WITH_CORE_ASSERT
        static void Uninitialized(TypedIndex (&dst)[4]) { dst[0] = dst[1] = dst[2] = dst[3] = InvalidValue; }
        static void Assign(TypedIndex (&dst)[4], const TypedIndex (&src)[3]) { dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = InvalidValue; }
#else
        static void Uninitialized(TypedIndex (&)[4]) { }
        static void Assign(TypedIndex (&dst)[4], const TypedIndex (&src)[3]) { dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; }
#endif
        static void Assign(TypedIndex (&dst)[4], const TypedIndex (&src)[4]) { dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; }
    };

    struct PositionTag  {};
    struct NormalTag    {};
    struct TexcoordTag  {};

    using PositionIndex = TypedIndex<PositionTag>;
    using NormalIndex   = TypedIndex<NormalTag>;
    using TexcoordIndex = TypedIndex<TexcoordTag>;

    struct Face {
        PositionIndex P[4];
        NormalIndex N[4];
        TexcoordIndex T[4];

        Face(const PositionIndex (&pos)[3]) { PositionIndex::Assign(P, pos); NormalIndex::Uninitialized(N); TexcoordIndex::Uninitialized(T); }
        Face(const PositionIndex (&pos)[3], const TexcoordIndex (&texcoords)[3]) { PositionIndex::Assign(P, pos); NormalIndex::Uninitialized(N); TexcoordIndex::Assign(T, texcoords); }
        Face(const PositionIndex (&pos)[3], const TexcoordIndex (&texcoords)[3], const NormalIndex (&normals)[3]) { PositionIndex::Assign(P, pos); NormalIndex::Assign(N, normals); TexcoordIndex::Assign(T, texcoords); }
        Face(const PositionIndex (&pos)[3], const NormalIndex (&normals)[3]) { PositionIndex::Assign(P, pos); NormalIndex::Assign(N, normals); TexcoordIndex::Uninitialized(T); }

        Face(const PositionIndex (&pos)[4]) { PositionIndex::Assign(P, pos); NormalIndex::Uninitialized(N); TexcoordIndex::Uninitialized(T); }
        Face(const PositionIndex (&pos)[4], const TexcoordIndex (&texcoords)[4]) { PositionIndex::Assign(P, pos); NormalIndex::Uninitialized(N); TexcoordIndex::Assign(T, texcoords); }
        Face(const PositionIndex (&pos)[4], const TexcoordIndex (&texcoords)[4], const NormalIndex (&normals)[4]) { PositionIndex::Assign(P, pos); NormalIndex::Assign(N, normals); TexcoordIndex::Assign(T, texcoords); }
        Face(const PositionIndex (&pos)[4], const NormalIndex (&normals)[4]) { PositionIndex::Assign(P, pos); NormalIndex::Assign(N, normals); TexcoordIndex::Uninitialized(T); }
    };

    struct Group {
        enum Flags : u32 {
            Default     = 0,

            Normals     = 1<<0,
            Quad        = 1<<1,
            Smooth      = 1<<2,
            Texcoords   = 1<<3,

            Triangle = Default,
            Triangle_Normals = Default|Normals,
            Triangle_Texcoords = Default|Texcoords,
            Triangle_Texcoords_Normals = Triangle_Texcoords|Normals,

            Quad_Normals = Quad|Normals,
            Quad_Texcoords = Quad|Texcoords,
            Quad_Texcoords_Normals = Quad_Texcoords|Normals,
        };

        String Name;

        u32 Start;
        u32 Count;

        u32 Material    : 28;
        u32 Mode        : 4;

        bool HasFlag(Flags flag) const { return flag == (Mode & flag); }
        void SetFlag(Flags flag) { Mode |= flag; }

        void SetMode_CheckCoherency(Flags mode) { Assert(Default == Mode || mode == Mode); Mode = mode; }

        Group(const char *name) : Name(name), Start(0), Count(0), Material(0), Mode(Default) { Assert(name); }
    };

    struct Material {
        enum Flags : u32 {
            Default         = 0,

            Ambient         = 1<<0,
            Color           = 1<<1,
            Highlight       = 1<<2,
            Reflection      = 1<<3,
            Transparency    = 1<<4,
            Glass           = 1<<5,
            Fresnel         = 1<<6,
            CastShadows     = 1<<7,
        };

        String Name;

        ColorRGBAF AmbientColor;
        ColorRGBAF DiffuseColor;
        ColorRGBAF SpecularColor;

        Filename AlphaMap;
        Filename AmbientMap;
        Filename DiffuseMap;
        Filename DisplacementMap;
        Filename NormalMap;
        Filename SpecularColorMap;
        Filename SpecularPowerMap;

        u32 Mode;

        bool HasFlag(Flags flag) const { return flag == (Mode & flag); }
        void SetFlag(Flags flag) { Mode |= flag; }

        Material(const char *name) : Name(name), Mode(Default) { Assert(name); }
    };

    ModelBuilder();
    ~ModelBuilder();

    const String& Name() const { return _name; }

    const VECTOR(Mesh, float4)& Positions() const { return _positions; }
    const VECTOR(Mesh, ColorRGBAF)& Colors() const { return _colors; }
    const VECTOR(Mesh, float3)& Texcoords() const { return _texcoords; }
    const VECTOR(Mesh, float3)& Normals() const { return _normals; }

    const VECTOR(Mesh, Face)& Faces() const { return _faces; }
    const VECTOR(Mesh, Group)& Groups() const { return _groups; }

    void SetName(const char *name);

    void AddPosition(const float3& xyz);
    void AddPosition(const float4& xyzw);
    void AddColor(const ColorRGBAF& rgba);
    void AddTexCoord(const float2& uv);
    void AddTexCoord(const float3& uvw);
    void AddNormal(const float3& value);

    void AddTriangle(const PositionIndex (&pos)[3]);
    void AddTriangle(const PositionIndex (&pos)[3], const TexcoordIndex (&texcoords)[3]);
    void AddTriangle(const PositionIndex (&pos)[3], const TexcoordIndex (&texcoords)[3], const NormalIndex (&normals)[3]);
    void AddTriangle(const PositionIndex (&pos)[3], const NormalIndex (&normals)[3]);

    void AddQuad(const PositionIndex (&pos)[4]);
    void AddQuad(const PositionIndex (&pos)[4], const TexcoordIndex (&texcoords)[4]);
    void AddQuad(const PositionIndex (&pos)[4], const TexcoordIndex (&texcoords)[4], const NormalIndex (&normals)[4]);
    void AddQuad(const PositionIndex (&pos)[4], const NormalIndex (&normals)[4]);

    Group *OpenGroup(const char *name);
    const Group& OpenedGroup() const { Assert(_openGroup); return _groups.back(); }
    void CloseGroup(Group *group);

    Material *OpenMaterial(const char *name);
    const Material& OpenedMaterial() const { Assert(_openMaterial); return _materials.back(); }
    void CloseMaterial(Material *material);

    PModel CreateModel();
    void Clear();

private:
    Group& OpenedGroup() { Assert(_openGroup); return _groups.back(); }
    Material& OpenedMaterial() { Assert(_openMaterial); return _materials.back(); }

    String _name;

#ifdef WITH_CORE_ASSERT
    bool _openGroup     : 1;
    bool _openMaterial  : 1;
#endif

    VECTOR(Mesh, float4) _positions;
    VECTOR(Mesh, ColorRGBAF) _colors;
    VECTOR(Mesh, float3) _texcoords;
    VECTOR(Mesh, float3) _normals;

    VECTOR(Mesh, Face) _faces;
    VECTOR(Mesh, Group) _groups;
    VECTOR(Mesh, Material) _materials;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
