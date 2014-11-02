#include "stdafx.h"

#include "MeshLoader.h"

#include "Core.Graphics/Device/DeviceAPIEncapsulator.h"

#include "Core.Graphics/Device/Geometry/IndexBuffer.h"
#include "Core.Graphics/Device/Geometry/VertexDeclaration.h"

#include "Core/IO/VFS/VirtualFileSystemStream.h"
#include "Core/IO/VirtualFileSystem.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace PLY {
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
enum class Token : u32 {
    Ply = 0,
    Format,
    Comment,
    Element,
    Vertex,
    VertexIndices,
    Face,
    Property,
    List,
    EndHeader,

    Char,
    UChar,
    Short,
    UShort,
    Int,
    UInt,
    Float,
    Double,

    Ascii,
    BinaryLittleEndian,

    X,
    Y,
    Z,

    NX,
    NY,
    NZ,

    Red,
    Green,
    Blue,
    Alpha,

    TextureU,
    TextureV,
};
//----------------------------------------------------------------------------
static const char *gTokenToCStr[] = {
    "ply",
    "format",
    "comment",
    "element",
    "vertex",
    "vertex_indices",
    "face",
    "property",
    "list",
    "end_header",

    "char",
    "uchar",
    "short",
    "ushort",
    "int",
    "uint",
    "float",
    "double",

    "ascii",
    "binary_little_endian",

    "x",
    "y",
    "z",

    "nx",
    "ny",
    "nz",

    "red",
    "green",
    "blue",
    "alpha",

    "texture_u",
    "texture_v",
};
//----------------------------------------------------------------------------
enum class Property : u32 {
    PositionX   = 1<<0,
    PositionY   = 1<<1,
    PositionZ   = 1<<2,

    NormalX     = 1<<3,
    NormalY     = 1<<4,
    NormalZ     = 1<<5,

    ColorR      = 1<<6,
    ColorG      = 1<<7,
    ColorB      = 1<<8,
    ColorA      = 1<<9,

    TextureU    = 1<<10,
    TextureV    = 1<<11,

    PositionXYZ = PositionX|PositionY|PositionZ,
    NormalXYZ   = NormalX|NormalY|NormalZ,
    ColorRGBA   = ColorR|ColorG|ColorB|ColorA,
    TextureUV   = TextureU|TextureV,

    Position_Normal_Color = PositionXYZ|NormalXYZ|ColorRGBA,
    Position_Normal_Color_UV = PositionXYZ|NormalXYZ|ColorRGBA|TextureUV,
};
//----------------------------------------------------------------------------
static bool ScalarTypeSizeInBytes(u32 *size, Token type) {
    Assert(size);
    switch (type)
    {
    case Token::Char:
        *size = 1;
        return true;
    case Token::UChar:
        *size = 1;
        return true;
    case Token::Short:
        *size = 2;
        return true;
    case Token::UShort:
        *size = 2;
        return true;
    case Token::Int:
        *size = 4;
        return true;
    case Token::UInt:
        *size = 4;
        return true;
    case Token::Float:
        *size = 4;
        return true;
    case Token::Double:
        *size = 8;
        return true;

    default:
        break;
    }
    return false;
}
//----------------------------------------------------------------------------
static bool ReadToken(Token *token, IVirtualFileSystemIStream *stream) {
    Assert(token);
    Assert(stream);

    const std::streamoff offsetI = stream->TellI();

    char cstr[32];
    const size_t length = checked_cast<size_t>(stream->ReadWord(cstr));
    if (0 == length || length + 1 >= lengthof(cstr))
        return false;

    const std::locale& locale = std::locale::classic();

    for (size_t i = 0; i + 1 < length; ++i)
        cstr[i] = std::tolower(cstr[i], locale);

    for (size_t i = 0; i < lengthof(gTokenToCStr); ++i)
        if (0 == CompareNI(cstr, gTokenToCStr[i], length - 1)) {
            *token = Token(i);
            return true;
        }

    stream->SeekI(offsetI);
    return false;
}
//----------------------------------------------------------------------------
static bool ReadToken_SkipCommentIFN(Token *token, IVirtualFileSystemIStream *stream) {
    if (!ReadToken(token, stream))
        return false;

    if (*token != Token::Comment)
        return true;

    if (!stream->SeekI_FirstOf('\n')) // skip the rest of the line
        return false;

    return ReadToken_SkipCommentIFN(token, stream);
}
//----------------------------------------------------------------------------
static bool ReadInt(u32 *value, IVirtualFileSystemIStream *stream) {
    char cstr[80];
    const size_t length = checked_cast<size_t>(stream->ReadWord(cstr));

    if (0 == length || lengthof(cstr) == length + 1)
        return false;

    if (!Atoi<10>(value, cstr, length - 1))
        return false;

    return true;
}
//----------------------------------------------------------------------------
struct Vertex_Position_Normal_Color {
    float3 Position;
    float3 Normal;
    ColorRGBA Color;
};
STATIC_ASSERT(sizeof(Vertex_Position_Normal_Color) == 3*4+3*4+4);
//----------------------------------------------------------------------------
struct Vertex_Position_Normal_Color_UV {
    float3 Position;
    float3 Normal;
    ColorRGBA Color;
    float2 UV;
};
STATIC_ASSERT(sizeof(Vertex_Position_Normal_Color_UV) == 3*4+3*4+4+2*4);
//----------------------------------------------------------------------------
struct Face_Triangle {
    u32 A;
    u32 B;
    u32 C;
};
STATIC_ASSERT(sizeof(Face_Triangle) == 3*4);
//----------------------------------------------------------------------------
static void SanitizeNormal3_(float3& normal) {
    const float length = Length3(normal);
    normal = (std::abs(length) < F_Epsilon)
         ? float3(0, 1, 0)
         : normal / length;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
bool ReadMeshHeader(MeshHeader& header, IVirtualFileSystemIStream *stream) {
    Assert(stream);

    Token token;

    if (!ReadToken(&token, stream) || Token::Ply != token)
        return false;

    if (!ReadToken_SkipCommentIFN(&token, stream) || Token::Format != token)
        return false;
    if (!ReadToken(&token, stream) || Token::BinaryLittleEndian != token)
        return false;
    if (!stream->SeekI_FirstOf('\n'))
        return false;

    if (!ReadToken_SkipCommentIFN(&token, stream) || Token::Element != token)
        return false;
    if (!ReadToken(&token, stream) || Token::Vertex != token)
        return false;
    if (!ReadInt(&header.VertexCount, stream))
        return false;

    header.VertexStride = 0;
    Property vertexProperties = Property(0);
    do {
        if (!ReadToken_SkipCommentIFN(&token, stream))
            return false;

        if (Token::Property != token) {
            Assert(Token::Element == token);
            break;
        }

        Token propertyType;
        u32 propertySize;
        if (!ReadToken(&propertyType, stream) || !ScalarTypeSizeInBytes(&propertySize, propertyType))
            return false;

        if (!ReadToken(&token, stream))
            return false;

        Property propertySemantic;
        switch (token)
        {
        case Token::X:
            AssertRelease(propertyType == Token::Float);
            propertySemantic = Property::PositionX;
            break;
        case Token::Y:
            AssertRelease(propertyType == Token::Float);
            propertySemantic = Property::PositionY;
            break;
        case Token::Z:
            AssertRelease(propertyType == Token::Float);
            propertySemantic = Property::PositionZ;
            break;
        case Token::NX:
            AssertRelease(propertyType == Token::Float);
            propertySemantic = Property::NormalX;
            break;
        case Token::NY:
            AssertRelease(propertyType == Token::Float);
            propertySemantic = Property::NormalY;
            break;
        case Token::NZ:
            AssertRelease(propertyType == Token::Float);
            propertySemantic = Property::NormalZ;
            break;
        case Token::Red:
            AssertRelease(propertyType == Token::UChar);
            propertySemantic = Property::ColorR;
            break;
        case Token::Green:
            AssertRelease(propertyType == Token::UChar);
            propertySemantic = Property::ColorG;
            break;
        case Token::Blue:
            AssertRelease(propertyType == Token::UChar);
            propertySemantic = Property::ColorB;
            break;
        case Token::Alpha:
            AssertRelease(propertyType == Token::UChar);
            propertySemantic = Property::ColorA;
            break;
        case Token::TextureU:
            AssertRelease(propertyType == Token::Float);
            propertySemantic = Property::TextureU;
            break;
        case Token::TextureV:
            AssertRelease(propertyType == Token::Float);
            propertySemantic = Property::TextureV;
            break;
        default:
            return false;
        }

        header.VertexStride += propertySize;
        vertexProperties = Property(u32(vertexProperties)|u32(propertySemantic));
    }
    while (true);

    AssertRelease( (sizeof(Vertex_Position_Normal_Color) == header.VertexStride &&
                        Property::Position_Normal_Color == vertexProperties) ||
                   (sizeof(Vertex_Position_Normal_Color_UV) == header.VertexStride &&
                        Property::Position_Normal_Color_UV == vertexProperties ));

    if (Token::Element != token)
        return false;
    if (!ReadToken(&token, stream) || Token::Face != token)
        return false;
    u32 faceCount;
    if (!ReadInt(&faceCount, stream))
        return false;

    header.IndexCount = faceCount * 3 /* assuming triangles */;
    header.IndexStride = u32(Graphics::IndexElementSize::ThirtyTwoBits);

    if (!ReadToken_SkipCommentIFN(&token, stream) || Token::Property != token)
        return false;
    if (!ReadToken(&token, stream) || Token::List != token)
        return false;
    if (!ReadToken(&token, stream) || Token::UChar != token)
        return false;
    if (!ReadToken(&token, stream) || Token::Int != token)
        return false;
    if (!ReadToken(&token, stream) || Token::VertexIndices != token)
        return false;

    if (!ReadToken_SkipCommentIFN(&token, stream) || Token::EndHeader != token)
        return false;

    return true;
}
//----------------------------------------------------------------------------
bool ReadMeshData(const MeshHeader& header, const MemoryView<u8>& indices, GenericVertex& vertices, IVirtualFileSystemIStream *stream) {
    Assert(stream);
    Assert(indices.SizeInBytes() == header.IndexCount * header.IndexStride);
    Assert(vertices.VertexCountRemaining() == header.VertexCount);
    Assert( sizeof(Vertex_Position_Normal_Color) == header.VertexStride ||
            sizeof(Vertex_Position_Normal_Color_UV) == header.VertexStride );

    const GenericVertex::SubPart sp_positions0 = vertices.Position3f(0);
    const GenericVertex::SubPart sp_colors0 = vertices.Color4b(0);
    const GenericVertex::SubPart sp_normals0 = vertices.Normal3f(0);

    Assert(sp_positions0);
    Assert(sp_colors0);
    Assert(sp_normals0);

    if (sizeof(Vertex_Position_Normal_Color) == header.VertexStride) {
        Vertex_Position_Normal_Color vertexData;
        for (size_t i = 0; i < header.VertexCount; ++i) {
            if (!stream->ReadPOD(&vertexData))
                return false;

            SanitizeNormal3_(vertexData.Normal); // beurk

            sp_positions0.AssignValue(vertices, vertexData.Position);
            sp_colors0.AssignValue(vertices, vertexData.Color);
            sp_normals0.AssignValue(vertices, vertexData.Normal);

            if (!vertices.NextVertex())
                Assert(i + 1 == header.VertexCount);
        }
    }
    else {
        const GenericVertex::SubPart sp_texcoord0 = vertices.TexCoord2f(0);
        Assert(sp_texcoord0);

        Vertex_Position_Normal_Color_UV vertexData;
        for (size_t i = 0; i < header.VertexCount; ++i) {
            if (!stream->ReadPOD(&vertexData))
                return false;

            SanitizeNormal3_(vertexData.Normal); // beurk

            vertexData.UV.y() = 1 - vertexData.UV.y(); // double-beurk

            sp_positions0.AssignValue(vertices, vertexData.Position);
            sp_colors0.AssignValue(vertices, vertexData.Color);
            sp_texcoord0.AssignValue(vertices, vertexData.UV);
            sp_normals0.AssignValue(vertices, vertexData.Normal);

            if (!vertices.NextVertex())
                Assert(i + 1 == header.VertexCount);
        }
    }

    Assert(header.IndexStride == u32(Graphics::IndexElementSize::ThirtyTwoBits));
    const MemoryView<u32> indicesU32 = indices.Cast<u32>();
    Assert(indicesU32.size() == header.IndexCount);

    Face_Triangle faceData;

    for (size_t i = 0; i < header.IndexCount; i += 3) {
        u8 faceCount;
        if (!stream->ReadPOD(&faceCount))
            return false;

        Assert(3 == faceCount); // assuming triangle topology

        if (!stream->ReadPOD(&faceData))
            return false;

        Assert(faceData.A < header.VertexCount);
        Assert(faceData.B < header.VertexCount);
        Assert(faceData.C < header.VertexCount);

        indicesU32[i + 0] = faceData.A;
        indicesU32[i + 1] = faceData.B;
        indicesU32[i + 2] = faceData.C;
    }

    return true;
}
//----------------------------------------------------------------------------
} //!namespace PLY
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
