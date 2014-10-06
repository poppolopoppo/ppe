#pragma once

#include "Graphics.h"

#include "Core/ScalarVector.h"
#include "Core/PackedVectors.h"

namespace Core {
namespace Graphics {
class DeviceEncapsulator;
class VertexDeclaration;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void VertexTypes_Start();
void VertexTypes_Shutdown();
//----------------------------------------------------------------------------
void VertexTypes_OnDeviceCreate(DeviceEncapsulator *device);
void VertexTypes_OnDeviceDestroy(DeviceEncapsulator *device);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Vertex {
//----------------------------------------------------------------------------
struct Position0_UByte4 {
    ubyte4 Position0;

    static const VertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct Position0_UShort2 {
    ushort2 Position0;

    static const VertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct Position0_Half2 {
    half2 Position0;

    static const VertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct Position0_Half4 {
    half4 Position0;

    static const VertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct Position0_Float3 {
    float3 Position0;

    static const VertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct Position0_Float4 {
    float4 Position0;

    static const VertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct Position0_Float3__Color0_UByte4N {
    float3 Position0;
    ubyte4n Color0;

    static const VertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct Position0_Float4__TexCoord0_Float2 {
    float4 Position0;
    float2 TexCoord0;

    static const VertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct Position0_Float3__Color0_UByte4N__TexCoord0_Float2 {
    float3 Position0;
    ubyte4n Color0;
    float2 TexCoord0;

    static const VertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct Position0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N {
    float3 Position0;
    ubyte4n Color0;
    float2 TexCoord0;
    UX10Y10Z10W2N Normal0;

    static const VertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct Position0_Float3__TexCoord0_Float2__Normal0_Float3__Tangent0_Float3__Binormal0_Float3 {
    float3 Position0;
    float2 TexCoord0;
    float3 Normal0;
    float3 Tangent0;
    float3 Binormal0;

    static const VertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct Position0_Float3__TexCoord0_Half2__Normal0_UByte4N__Tangent0_UByte4N__Binormal0_UByte4N {
    float3 Position0;
    half2 TexCoord0;
    ubyte4n Normal0;
    ubyte4n Tangent0;
    ubyte4n Binormal0;

    static const VertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct Position0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N {
    float3 Position0;
    half2 TexCoord0;
    UX10Y10Z10W2N Normal0;
    UX10Y10Z10W2N Tangent0;

    static const VertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
} //!namespace Vertex
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
