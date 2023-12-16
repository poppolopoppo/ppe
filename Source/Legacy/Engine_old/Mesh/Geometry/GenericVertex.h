#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Color/Color.h"
#include "Core/Maths/ScalarVector_fwd.h"
#include "Core/Maths/PackedVectors.h"
#include "Core/Memory/MemoryView.h"

namespace Core {
namespace Graphics {
class FAbstractVertexSubPart;
FWD_REFPTR(VertexDeclaration);
struct FVertexSubPartKey;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FGenericVertex {
public:
    struct FSubPart {
        const Graphics::FVertexSubPartKey *Key;
        const Graphics::FAbstractVertexSubPart *FValue;

        operator const void * () const { return Key; }

        static FSubPart Null() { return FSubPart{nullptr, nullptr}; }

        void ReadValue(FGenericVertex& vertex, float *pValue) const;
        void ReadValue(FGenericVertex& vertex, float2 *pValue) const;
        void ReadValue(FGenericVertex& vertex, float3 *pValue) const;
        void ReadValue(FGenericVertex& vertex, float4 *pValue) const;

        void ReadValue(FGenericVertex& vertex, ColorRGBA *pValue) const;
        void ReadValue(FGenericVertex& vertex, ColorRGBA16 *pValue) const;
        void ReadValue(FGenericVertex& vertex, ColorRGBAF *pValue) const;

        void ReadValue(FGenericVertex& vertex, ColorBGRA *pValue) const;
        void ReadValue(FGenericVertex& vertex, ColorBGRA16 *pValue) const;
        void ReadValue(FGenericVertex& vertex, ColorBGRAF *pValue) const;

        void WriteValue(FGenericVertex& vertex, float value) const;
        void WriteValue(FGenericVertex& vertex, const float2& value) const;
        void WriteValue(FGenericVertex& vertex, const float3& value) const;
        void WriteValue(FGenericVertex& vertex, const float4& value) const;

        void WriteValue(FGenericVertex& vertex, const ColorRGBA& value) const;
        void WriteValue(FGenericVertex& vertex, const ColorRGBA16& value) const;
        void WriteValue(FGenericVertex& vertex, const ColorRGBAF& value) const;

        void WriteValue(FGenericVertex& vertex, const ColorBGRA& value) const;
        void WriteValue(FGenericVertex& vertex, const ColorBGRA16& value) const;
        void WriteValue(FGenericVertex& vertex, const ColorBGRAF& value) const;
    };

    FGenericVertex(const Graphics::FVertexDeclaration *vertexDeclaration);
    ~FGenericVertex();

    const Graphics::FVertexDeclaration *FVertexDeclaration() const { return _vertexDeclaration; }
    const TMemoryView<u8>& Destination() const { return _destination; }
    size_t VertexOffset() const { return _vertexOffset; }

    void SetDestination(const TMemoryView<u8>& data);
    void SetDestination(void *dst, size_t sizeInBytes);

    size_t VertexCountWritten() const;
    size_t VertexCountRemaining() const;
    void *CurrentVertex() const;

    void SeekVertex(size_t vertexIndex);
    bool NextVertex(); // advance to next offset, return if storage available

    void ZeroMemory_CurrentVertex() const;
    void CopyVertex(size_t dst, size_t src) const;

    FSubPart Position2f(size_t index) const;
    FSubPart Position3f(size_t index) const;
    FSubPart Position4f(size_t index) const;

    FSubPart Color4b(size_t index) const;
    FSubPart Color4u(size_t index) const;
    FSubPart Color4f(size_t index) const;

    FSubPart TexCoord1f(size_t index) const;
    FSubPart TexCoord2f(size_t index) const;
    FSubPart TexCoord3f(size_t index) const;
    FSubPart TexCoord4f(size_t index) const;

    FSubPart Normal3f(size_t index) const;
    FSubPart Normal4f(size_t index) const;

    FSubPart Tangent3f(size_t index) const;
    FSubPart Tangent4f(size_t index) const;

    FSubPart Binormal3f(size_t index) const;
    FSubPart Binormal4f(size_t index) const;

private:
    Graphics::SCVertexDeclaration _vertexDeclaration;
    TMemoryView<u8> _destination;
    size_t _vertexOffset;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
