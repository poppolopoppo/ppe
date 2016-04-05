#include "stdafx.h"

#include "VertexTypes.h"

#include "Device/DeviceEncapsulator.h"
#include "VertexDeclaration.h"

#include "Core/Container/StringHashMap.h"
#include "Core/IO/StringSlice.h"
#include "Core/Memory/AlignedStorage.h"
#include "Core/Meta/Singleton.h"

#define EACH_VERTEXDECL_BUILTINTYPE(_Foreach) \
    _Foreach(Position0_UByte4) \
    _Foreach(Position0_UShort2) \
    _Foreach(Position0_Half2) \
    _Foreach(Position0_Half4) \
    _Foreach(Position0_Float3) \
    _Foreach(Position0_Float4) \
    _Foreach(Position0_Float3__Color0_UByte4N) \
    _Foreach(Position0_Float3__Color0_UByte4N__Normal0_UX10Y10Z10W2N) \
    _Foreach(Position0_Float3__Normal0_UX10Y10Z10W2N) \
    _Foreach(Position0_Float3__TexCoord0_Half2) \
    _Foreach(Position0_Float4__TexCoord0_Float2) \
    _Foreach(Position0_Float4__TexCoord0_Half2) \
    _Foreach(Position0_Float3__Color0_UByte4N__TexCoord0_Float2) \
    _Foreach(Position0_Float3__Color0_UByte4N__TexCoord0_Half2) \
    _Foreach(Position0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N) \
    _Foreach(Position0_Float3__TexCoord0_Half2__Normal0_UByte4N) \
    _Foreach(Position0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N) \
    _Foreach(Position0_Float3__TexCoord0_Float2__Normal0_Float3) \
    _Foreach(Position0_Float3__TexCoord0_Float2__Normal0_Float3__Tangent0_Float3__Binormal0_Float3) \
    _Foreach(Position0_Float3__TexCoord0_Half2__Normal0_UByte4N__Tangent0_UByte4N__Binormal0_UByte4N) \
    _Foreach(Position0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N) \
    _Foreach(Position0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N) \
    _Foreach(Position0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N)

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
typedef STRINGSLICE_HASHMAP(Vertex, SCVertexDeclaration, Case::Sensitive) stringslice_to_vdecl_type;
class VertexDeclarationDico_ : Meta::Singleton<stringslice_to_vdecl_type, VertexDeclarationDico_> {
    typedef Meta::Singleton<stringslice_to_vdecl_type, VertexDeclarationDico_> parent_type;
public:
    using parent_type::HasInstance;
    using parent_type::Destroy;

    static void Create(stringslice_to_vdecl_type&& dico) {
        parent_type::Create(std::move(dico));
    }

    static const stringslice_to_vdecl_type& Instance() {
        return parent_type::Instance();
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#define DEF_VERTEXDECL_BUILTINTYPE_STATICPOD(_Name) \
    static POD_STORAGE(VertexDeclaration) CONCAT(gVertexDeclarationPOD_, _Name);
//----------------------------------------------------------------------------
EACH_VERTEXDECL_BUILTINTYPE(DEF_VERTEXDECL_BUILTINTYPE_STATICPOD)
//----------------------------------------------------------------------------
#undef DEF_VERTEXDECL_BUILTINTYPE_STATICPOD
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Vertex {
//----------------------------------------------------------------------------
#define DEF_VERTEXDECL_BUILTINTYPE_STATICPTR(_Name) \
    const VertexDeclaration *_Name::Declaration = nullptr;
//----------------------------------------------------------------------------
EACH_VERTEXDECL_BUILTINTYPE(DEF_VERTEXDECL_BUILTINTYPE_STATICPTR)
//----------------------------------------------------------------------------
#undef DEF_VERTEXDECL_BUILTINTYPE_STATICPTR
//----------------------------------------------------------------------------
} //!namespace Vertex
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void VertexTypes_Start() {
    stringslice_to_vdecl_type dico;
    dico.reserve(128);

    #define CREATE_VERTEXDECL_BUILTINTYPE(_Name) \
        Assert(nullptr == Vertex::_Name::Declaration); \
        void *const pod = reinterpret_cast<void *>(&CONCAT(gVertexDeclarationPOD_, _Name)); \
        Vertex::_Name::Declaration = ::new (pod) VertexDeclaration(); \
        AddRef(Vertex::_Name::Declaration); \
        VertexDeclarator< Vertex::_Name > vdecl(const_cast<VertexDeclaration *>(Vertex::_Name::Declaration)); \
        vdecl.SetResourceName(STRINGIZE(_Name)); \
        const StringSlice key = MakeStringSlice(STRINGIZE(_Name)); \
        Insert_AssertUnique(dico, key, SCVertexDeclaration(Vertex::_Name::Declaration))

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
        CREATE_VERTEXDECL_BUILTINTYPE(Position0_Float3__Color0_UByte4N__Normal0_UX10Y10Z10W2N);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Position>(&Vertex::Position0_Float3__Color0_UByte4N__Normal0_UX10Y10Z10W2N::Position0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Color>(&Vertex::Position0_Float3__Color0_UByte4N__Normal0_UX10Y10Z10W2N::Color0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Normal>(&Vertex::Position0_Float3__Color0_UByte4N__Normal0_UX10Y10Z10W2N::Normal0, 0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(Position0_Float3__Normal0_UX10Y10Z10W2N);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Position>(&Vertex::Position0_Float3__Normal0_UX10Y10Z10W2N::Position0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Normal>(&Vertex::Position0_Float3__Normal0_UX10Y10Z10W2N::Normal0, 0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(Position0_Float3__TexCoord0_Half2);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Position>(&Vertex::Position0_Float3__TexCoord0_Half2::Position0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::TexCoord>(&Vertex::Position0_Float3__TexCoord0_Half2::TexCoord0, 0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(Position0_Float4__TexCoord0_Float2);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Position>(&Vertex::Position0_Float4__TexCoord0_Float2::Position0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::TexCoord>(&Vertex::Position0_Float4__TexCoord0_Float2::TexCoord0, 0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(Position0_Float4__TexCoord0_Half2);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Position>(&Vertex::Position0_Float4__TexCoord0_Half2::Position0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::TexCoord>(&Vertex::Position0_Float4__TexCoord0_Half2::TexCoord0, 0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(Position0_Float3__Color0_UByte4N__TexCoord0_Float2);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Position>(&Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Float2::Position0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Color>(&Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Float2::Color0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::TexCoord>(&Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Float2::TexCoord0, 0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(Position0_Float3__Color0_UByte4N__TexCoord0_Half2);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Position>(&Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Half2::Position0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Color>(&Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Half2::Color0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::TexCoord>(&Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Half2::TexCoord0, 0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(Position0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Position>(&Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N::Position0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Color>(&Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N::Color0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::TexCoord>(&Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N::TexCoord0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Normal>(&Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N::Normal0, 0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(Position0_Float3__TexCoord0_Half2__Normal0_UByte4N);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Position>(&Vertex::Position0_Float3__TexCoord0_Half2__Normal0_UByte4N::Position0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::TexCoord>(&Vertex::Position0_Float3__TexCoord0_Half2__Normal0_UByte4N::TexCoord0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Normal>(&Vertex::Position0_Float3__TexCoord0_Half2__Normal0_UByte4N::Normal0, 0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(Position0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Position>(&Vertex::Position0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N::Position0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::TexCoord>(&Vertex::Position0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N::TexCoord0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Normal>(&Vertex::Position0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N::Normal0, 0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(Position0_Float3__TexCoord0_Float2__Normal0_Float3);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Position>(&Vertex::Position0_Float3__TexCoord0_Float2__Normal0_Float3::Position0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::TexCoord>(&Vertex::Position0_Float3__TexCoord0_Float2__Normal0_Float3::TexCoord0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Normal>(&Vertex::Position0_Float3__TexCoord0_Float2__Normal0_Float3::Normal0, 0);
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
    {
        CREATE_VERTEXDECL_BUILTINTYPE(Position0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Position>(&Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N::Position0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Color>(&Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N::Color0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::TexCoord>(&Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N::TexCoord0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Normal>(&Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N::Normal0, 0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(Position0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Position>(&Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N::Position0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Color>(&Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N::Color0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::TexCoord>(&Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N::TexCoord0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Normal>(&Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N::Normal0, 0);
        vdecl.AddTypedSubPart<VertexSubPartSemantic::Tangent>(&Vertex::Position0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N::Tangent0, 0);
    }

#undef CREATE_VERTEXDECL_BUILTINTYPE

    VertexDeclarationDico_::Create(std::move(dico));
}
//----------------------------------------------------------------------------
void VertexTypes_Shutdown() {

    VertexDeclarationDico_::Destroy();

#define DESTROY_VERTEXDECL_BUILTINTYPE(_Name) \
    Assert(Vertex::_Name::Declaration); \
    Assert((void *)&CONCAT(gVertexDeclarationPOD_, _Name) == Vertex::_Name::Declaration); \
    RemoveRef_AssertReachZero_NoDelete(Vertex::_Name::Declaration); \
    Vertex::_Name::Declaration = nullptr;

    EACH_VERTEXDECL_BUILTINTYPE(DESTROY_VERTEXDECL_BUILTINTYPE)

#undef DESTROY_VERTEXDECL_BUILTINTYPE
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void VertexTypes_OnDeviceCreate(DeviceEncapsulator *device) {
#define CREATEWDEVICE_VERTEXDECL_BUILTINTYPE(_NAME) \
    const_cast<VertexDeclaration *>(Vertex::_NAME::Declaration)->Create(device->Device());

    EACH_VERTEXDECL_BUILTINTYPE(CREATEWDEVICE_VERTEXDECL_BUILTINTYPE)

#undef CREATEWDEVICE_VERTEXDECL_BUILTINTYPE
}
//----------------------------------------------------------------------------
void VertexTypes_OnDeviceDestroy(DeviceEncapsulator *device) {
#define DESTROYWDEVICE_VERTEXDECL_BUILTINTYPE(_NAME) \
    const_cast<VertexDeclaration *>(Vertex::_NAME::Declaration)->Destroy(device->Device());

    EACH_VERTEXDECL_BUILTINTYPE(DESTROYWDEVICE_VERTEXDECL_BUILTINTYPE)

#undef DESTROYWDEVICE_VERTEXDECL_BUILTINTYPE
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const VertexDeclaration* VertexTypeByName(const StringSlice& name) {
    SCVertexDeclaration vdecl(nullptr);
    TryGetValue(VertexDeclarationDico_::Instance(), name, &vdecl);
    return vdecl;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
