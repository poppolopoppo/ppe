#include "stdafx.h"

#include "VertexTypes.h"

#include "DeviceEncapsulator.h"
#include "VertexDeclaration.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef std::aligned_storage< sizeof(VertexDeclaration), std::alignment_of<VertexDeclaration>::value >::type
        VertexDeclarationPOD;
//----------------------------------------------------------------------------
#define DEF_VERTEXDECL_BUILTINTYPE(_Name) \
    namespace { static VertexDeclarationPOD CONCAT(gVertexDeclarationPOD_, _Name); } \
    namespace Vertex { const VertexDeclaration *_Name::Declaration = nullptr; }

DEF_VERTEXDECL_BUILTINTYPE(Position0_UByte4)
DEF_VERTEXDECL_BUILTINTYPE(Position0_UShort2)
DEF_VERTEXDECL_BUILTINTYPE(Position0_Half2)
DEF_VERTEXDECL_BUILTINTYPE(Position0_Half4)
DEF_VERTEXDECL_BUILTINTYPE(Position0_Float3)
DEF_VERTEXDECL_BUILTINTYPE(Position0_Float4)
DEF_VERTEXDECL_BUILTINTYPE(Position0_Float3__Color0_UByte4N)
DEF_VERTEXDECL_BUILTINTYPE(Position0_Float4__TexCoord0_Float2)
DEF_VERTEXDECL_BUILTINTYPE(Position0_Float3__Color0_UByte4N__TexCoord0_Float2);
DEF_VERTEXDECL_BUILTINTYPE(Position0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N);
DEF_VERTEXDECL_BUILTINTYPE(Position0_Float3__TexCoord0_Float2__Normal0_Float3__Tangent0_Float3__Binormal0_Float3)
DEF_VERTEXDECL_BUILTINTYPE(Position0_Float3__TexCoord0_Half2__Normal0_UByte4N__Tangent0_UByte4N__Binormal0_UByte4N)
DEF_VERTEXDECL_BUILTINTYPE(Position0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N)

#undef DEF_VERTEXDECL_BUILTINTYPE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void VertexTypes_Start() {
#define CREATE_VERTEXDECL_BUILTINTYPE(_NAME) \
    Assert(nullptr == Vertex::_NAME::Declaration); \
    void *const pod = reinterpret_cast<void *>(&CONCAT(gVertexDeclarationPOD_, _NAME)); \
    Vertex::_NAME::Declaration = ::new (pod) VertexDeclaration(); \
    AddRef(Vertex::_NAME::Declaration); \
    VertexDeclarator< Vertex::_NAME > vdecl(const_cast<VertexDeclaration *>(Vertex::_NAME::Declaration)); \
    vdecl.SetResourceName(STRINGIZE(_NAME))

    {
        CREATE_VERTEXDECL_BUILTINTYPE(Position0_UByte4);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Position>(&Vertex::Position0_UByte4::Position0, 0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(Position0_UShort2);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Position>(&Vertex::Position0_UShort2::Position0, 0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(Position0_Half2);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Position>(&Vertex::Position0_Half2::Position0, 0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(Position0_Half4);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Position>(&Vertex::Position0_Half4::Position0, 0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(Position0_Float3);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Position>(&Vertex::Position0_Float3::Position0, 0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(Position0_Float4);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Position>(&Vertex::Position0_Float4::Position0, 0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(Position0_Float3__Color0_UByte4N);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Position>(&Vertex::Position0_Float3__Color0_UByte4N::Position0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Color>(&Vertex::Position0_Float3__Color0_UByte4N::Color0, 0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(Position0_Float4__TexCoord0_Float2);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Position>(&Vertex::Position0_Float4__TexCoord0_Float2::Position0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::TexCoord>(&Vertex::Position0_Float4__TexCoord0_Float2::TexCoord0, 0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(Position0_Float3__Color0_UByte4N__TexCoord0_Float2);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Position>(&Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Float2::Position0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Color>(&Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Float2::Color0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::TexCoord>(&Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Float2::TexCoord0, 0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(Position0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Position>(&Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N::Position0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Color>(&Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N::Color0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::TexCoord>(&Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N::TexCoord0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Normal>(&Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N::Normal0, 0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(Position0_Float3__TexCoord0_Float2__Normal0_Float3__Tangent0_Float3__Binormal0_Float3);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Position>(&Vertex::Position0_Float3__TexCoord0_Float2__Normal0_Float3__Tangent0_Float3__Binormal0_Float3::Position0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::TexCoord>(&Vertex::Position0_Float3__TexCoord0_Float2__Normal0_Float3__Tangent0_Float3__Binormal0_Float3::TexCoord0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Normal>(&Vertex::Position0_Float3__TexCoord0_Float2__Normal0_Float3__Tangent0_Float3__Binormal0_Float3::Normal0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Tangent>(&Vertex::Position0_Float3__TexCoord0_Float2__Normal0_Float3__Tangent0_Float3__Binormal0_Float3::Tangent0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Binormal>(&Vertex::Position0_Float3__TexCoord0_Float2__Normal0_Float3__Tangent0_Float3__Binormal0_Float3::Binormal0, 0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(Position0_Float3__TexCoord0_Half2__Normal0_UByte4N__Tangent0_UByte4N__Binormal0_UByte4N);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Position>(&Vertex::Position0_Float3__TexCoord0_Half2__Normal0_UByte4N__Tangent0_UByte4N__Binormal0_UByte4N::Position0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::TexCoord>(&Vertex::Position0_Float3__TexCoord0_Half2__Normal0_UByte4N__Tangent0_UByte4N__Binormal0_UByte4N::TexCoord0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Normal>(&Vertex::Position0_Float3__TexCoord0_Half2__Normal0_UByte4N__Tangent0_UByte4N__Binormal0_UByte4N::Normal0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Tangent>(&Vertex::Position0_Float3__TexCoord0_Half2__Normal0_UByte4N__Tangent0_UByte4N__Binormal0_UByte4N::Tangent0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Binormal>(&Vertex::Position0_Float3__TexCoord0_Half2__Normal0_UByte4N__Tangent0_UByte4N__Binormal0_UByte4N::Binormal0, 0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(Position0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Position>(&Vertex::Position0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N::Position0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::TexCoord>(&Vertex::Position0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N::TexCoord0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Normal>(&Vertex::Position0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N::Normal0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Tangent>(&Vertex::Position0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N::Tangent0, 0);
    }

#undef CREATE_VERTEXDECL_BUILTINTYPE
}
//----------------------------------------------------------------------------
void VertexTypes_Shutdown() {
#define DESTROY_VERTEXDECL_BUILTINTYPE(_NAME) \
    Assert(Vertex::_NAME::Declaration); \
    Assert((void *)&CONCAT(gVertexDeclarationPOD_, _NAME) == Vertex::_NAME::Declaration); \
    RemoveRef_AssertReachZero_NoDelete(Vertex::_NAME::Declaration); \
    Vertex::_NAME::Declaration = nullptr

    DESTROY_VERTEXDECL_BUILTINTYPE(Position0_UByte4);
    DESTROY_VERTEXDECL_BUILTINTYPE(Position0_UShort2);
    DESTROY_VERTEXDECL_BUILTINTYPE(Position0_Half2);
    DESTROY_VERTEXDECL_BUILTINTYPE(Position0_Half4);
    DESTROY_VERTEXDECL_BUILTINTYPE(Position0_Float3);
    DESTROY_VERTEXDECL_BUILTINTYPE(Position0_Float4);
    DESTROY_VERTEXDECL_BUILTINTYPE(Position0_Float3__Color0_UByte4N);
    DESTROY_VERTEXDECL_BUILTINTYPE(Position0_Float4__TexCoord0_Float2);
    DESTROY_VERTEXDECL_BUILTINTYPE(Position0_Float3__Color0_UByte4N__TexCoord0_Float2);
    DESTROY_VERTEXDECL_BUILTINTYPE(Position0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N);
    DESTROY_VERTEXDECL_BUILTINTYPE(Position0_Float3__TexCoord0_Float2__Normal0_Float3__Tangent0_Float3__Binormal0_Float3);
    DESTROY_VERTEXDECL_BUILTINTYPE(Position0_Float3__TexCoord0_Half2__Normal0_UByte4N__Tangent0_UByte4N__Binormal0_UByte4N);
    DESTROY_VERTEXDECL_BUILTINTYPE(Position0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N);

#undef DESTROY_VERTEXDECL_BUILTINTYPE
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void VertexTypes_OnDeviceCreate(DeviceEncapsulator *device) {
#define CREATEWDEVICE_VERTEXDECL_BUILTINTYPE(_NAME) \
    const_cast<VertexDeclaration *>(Vertex::_NAME::Declaration)->Create(device->Device())

    CREATEWDEVICE_VERTEXDECL_BUILTINTYPE(Position0_UByte4);
    CREATEWDEVICE_VERTEXDECL_BUILTINTYPE(Position0_UShort2);
    CREATEWDEVICE_VERTEXDECL_BUILTINTYPE(Position0_Half2);
    CREATEWDEVICE_VERTEXDECL_BUILTINTYPE(Position0_Half4);
    CREATEWDEVICE_VERTEXDECL_BUILTINTYPE(Position0_Float3);
    CREATEWDEVICE_VERTEXDECL_BUILTINTYPE(Position0_Float4);
    CREATEWDEVICE_VERTEXDECL_BUILTINTYPE(Position0_Float3__Color0_UByte4N);
    CREATEWDEVICE_VERTEXDECL_BUILTINTYPE(Position0_Float4__TexCoord0_Float2);
    CREATEWDEVICE_VERTEXDECL_BUILTINTYPE(Position0_Float3__Color0_UByte4N__TexCoord0_Float2);
    CREATEWDEVICE_VERTEXDECL_BUILTINTYPE(Position0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N);
    CREATEWDEVICE_VERTEXDECL_BUILTINTYPE(Position0_Float3__TexCoord0_Float2__Normal0_Float3__Tangent0_Float3__Binormal0_Float3);
    CREATEWDEVICE_VERTEXDECL_BUILTINTYPE(Position0_Float3__TexCoord0_Half2__Normal0_UByte4N__Tangent0_UByte4N__Binormal0_UByte4N);
    CREATEWDEVICE_VERTEXDECL_BUILTINTYPE(Position0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N);

#undef CREATEWDEVICE_VERTEXDECL_BUILTINTYPE
}
//----------------------------------------------------------------------------
void VertexTypes_OnDeviceDestroy(DeviceEncapsulator *device) {
#define DESTROYWDEVICE_VERTEXDECL_BUILTINTYPE(_NAME) \
    const_cast<VertexDeclaration *>(Vertex::_NAME::Declaration)->Destroy(device->Device())

    DESTROYWDEVICE_VERTEXDECL_BUILTINTYPE(Position0_UByte4);
    DESTROYWDEVICE_VERTEXDECL_BUILTINTYPE(Position0_UShort2);
    DESTROYWDEVICE_VERTEXDECL_BUILTINTYPE(Position0_Half2);
    DESTROYWDEVICE_VERTEXDECL_BUILTINTYPE(Position0_Half4);
    DESTROYWDEVICE_VERTEXDECL_BUILTINTYPE(Position0_Float3);
    DESTROYWDEVICE_VERTEXDECL_BUILTINTYPE(Position0_Float4);
    DESTROYWDEVICE_VERTEXDECL_BUILTINTYPE(Position0_Float3__Color0_UByte4N);
    DESTROYWDEVICE_VERTEXDECL_BUILTINTYPE(Position0_Float4__TexCoord0_Float2);
    DESTROYWDEVICE_VERTEXDECL_BUILTINTYPE(Position0_Float3__Color0_UByte4N__TexCoord0_Float2);
    DESTROYWDEVICE_VERTEXDECL_BUILTINTYPE(Position0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N);
    DESTROYWDEVICE_VERTEXDECL_BUILTINTYPE(Position0_Float3__TexCoord0_Float2__Normal0_Float3__Tangent0_Float3__Binormal0_Float3);
    DESTROYWDEVICE_VERTEXDECL_BUILTINTYPE(Position0_Float3__TexCoord0_Half2__Normal0_UByte4N__Tangent0_UByte4N__Binormal0_UByte4N);
    DESTROYWDEVICE_VERTEXDECL_BUILTINTYPE(Position0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N);

#undef DESTROYWDEVICE_VERTEXDECL_BUILTINTYPE
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
