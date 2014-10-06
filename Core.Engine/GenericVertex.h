#pragma once

#include "Engine.h"

#include "Core/Color.h"
#include "Core/MemoryView.h"
#include "Core/ScalarVector_fwd.h"

namespace Core {
namespace Graphics {
class AbstractVertexSubPart;
FWD_REFPTR(VertexDeclaration);
struct VertexSubPartKey;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class GenericVertex {
public:
    struct SubPart {
        const Graphics::VertexSubPartKey *Key;
        const Graphics::AbstractVertexSubPart *Value;

        operator bool () const { return (nullptr != Key); }

        void AssignValue(GenericVertex& vertex, float value) const;
        void AssignValue(GenericVertex& vertex, const float2& value) const;
        void AssignValue(GenericVertex& vertex, const float3& value) const;
        void AssignValue(GenericVertex& vertex, const float4& value) const;

        void AssignValue(GenericVertex& vertex, const ColorRGBA& value) const;
        void AssignValue(GenericVertex& vertex, const ColorRGBA16& value) const;
        void AssignValue(GenericVertex& vertex, const ColorRGBAF& value) const;

        void AssignValue(GenericVertex& vertex, const ColorBGRA& value) const;
        void AssignValue(GenericVertex& vertex, const ColorBGRA16& value) const;
        void AssignValue(GenericVertex& vertex, const ColorBGRAF& value) const;

        static SubPart Null() { return SubPart{nullptr, nullptr}; }
    };

    GenericVertex(const Graphics::VertexDeclaration *vertexDeclaration);
    ~GenericVertex();

    const Graphics::PCVertexDeclaration& VertexDeclaration() const { return _vertexDeclaration; }
    const MemoryView<u8>& Destination() const { return _destination; }
    size_t VertexOffset() const { return _vertexOffset; }

    void SetDestination(const MemoryView<u8>& data);
    void SetDestination(void *dst, size_t sizeInBytes);

    size_t VertexCountRemaining() const;
    void *CurrentVertex() const;

    void SeekVertex(size_t vertexIndex);
    bool NextVertex(); // advance to next offset, return if storage available

    void ClearSubParts() const;

    SubPart Position2f(size_t index) const;
    SubPart Position3f(size_t index) const;
    SubPart Position4f(size_t index) const;

    SubPart Color4b(size_t index) const;
    SubPart Color4u(size_t index) const;
    SubPart Color4f(size_t index) const;

    SubPart TexCoord1f(size_t index) const;
    SubPart TexCoord2f(size_t index) const;
    SubPart TexCoord3f(size_t index) const;
    SubPart TexCoord4f(size_t index) const;

    SubPart Normal3f(size_t index) const;
    SubPart Normal4f(size_t index) const;

    SubPart Tangent3f(size_t index) const;
    SubPart Tangent4f(size_t index) const;

    SubPart Binormal3f(size_t index) const;
    SubPart Binormal4f(size_t index) const;

private:
    Graphics::PCVertexDeclaration _vertexDeclaration;
    MemoryView<u8> _destination;
    size_t _vertexOffset;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
