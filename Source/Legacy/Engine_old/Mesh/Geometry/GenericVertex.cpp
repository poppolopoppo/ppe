﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "GenericVertex.h"

#include "Core.Graphics/Device/Geometry/VertexDeclaration.h"
#include "Core.Graphics/Device/Geometry/VertexSubPart.h"

#include "Core/Maths/PackedVectors.h"
#include "Core/Maths/PackingHelpers.h"

#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/ScalarVectorHelpers.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool AssignableFrom_(const Graphics::VertexSubPartFormat format, float ) {
    using namespace Graphics;
    switch (format)
    {
    case VertexSubPartFormat::Float:
    case VertexSubPartFormat::Half:
        return true;
    default:
        break;
    }
    return false;
}
//----------------------------------------------------------------------------
static void ReadValue_(
    void *const vertex, const Graphics::FVertexSubPartKey& key,
    const Graphics::FAbstractVertexSubPart *subPart,
    float *pValue) {
    Assert(pValue);
    using namespace Graphics;
    switch (key.Format())
    {
    case VertexSubPartFormat::Float:
        subPart->Get(vertex, pValue);
        break;
    case VertexSubPartFormat::Half:
        {
            FHalfFloat packed;
            subPart->Get(vertex, &packed);
            *pValue = packed.Unpack();
        }
        break;
    default:
        AssertNotImplemented();
        return;
    }
}
//----------------------------------------------------------------------------
static void WriteValue_(
    void *const vertex, const Graphics::FVertexSubPartKey& key,
    const Graphics::FAbstractVertexSubPart *subPart,
    float value) {
    using namespace Graphics;
    switch (key.Format())
    {
    case VertexSubPartFormat::Float:
        subPart->Set(vertex, value);
        break;
    case VertexSubPartFormat::Half:
        subPart->Set(vertex, FHalfFloat(value));
        break;
    default:
        AssertNotImplemented();
        return;
    }
}
//----------------------------------------------------------------------------
static bool AssignableFrom_(const Graphics::VertexSubPartFormat format, const float2& ) {
    using namespace Graphics;
    switch (format)
    {
    case VertexSubPartFormat::Float2:
    case VertexSubPartFormat::Byte2N:
    case VertexSubPartFormat::UByte2N:
    case VertexSubPartFormat::Short2N:
    case VertexSubPartFormat::UShort2N:
    case VertexSubPartFormat::Half2:
        return true;
    default:
        break;
    }
    return false;
}
//----------------------------------------------------------------------------
static void ReadValue_(
    void *const vertex, const Graphics::FVertexSubPartKey& key,
    const Graphics::FAbstractVertexSubPart *subPart,
    float2 *pValue) {
    Assert(pValue);
    using namespace Graphics;
    switch (key.Format())
    {
    case VertexSubPartFormat::Float2:
        subPart->Get(vertex, pValue);
        break;
    case VertexSubPartFormat::Byte2N:
        {
            byte2n packed;
            subPart->Get(vertex, &packed);
            *pValue = NormUnpack(packed);
        }
        break;
    case VertexSubPartFormat::UByte2N:
        {
            ubyte2n packed;
            subPart->Get(vertex, &packed);
            *pValue = NormUnpack(packed);
        }
        break;
    case VertexSubPartFormat::Short2N:
        {
            short2n packed;
            subPart->Get(vertex, &packed);
            *pValue = NormUnpack(packed);
        }
        break;
    case VertexSubPartFormat::UShort2N:
        {
            ushort2n packed;
            subPart->Get(vertex, &packed);
            *pValue = NormUnpack(packed);
        }
        break;
    case VertexSubPartFormat::Half2:
        {
            half2 packed;
            subPart->Get(vertex, &packed);
            *pValue = HalfUnpack(packed);
        }
        break;
    default:
        AssertNotImplemented();
        return;
    }
}
//----------------------------------------------------------------------------
static void WriteValue_(
    void *const vertex, const Graphics::FVertexSubPartKey& key,
    const Graphics::FAbstractVertexSubPart *subPart,
    const float2& value) {
    using namespace Graphics;
    switch (key.Format())
    {
    case VertexSubPartFormat::Float2:
        subPart->Set(vertex, value);
        break;
    case VertexSubPartFormat::Byte2N:
        subPart->Set(vertex, byte2n(value));
        break;
    case VertexSubPartFormat::UByte2N:
        subPart->Set(vertex, ubyte2n(value));
        break;
    case VertexSubPartFormat::Short2N:
        subPart->Set(vertex, short2n(value));
        break;
    case VertexSubPartFormat::UShort2N:
        subPart->Set(vertex, ushort2n(value));
        break;
    case VertexSubPartFormat::Half2:
        subPart->Set(vertex, half2(value));
        break;
    default:
        AssertNotImplemented();
        return;
    }
}
//----------------------------------------------------------------------------
static bool AssignableFrom_(const Graphics::VertexSubPartFormat format, const float3& ) {
    using namespace Graphics;
    switch (format)
    {
    case VertexSubPartFormat::Float3:
    case VertexSubPartFormat::Float4:
    case VertexSubPartFormat::Byte4N:
    case VertexSubPartFormat::UByte4N:
    case VertexSubPartFormat::Short4N:
    case VertexSubPartFormat::UShort4N:
    case VertexSubPartFormat::Half4:
    case VertexSubPartFormat::UX10Y10Z10W2N:
        return true;
    default:
        break;
    }
    return false;
}
//----------------------------------------------------------------------------
static void ReadValue_(void *const vertex, const Graphics::FVertexSubPartKey& key, const Graphics::FAbstractVertexSubPart *subPart, float3 *pValue) {
    Assert(pValue);
    using namespace Graphics;
    switch (key.Format())
    {
    case VertexSubPartFormat::Float3:
        subPart->Get(vertex, pValue);
        break;
    case VertexSubPartFormat::Float4:
        {
            float4 extended;
            subPart->Get(vertex, &extended);
            *pValue = extended.xyz();
        }
        break;
    case VertexSubPartFormat::Byte4N:
        {
            byte4n packed;
            subPart->Get(vertex, &packed);
            *pValue = NormUnpack(packed).xyz();
        }
        break;
    case VertexSubPartFormat::UByte4N:
        {
            ubyte4n packed;
            subPart->Get(vertex, &packed);
            *pValue = NormUnpack(packed).xyz();
        }
        break;
    case VertexSubPartFormat::Short4N:
        {
            short4n packed;
            subPart->Get(vertex, &packed);
            *pValue = NormUnpack(packed).xyz();
        }
        break;
    case VertexSubPartFormat::UShort4N:
        {
            ushort4n packed;
            subPart->Get(vertex, &packed);
            *pValue = NormUnpack(packed).xyz();
        }
        break;
    case VertexSubPartFormat::Half4:
        {
            half4 packed;
            subPart->Get(vertex, &packed);
            *pValue = HalfUnpack(packed).xyz();
        }
        break;
    case VertexSubPartFormat::UX10Y10Z10W2N:
        {
            UX10Y10Z10W2N packed;
            subPart->Get(vertex, &packed);
            packed.Unpack_FloatM11(*pValue);
        }
        break;
    default:
        AssertNotImplemented();
        return;
    }
}
//----------------------------------------------------------------------------
static void WriteValue_(void *const vertex, const Graphics::FVertexSubPartKey& key, const Graphics::FAbstractVertexSubPart *subPart, const float3& value) {
    using namespace Graphics;
    switch (key.Format())
    {
    case VertexSubPartFormat::Float3:
        subPart->Set(vertex, value);
        break;
    case VertexSubPartFormat::Float4:
        subPart->Set(vertex, value.ZeroExtend());
        break;
    case VertexSubPartFormat::Byte4N:
        subPart->Set(vertex, byte4n(value.ZeroExtend()));
        break;
    case VertexSubPartFormat::UByte4N:
        subPart->Set(vertex, ubyte4n(value.ZeroExtend()));
        break;
    case VertexSubPartFormat::Short4N:
        subPart->Set(vertex, short4n(value.ZeroExtend()));
        break;
    case VertexSubPartFormat::UShort4N:
        subPart->Set(vertex, ushort4n(value.ZeroExtend()));
        break;
    case VertexSubPartFormat::Half4:
        subPart->Set(vertex, HalfPack(value.ZeroExtend()));
        break;
    case VertexSubPartFormat::UX10Y10Z10W2N:
        subPart->Set(vertex, FloatM11_to_UX10Y10Z10W2N(value, 0));
        break;
    default:
        AssertNotImplemented();
        return;
    }
}
//----------------------------------------------------------------------------
static bool AssignableFrom_(const Graphics::VertexSubPartFormat format, const float4& ) {
    using namespace Graphics;
    switch (format)
    {
    case VertexSubPartFormat::Float4:
    case VertexSubPartFormat::Byte4N:
    case VertexSubPartFormat::UByte4N:
    case VertexSubPartFormat::Short4N:
    case VertexSubPartFormat::UShort4N:
    case VertexSubPartFormat::UX10Y10Z10W2N:
    case VertexSubPartFormat::Half4:
        return true;
    default:
        break;
    }
    return false;
}
//----------------------------------------------------------------------------
static void ReadValue_(
    void *const vertex,
    const Graphics::FVertexSubPartKey& key,
    const Graphics::FAbstractVertexSubPart *subPart,
    float4 *pValue) {
    Assert(pValue);
    using namespace Graphics;
    switch (key.Format())
    {
    case VertexSubPartFormat::Float4:
        subPart->Get(vertex, pValue);
        break;
    case VertexSubPartFormat::Byte4N:
        {
            byte4n packed;
            subPart->Get(vertex, &packed);
            *pValue = NormUnpack(packed);
        }
        break;
    case VertexSubPartFormat::UByte4N:
        {
            ubyte4n packed;
            subPart->Get(vertex, &packed);
            *pValue = NormUnpack(packed);
        }
        break;
    case VertexSubPartFormat::Short4N:
        {
            short4n packed;
            subPart->Get(vertex, &packed);
            *pValue = NormUnpack(packed);
        }
        break;
    case VertexSubPartFormat::UShort4N:
        {
            ushort4n packed;
            subPart->Get(vertex, &packed);
            *pValue = NormUnpack(packed);
        }
        break;
    case VertexSubPartFormat::UX10Y10Z10W2N:
        {
            UX10Y10Z10W2N packed;
            subPart->Get(vertex, &packed);

            float3 xyz;
            u8 w;
            packed.Unpack_FloatM11(xyz, w);
            *pValue = float4(xyz, float(w));
        }
        break;
    case VertexSubPartFormat::Half4:
        {
            half4 packed;
            subPart->Get(vertex, &packed);
            *pValue = HalfUnpack(packed);
        }
        break;
    default:
        AssertNotImplemented();
        return;
    }
}
//----------------------------------------------------------------------------
static void WriteValue_(
    void *const vertex,
    const Graphics::FVertexSubPartKey& key,
    const Graphics::FAbstractVertexSubPart *subPart,
    const float4& value) {
    using namespace Graphics;
    switch (key.Format())
    {
    case VertexSubPartFormat::Float4:
        subPart->Set(vertex, value);
        break;
    case VertexSubPartFormat::Byte4N:
        subPart->Set(vertex, byte4n(value));
        break;
    case VertexSubPartFormat::UByte4N:
        subPart->Set(vertex, ubyte4n(value));
        break;
    case VertexSubPartFormat::Short4N:
        subPart->Set(vertex, short4n(value));
        break;
    case VertexSubPartFormat::UShort4N:
        subPart->Set(vertex, ushort4n(value));
        break;
    case VertexSubPartFormat::UX10Y10Z10W2N:
        subPart->Set(vertex, FloatM11_to_UX10Y10Z10W2N(value.xyz(), u8(value.w())));
        break;
    case VertexSubPartFormat::Half4:
        subPart->Set(vertex, half4(value));
        break;
    default:
        AssertNotImplemented();
        return;
    }
}
//----------------------------------------------------------------------------
static bool AssignableFrom_(const Graphics::VertexSubPartFormat format, const ColorBGRAF& ) {
    return AssignableFrom_(format, float4());
}
//----------------------------------------------------------------------------
static void ReadValue_(void *const vertex, const Graphics::FVertexSubPartKey& key, const Graphics::FAbstractVertexSubPart *subPart, ColorBGRAF *pColor) {
    Assert(pColor);
    float4 data;
    ReadValue_(vertex, key, subPart, &data);
    pColor->Data() = data;
}
//----------------------------------------------------------------------------
static void WriteValue_(void *const vertex, const Graphics::FVertexSubPartKey& key, const Graphics::FAbstractVertexSubPart *subPart, const ColorBGRAF& color) {
    WriteValue_(vertex, key, subPart, color.Data());
}
//----------------------------------------------------------------------------
static bool AssignableFrom_(const Graphics::VertexSubPartFormat format, const ColorBGRA& ) {
    return AssignableFrom_(format, float4());
}
//----------------------------------------------------------------------------
static void ReadValue_(void *const vertex, const Graphics::FVertexSubPartKey& key, const Graphics::FAbstractVertexSubPart *subPart, ColorBGRA *pColor) {
    Assert(pColor);
    float4 data;
    ReadValue_(vertex, key, subPart, &data);
    *pColor = ColorBGRA(ColorBGRAF(data));
}
//----------------------------------------------------------------------------
static void WriteValue_(void *const vertex, const Graphics::FVertexSubPartKey& key, const Graphics::FAbstractVertexSubPart *subPart, const ColorBGRA& color) {
    WriteValue_(vertex, key, subPart, ColorBGRAF(color).Data());
}
//----------------------------------------------------------------------------
static bool AssignableFrom_(const Graphics::VertexSubPartFormat format, const ColorBGRA16& ) {
    return AssignableFrom_(format, float4());
}
//----------------------------------------------------------------------------
static void ReadValue_(void *const vertex, const Graphics::FVertexSubPartKey& key, const Graphics::FAbstractVertexSubPart *subPart, ColorBGRA16 *pColor) {
    float4 data;
    ReadValue_(vertex, key, subPart, &data);
    *pColor = ColorBGRA16(ColorBGRAF(data));
}
//----------------------------------------------------------------------------
static void WriteValue_(void *const vertex, const Graphics::FVertexSubPartKey& key, const Graphics::FAbstractVertexSubPart *subPart, const ColorBGRA16& color) {
    WriteValue_(vertex, key, subPart, ColorBGRAF(color).Data());
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FGenericVertex::FGenericVertex(const Graphics::FVertexDeclaration *vertexDeclaration)
:   _vertexDeclaration(vertexDeclaration)
,   _vertexOffset(0) {
    Assert(vertexDeclaration);
}
//----------------------------------------------------------------------------
FGenericVertex::~FGenericVertex() {}
//----------------------------------------------------------------------------
void FGenericVertex::SetDestination(const TMemoryView<u8>& data) {
    Assert(data.SizeInBytes() >= _vertexDeclaration->SizeInBytes());
    Assert(0 == (data.SizeInBytes() % _vertexDeclaration->SizeInBytes()) );

    _destination = data;
    _vertexOffset = 0;
}
//----------------------------------------------------------------------------
void FGenericVertex::SetDestination(void *dst, size_t sizeInBytes) {
    SetDestination(TMemoryView<u8>(reinterpret_cast<u8 *>(dst), sizeInBytes));
}
//----------------------------------------------------------------------------
size_t FGenericVertex::VertexCountWritten() const { 
    const size_t vertexSizeInBytes = _vertexDeclaration->SizeInBytes();
    Assert(0 == ((_destination.SizeInBytes() - _vertexOffset) % vertexSizeInBytes) );

    return _vertexOffset / vertexSizeInBytes;
}
//----------------------------------------------------------------------------
size_t FGenericVertex::VertexCountRemaining() const {
    const size_t vertexSizeInBytes = _vertexDeclaration->SizeInBytes();
    Assert(0 == ((_destination.SizeInBytes() - _vertexOffset) % vertexSizeInBytes) );

    return (_destination.SizeInBytes() - _vertexOffset) / vertexSizeInBytes;
}
//----------------------------------------------------------------------------
void *FGenericVertex::CurrentVertex() const {
    Assert(_destination.size());
    Assert(_vertexOffset < _destination.SizeInBytes());

    return _destination.Pointer() + _vertexOffset;
}
//----------------------------------------------------------------------------
void FGenericVertex::SeekVertex(size_t vertexIndex) {
    Assert(_destination.size());

    _vertexOffset = _vertexDeclaration->SizeInBytes() * vertexIndex;
    Assert(_vertexOffset <= _destination.SizeInBytes());
}
//----------------------------------------------------------------------------
bool FGenericVertex::NextVertex() {
    const size_t vertexSizeInBytes = _vertexDeclaration->SizeInBytes();
    Assert((_destination.SizeInBytes() - _vertexOffset) >= vertexSizeInBytes);

    _vertexOffset += vertexSizeInBytes;

    return _vertexOffset < _destination.SizeInBytes();
}
//----------------------------------------------------------------------------
void FGenericVertex::ZeroMemory_CurrentVertex() const {
    void *const currentVertex = CurrentVertex();

    for (const TPair<Graphics::FVertexSubPartKey, Graphics::VertexSubPartPOD>& it : _vertexDeclaration->SubParts()) {
        const Graphics::FAbstractVertexSubPart *subPart = reinterpret_cast<const Graphics::FAbstractVertexSubPart *>(&it.second);
        subPart->Clear(currentVertex);
    }
}
//----------------------------------------------------------------------------
void FGenericVertex::CopyVertex(size_t dst, size_t src) const {
    if (dst == src)
        return;

    Assert(VertexCountWritten() > dst);
    Assert(VertexCountWritten() > src);

    const size_t vertexSizeInBytes = _vertexDeclaration->SizeInBytes();
    memcpy( _destination.Pointer() + dst * vertexSizeInBytes, 
            _destination.Pointer() + src * vertexSizeInBytes,
            vertexSizeInBytes );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FGenericVertex::FSubPart::ReadValue(FGenericVertex& vertex, float *pValue) const {
    Assert(Key);
    Assert(pValue);
    ReadValue_(vertex.CurrentVertex(), *Key, FValue, pValue);
}
//----------------------------------------------------------------------------
void FGenericVertex::FSubPart::ReadValue(FGenericVertex& vertex, float2 *pValue) const {
    Assert(Key);
    Assert(pValue);
    ReadValue_(vertex.CurrentVertex(), *Key, FValue, pValue);
}
//----------------------------------------------------------------------------
void FGenericVertex::FSubPart::ReadValue(FGenericVertex& vertex, float3 *pValue) const {
    Assert(Key);
    Assert(pValue);
    ReadValue_(vertex.CurrentVertex(), *Key, FValue, pValue);
}
//----------------------------------------------------------------------------
void FGenericVertex::FSubPart::ReadValue(FGenericVertex& vertex, float4 *pValue) const {
    Assert(Key);
    Assert(pValue);
    ReadValue_(vertex.CurrentVertex(), *Key, FValue, pValue);
}
//----------------------------------------------------------------------------
void FGenericVertex::FSubPart::ReadValue(FGenericVertex& vertex, ColorRGBA *pValue) const {
    Assert(Key);
    Assert(pValue);
    ColorBGRA tmp;
    ReadValue_(vertex.CurrentVertex(), *Key, FValue, &tmp);
    *pValue = tmp;
}
//----------------------------------------------------------------------------
void FGenericVertex::FSubPart::ReadValue(FGenericVertex& vertex, ColorRGBA16 *pValue) const {
    Assert(Key);
    Assert(pValue);
    ColorBGRA16 tmp;
    ReadValue_(vertex.CurrentVertex(), *Key, FValue, &tmp);
    *pValue = tmp;
}
//----------------------------------------------------------------------------
void FGenericVertex::FSubPart::ReadValue(FGenericVertex& vertex, ColorRGBAF *pValue) const {
    Assert(Key);
    Assert(pValue);
    ColorBGRAF tmp;
    ReadValue_(vertex.CurrentVertex(), *Key, FValue, &tmp);
    *pValue = tmp;
}
//----------------------------------------------------------------------------
void FGenericVertex::FSubPart::ReadValue(FGenericVertex& vertex, ColorBGRA *pValue) const {
    Assert(Key);
    ReadValue_(vertex.CurrentVertex(), *Key, FValue, pValue);
}
//----------------------------------------------------------------------------
void FGenericVertex::FSubPart::ReadValue(FGenericVertex& vertex, ColorBGRA16 *pValue) const {
    Assert(Key);
    Assert(pValue);
    ReadValue_(vertex.CurrentVertex(), *Key, FValue, pValue);
}
//----------------------------------------------------------------------------
void FGenericVertex::FSubPart::ReadValue(FGenericVertex& vertex, ColorBGRAF *pValue) const {
    Assert(Key);
    Assert(pValue);
    ReadValue_(vertex.CurrentVertex(), *Key, FValue, pValue);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FGenericVertex::FSubPart::WriteValue(FGenericVertex& vertex, float value) const {
    Assert(Key);
    WriteValue_(vertex.CurrentVertex(), *Key, FValue, value);
}
//----------------------------------------------------------------------------
void FGenericVertex::FSubPart::WriteValue(FGenericVertex& vertex, const float2& value) const {
    Assert(Key);
    WriteValue_(vertex.CurrentVertex(), *Key, FValue, value);
}
//----------------------------------------------------------------------------
void FGenericVertex::FSubPart::WriteValue(FGenericVertex& vertex, const float3& value) const {
    Assert(Key);
    WriteValue_(vertex.CurrentVertex(), *Key, FValue, value);
}
//----------------------------------------------------------------------------
void FGenericVertex::FSubPart::WriteValue(FGenericVertex& vertex, const float4& value) const {
    Assert(Key);
    WriteValue_(vertex.CurrentVertex(), *Key, FValue, value);
}
//----------------------------------------------------------------------------
void FGenericVertex::FSubPart::WriteValue(FGenericVertex& vertex, const ColorRGBA& value) const {
    Assert(Key);
    WriteValue_(vertex.CurrentVertex(), *Key, FValue, ColorBGRA(value));
}
//----------------------------------------------------------------------------
void FGenericVertex::FSubPart::WriteValue(FGenericVertex& vertex, const ColorRGBA16& value) const {
    Assert(Key);
    WriteValue_(vertex.CurrentVertex(), *Key, FValue, ColorBGRA16(value));
}
//----------------------------------------------------------------------------
void FGenericVertex::FSubPart::WriteValue(FGenericVertex& vertex, const ColorRGBAF& value) const {
    Assert(Key);
    WriteValue_(vertex.CurrentVertex(), *Key, FValue, ColorBGRAF(value));
}
//----------------------------------------------------------------------------
void FGenericVertex::FSubPart::WriteValue(FGenericVertex& vertex, const ColorBGRA& value) const {
    Assert(Key);
    WriteValue_(vertex.CurrentVertex(), *Key, FValue, value);
}
//----------------------------------------------------------------------------
void FGenericVertex::FSubPart::WriteValue(FGenericVertex& vertex, const ColorBGRA16& value) const {
    Assert(Key);
    WriteValue_(vertex.CurrentVertex(), *Key, FValue, value);
}
//----------------------------------------------------------------------------
void FGenericVertex::FSubPart::WriteValue(FGenericVertex& vertex, const ColorBGRAF& value) const {
    Assert(Key);
    WriteValue_(vertex.CurrentVertex(), *Key, FValue, value);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DEF_GENERICVERTEX_SETTER(_NAME, _SEMANTIC, _TYPE) \
    FGenericVertex::FSubPart FGenericVertex::_NAME(size_t index) const { \
        const TPair<const Graphics::FVertexSubPartKey *, const Graphics::FAbstractVertexSubPart *> subPart = \
            _vertexDeclaration->SubPartBySemanticIFP(Graphics::VertexSubPartSemantic::_SEMANTIC, index); \
        \
        return (subPart.first && subPart.second && \
                AssignableFrom_(subPart.first->Format(), _TYPE()) ) \
            ? FGenericVertex::FSubPart{subPart.first, subPart.second} \
            : FGenericVertex::FSubPart{nullptr, nullptr}; \
    }
//----------------------------------------------------------------------------
DEF_GENERICVERTEX_SETTER(Position2f, Position, float2)
DEF_GENERICVERTEX_SETTER(Position3f, Position, float3)
DEF_GENERICVERTEX_SETTER(Position4f, Position, float4)
//----------------------------------------------------------------------------
DEF_GENERICVERTEX_SETTER(Color4b, Color, ColorBGRA)
DEF_GENERICVERTEX_SETTER(Color4u, Color, ColorBGRA16)
DEF_GENERICVERTEX_SETTER(Color4f, Color, ColorBGRAF)
//----------------------------------------------------------------------------
DEF_GENERICVERTEX_SETTER(TexCoord1f, TexCoord, float);
DEF_GENERICVERTEX_SETTER(TexCoord2f, TexCoord, float2);
DEF_GENERICVERTEX_SETTER(TexCoord3f, TexCoord, float3);
DEF_GENERICVERTEX_SETTER(TexCoord4f, TexCoord, float4);
//----------------------------------------------------------------------------
DEF_GENERICVERTEX_SETTER(Normal3f, Normal, float3);
DEF_GENERICVERTEX_SETTER(Normal4f, Normal, float4);
//----------------------------------------------------------------------------
DEF_GENERICVERTEX_SETTER(Tangent3f, Tangent, float3);
DEF_GENERICVERTEX_SETTER(Tangent4f, Tangent, float4);
//----------------------------------------------------------------------------
DEF_GENERICVERTEX_SETTER(Binormal3f, Binormal, float3);
DEF_GENERICVERTEX_SETTER(Binormal4f, Binormal, float4);
//----------------------------------------------------------------------------
#undef DEF_GENERICVERTEX_SETTER
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
