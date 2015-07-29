#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Color/Color.h"
#include "Core/Maths/Geometry/ScalarVector_fwd.h"
#include "Core/Maths/Packing/PackedVectors.h"
#include "Core/Memory/MemoryView.h"

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

        operator const void * () const { return Key; }

        static SubPart Null() { return SubPart{nullptr, nullptr}; }

        void ReadValue(GenericVertex& vertex, float *pValue) const;
        void ReadValue(GenericVertex& vertex, float2 *pValue) const;
        void ReadValue(GenericVertex& vertex, float3 *pValue) const;
        void ReadValue(GenericVertex& vertex, float4 *pValue) const;

        void ReadValue(GenericVertex& vertex, ColorRGBA *pValue) const;
        void ReadValue(GenericVertex& vertex, ColorRGBA16 *pValue) const;
        void ReadValue(GenericVertex& vertex, ColorRGBAF *pValue) const;

        void ReadValue(GenericVertex& vertex, ColorBGRA *pValue) const;
        void ReadValue(GenericVertex& vertex, ColorBGRA16 *pValue) const;
        void ReadValue(GenericVertex& vertex, ColorBGRAF *pValue) const;

        void WriteValue(GenericVertex& vertex, float value) const;
        void WriteValue(GenericVertex& vertex, const float2& value) const;
        void WriteValue(GenericVertex& vertex, const float3& value) const;
        void WriteValue(GenericVertex& vertex, const float4& value) const;

        void WriteValue(GenericVertex& vertex, const ColorRGBA& value) const;
        void WriteValue(GenericVertex& vertex, const ColorRGBA16& value) const;
        void WriteValue(GenericVertex& vertex, const ColorRGBAF& value) const;

        void WriteValue(GenericVertex& vertex, const ColorBGRA& value) const;
        void WriteValue(GenericVertex& vertex, const ColorBGRA16& value) const;
        void WriteValue(GenericVertex& vertex, const ColorBGRAF& value) const;
    };

    GenericVertex(const Graphics::VertexDeclaration *vertexDeclaration);
    ~GenericVertex();

    const Graphics::VertexDeclaration *VertexDeclaration() const { return _vertexDeclaration; }
    const MemoryView<u8>& Destination() const { return _destination; }
    size_t VertexOffset() const { return _vertexOffset; }

    void SetDestination(const MemoryView<u8>& data);
    void SetDestination(void *dst, size_t sizeInBytes);

    size_t VertexCountWritten() const;
    size_t VertexCountRemaining() const;
    void *CurrentVertex() const;

    void SeekVertex(size_t vertexIndex);
    bool NextVertex(); // advance to next offset, return if storage available

    void ZeroMemory_CurrentVertex() const;
    void CopyVertex(size_t dst, size_t src) const;

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
    Graphics::SCVertexDeclaration _vertexDeclaration;
    MemoryView<u8> _destination;
    size_t _vertexOffset;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
