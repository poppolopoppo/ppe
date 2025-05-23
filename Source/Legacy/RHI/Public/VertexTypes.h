#pragma once

#include "Graphics.h"

#include "IO/String_fwd.h"
#include "Maths/ScalarVector.h"
#include "Maths/PackedVectors.h"

namespace PPE {
namespace Graphics {
class FDeviceEncapsulator;
FWD_REFPTR(VertexDeclaration);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void VertexTypes_Start();
void VertexTypes_Shutdown();
//----------------------------------------------------------------------------
void VertexTypes_OnDeviceCreate(FDeviceEncapsulator *device);
void VertexTypes_OnDeviceDestroy(FDeviceEncapsulator *device);
//----------------------------------------------------------------------------
void RegisterVertexType(FWString&& name, const FVertexDeclaration* vdecl);
void UnregisterVertexType(const FVertexDeclaration* vdecl);
const FVertexDeclaration* VertexTypeByName(const FWStringView& name);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Vertex {
//----------------------------------------------------------------------------
struct FPosition0_UByte4 {
    ubyte4 Position0;

    static const FVertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct FPosition0_UShort2 {
    ushort2 Position0;

    static const FVertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct FPosition0_Half2 {
    half2 Position0;

    static const FVertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct FPosition0_Half4 {
    half4 Position0;

    static const FVertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct FPosition0_Float3 {
    float3 Position0;

    static const FVertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct FPosition0_Float4 {
    float4 Position0;

    static const FVertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct FPosition0_Float3__Color0_UByte4N {
    float3 Position0;
    ubyte4n Color0;

    static const FVertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct FPosition0_Float3__Color0_UByte4N__Normal0_UX10Y10Z10W2N {
    float3 Position0;
    ubyte4n Color0;
    UX10Y10Z10W2N Normal0;

    static const FVertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct FPosition0_Float3__Normal0_UX10Y10Z10W2N {
    float3 Position0;
    UX10Y10Z10W2N Normal0;

    static const FVertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct FPosition0_Float4__TexCoord0_Float2 {
    float4 Position0;
    float2 TexCoord0;

    static const FVertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct FPosition0_Float4__TexCoord0_Half2 {
    float4 Position0;
    half2 TexCoord0;

    static const FVertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct FPosition0_Float3__TexCoord0_Half2 {
    float3 Position0;
    half2 TexCoord0;

    static const FVertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct FPosition0_Float3__Color0_UByte4N__TexCoord0_Float2 {
    float3 Position0;
    ubyte4n Color0;
    float2 TexCoord0;

    static const FVertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2 {
    float3 Position0;
    ubyte4n Color0;
    half2 TexCoord0;

    static const FVertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct FPosition0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N {
    float3 Position0;
    ubyte4n Color0;
    float2 TexCoord0;
    UX10Y10Z10W2N Normal0;

    static const FVertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct FPosition0_Float3__TexCoord0_Half2__Normal0_UByte4N {
    float3 Position0;
    half2 TexCoord0;
    ubyte4n Normal0;

    static const FVertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct FPosition0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N {
    float3 Position0;
    half2 TexCoord0;
    UX10Y10Z10W2N Normal0;

    static const FVertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct FPosition0_Float3__TexCoord0_Float2__Normal0_Float3 {
    float3 Position0;
    float2 TexCoord0;
    float3 Normal0;

    static const FVertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct FPosition0_Float3__TexCoord0_Float2__Normal0_Float3__Tangent0_Float3__Binormal0_Float3 {
    float3 Position0;
    float2 TexCoord0;
    float3 Normal0;
    float3 Tangent0;
    float3 Binormal0;

    static const FVertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct FPosition0_Float3__TexCoord0_Half2__Normal0_UByte4N__Tangent0_UByte4N__Binormal0_UByte4N {
    float3 Position0;
    half2 TexCoord0;
    ubyte4n Normal0;
    ubyte4n Tangent0;
    ubyte4n Binormal0;

    static const FVertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct FPosition0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N {
    float3 Position0;
    half2 TexCoord0;
    UX10Y10Z10W2N Normal0;
    UX10Y10Z10W2N Tangent0;

    static const FVertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N {
    float3 Position0;
    ubyte4n Color0;
    half2 TexCoord0;
    UX10Y10Z10W2N Normal0;

    static const FVertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct FPosition0_Float3__Color0_UByte4N__Color1_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N {
    float3 Position0;
    ubyte4n Color0;
    ubyte4n Color1;
    half2 TexCoord0;
    UX10Y10Z10W2N Normal0;

    static const FVertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
struct FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N {
    float3 Position0;
    ubyte4n Color0;
    half2 TexCoord0;
    UX10Y10Z10W2N Normal0;
    UX10Y10Z10W2N Tangent0;

    static const FVertexDeclaration *Declaration;
};
//----------------------------------------------------------------------------
} //!namespace Vertex
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FVertexSemantic;
class FVertexDeclarator {
public:
    FVertexDeclarator(FVertexDeclaration* vdecl);
    ~FVertexDeclarator();

    FVertexDeclarator(const FVertexDeclarator& ) = delete;
    FVertexDeclarator& operator =(const FVertexDeclarator& ) = delete;

    FVertexDeclaration* VDecl() const { return _vdecl.get(); }

    template <typename _Class, typename T>
    void AddSubPart(const FVertexSemantic& semantic, size_t index, T _Class:: *member) const {
        Assert(!_vdecl->Frozen());
        _vdecl->AddTypedSubPart(semantic, index, member);
    }

private:
    PVertexDeclaration _vdecl;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
