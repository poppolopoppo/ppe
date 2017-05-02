#include "stdafx.h"

#include "VertexTypes.h"

#include "Device/DeviceEncapsulator.h"
#include "VertexDeclaration.h"

#include "Core/Container/StringHashMap.h"
#include "Core/IO/StringView.h"
#include "Core/Memory/AlignedStorage.h"
#include "Core/Meta/Singleton.h"

#define FOREACH_VERTEXDECL_BUILTINTYPE(_Foreach) \
    _Foreach(FPosition0_UByte4) \
    _Foreach(FPosition0_UShort2) \
    _Foreach(FPosition0_Half2) \
    _Foreach(FPosition0_Half4) \
    _Foreach(FPosition0_Float3) \
    _Foreach(FPosition0_Float4) \
    _Foreach(FPosition0_Float3__Color0_UByte4N) \
    _Foreach(FPosition0_Float3__Color0_UByte4N__Normal0_UX10Y10Z10W2N) \
    _Foreach(FPosition0_Float3__Normal0_UX10Y10Z10W2N) \
    _Foreach(FPosition0_Float3__TexCoord0_Half2) \
    _Foreach(FPosition0_Float4__TexCoord0_Float2) \
    _Foreach(FPosition0_Float4__TexCoord0_Half2) \
    _Foreach(FPosition0_Float3__Color0_UByte4N__TexCoord0_Float2) \
    _Foreach(FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2) \
    _Foreach(FPosition0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N) \
    _Foreach(FPosition0_Float3__TexCoord0_Half2__Normal0_UByte4N) \
    _Foreach(FPosition0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N) \
    _Foreach(FPosition0_Float3__TexCoord0_Float2__Normal0_Float3) \
    _Foreach(FPosition0_Float3__TexCoord0_Float2__Normal0_Float3__Tangent0_Float3__Binormal0_Float3) \
    _Foreach(FPosition0_Float3__TexCoord0_Half2__Normal0_UByte4N__Tangent0_UByte4N__Binormal0_UByte4N) \
    _Foreach(FPosition0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N) \
    _Foreach(FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N) \
    _Foreach(FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N)

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
typedef STRINGVIEW_HASHMAP(Vertex, PCVertexDeclaration, ECase::Sensitive) stringview_to_vdecl_type;
class FVertexDeclarationDico_ : Meta::TSingleton<stringview_to_vdecl_type, FVertexDeclarationDico_> {
    typedef Meta::TSingleton<stringview_to_vdecl_type, FVertexDeclarationDico_> parent_type;
public:
#ifdef WITH_CORE_ASSERT
    using parent_type::HasInstance;
#endif

    static void Create() {
        AssertIsMainThread();
        parent_type::Create();
    }

    static stringview_to_vdecl_type& Instance() {
        AssertIsMainThread();
        return parent_type::Instance();
    }

    static void Destroy() {
        AssertIsMainThread();
        parent_type::Destroy();
    }
};
//----------------------------------------------------------------------------
static FString VertexTypeName_(const FVertexDeclaration* vdecl) {
    FOStringStream oss;

    oss << "Vertex";
    for (const FValueBlock::TField& subPart : vdecl->SubParts())
        Format( oss, "__{0}{1}_{2}", subPart.Name(), subPart.Index(), ValueTypeToCStr(subPart.Type()) );

    return oss.str();
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#define DEF_VERTEXDECL_BUILTINTYPE_STATICPOD(_Name) \
    static POD_STORAGE(FVertexDeclaration) CONCAT(GVertexDeclarationPOD_, _Name);
//----------------------------------------------------------------------------
FOREACH_VERTEXDECL_BUILTINTYPE(DEF_VERTEXDECL_BUILTINTYPE_STATICPOD)
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
    const FVertexDeclaration *_Name::Declaration = nullptr;
//----------------------------------------------------------------------------
FOREACH_VERTEXDECL_BUILTINTYPE(DEF_VERTEXDECL_BUILTINTYPE_STATICPTR)
//----------------------------------------------------------------------------
#undef DEF_VERTEXDECL_BUILTINTYPE_STATICPTR
//----------------------------------------------------------------------------
} //!namespace Vertex
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void VertexTypes_Start() {
    FVertexDeclarationDico_::Create();

    #define CREATE_VERTEXDECL_BUILTINTYPE(_Name) \
        Assert(nullptr == Vertex::_Name::Declaration); \
        Vertex::_Name::Declaration = ::new ((void*)&CONCAT(GVertexDeclarationPOD_, _Name)) FVertexDeclaration(); \
        FVertexDeclarator vdecl(remove_const(Vertex::_Name::Declaration));

    {
        CREATE_VERTEXDECL_BUILTINTYPE(FPosition0_UByte4);
        vdecl.AddSubPart(FVertexSemantic::Position, 0, &Vertex::FPosition0_UByte4::Position0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(FPosition0_UShort2);
        vdecl.AddSubPart(FVertexSemantic::Position, 0, &Vertex::FPosition0_UShort2::Position0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(FPosition0_Half2);
        vdecl.AddSubPart(FVertexSemantic::Position, 0, &Vertex::FPosition0_Half2::Position0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(FPosition0_Half4);
        vdecl.AddSubPart(FVertexSemantic::Position, 0, &Vertex::FPosition0_Half4::Position0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(FPosition0_Float3);
        vdecl.AddSubPart(FVertexSemantic::Position, 0, &Vertex::FPosition0_Float3::Position0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(FPosition0_Float4);
        vdecl.AddSubPart(FVertexSemantic::Position, 0, &Vertex::FPosition0_Float4::Position0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(FPosition0_Float3__Color0_UByte4N);
        vdecl.AddSubPart(FVertexSemantic::Position, 0, &Vertex::FPosition0_Float3__Color0_UByte4N::Position0);
        vdecl.AddSubPart(FVertexSemantic::Color, 0, &Vertex::FPosition0_Float3__Color0_UByte4N::Color0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(FPosition0_Float3__Color0_UByte4N__Normal0_UX10Y10Z10W2N);
        vdecl.AddSubPart(FVertexSemantic::Position, 0, &Vertex::FPosition0_Float3__Color0_UByte4N__Normal0_UX10Y10Z10W2N::Position0);
        vdecl.AddSubPart(FVertexSemantic::Color, 0, &Vertex::FPosition0_Float3__Color0_UByte4N__Normal0_UX10Y10Z10W2N::Color0);
        vdecl.AddSubPart(FVertexSemantic::Normal, 0, &Vertex::FPosition0_Float3__Color0_UByte4N__Normal0_UX10Y10Z10W2N::Normal0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(FPosition0_Float3__Normal0_UX10Y10Z10W2N);
        vdecl.AddSubPart(FVertexSemantic::Position, 0, &Vertex::FPosition0_Float3__Normal0_UX10Y10Z10W2N::Position0);
        vdecl.AddSubPart(FVertexSemantic::Normal, 0, &Vertex::FPosition0_Float3__Normal0_UX10Y10Z10W2N::Normal0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(FPosition0_Float3__TexCoord0_Half2);
        vdecl.AddSubPart(FVertexSemantic::Position, 0, &Vertex::FPosition0_Float3__TexCoord0_Half2::Position0);
        vdecl.AddSubPart(FVertexSemantic::TexCoord, 0, &Vertex::FPosition0_Float3__TexCoord0_Half2::TexCoord0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(FPosition0_Float4__TexCoord0_Float2);
        vdecl.AddSubPart(FVertexSemantic::Position, 0, &Vertex::FPosition0_Float4__TexCoord0_Float2::Position0);
        vdecl.AddSubPart(FVertexSemantic::TexCoord, 0, &Vertex::FPosition0_Float4__TexCoord0_Float2::TexCoord0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(FPosition0_Float4__TexCoord0_Half2);
        vdecl.AddSubPart(FVertexSemantic::Position, 0, &Vertex::FPosition0_Float4__TexCoord0_Half2::Position0);
        vdecl.AddSubPart(FVertexSemantic::TexCoord, 0, &Vertex::FPosition0_Float4__TexCoord0_Half2::TexCoord0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(FPosition0_Float3__Color0_UByte4N__TexCoord0_Float2);
        vdecl.AddSubPart(FVertexSemantic::Position, 0, &Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Float2::Position0);
        vdecl.AddSubPart(FVertexSemantic::Color, 0, &Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Float2::Color0);
        vdecl.AddSubPart(FVertexSemantic::TexCoord, 0, &Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Float2::TexCoord0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2);
        vdecl.AddSubPart(FVertexSemantic::Position, 0, &Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2::Position0);
        vdecl.AddSubPart(FVertexSemantic::Color, 0, &Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2::Color0);
        vdecl.AddSubPart(FVertexSemantic::TexCoord, 0, &Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2::TexCoord0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(FPosition0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N);
        vdecl.AddSubPart(FVertexSemantic::Position, 0, &Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N::Position0);
        vdecl.AddSubPart(FVertexSemantic::Color, 0, &Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N::Color0);
        vdecl.AddSubPart(FVertexSemantic::TexCoord, 0, &Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N::TexCoord0);
        vdecl.AddSubPart(FVertexSemantic::Normal, 0, &Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Float2__Normal0_UX10Y10Z10W2N::Normal0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(FPosition0_Float3__TexCoord0_Half2__Normal0_UByte4N);
        vdecl.AddSubPart(FVertexSemantic::Position, 0, &Vertex::FPosition0_Float3__TexCoord0_Half2__Normal0_UByte4N::Position0);
        vdecl.AddSubPart(FVertexSemantic::TexCoord, 0, &Vertex::FPosition0_Float3__TexCoord0_Half2__Normal0_UByte4N::TexCoord0);
        vdecl.AddSubPart(FVertexSemantic::Normal, 0, &Vertex::FPosition0_Float3__TexCoord0_Half2__Normal0_UByte4N::Normal0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(FPosition0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N);
        vdecl.AddSubPart(FVertexSemantic::Position, 0, &Vertex::FPosition0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N::Position0);
        vdecl.AddSubPart(FVertexSemantic::TexCoord, 0, &Vertex::FPosition0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N::TexCoord0);
        vdecl.AddSubPart(FVertexSemantic::Normal, 0, &Vertex::FPosition0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N::Normal0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(FPosition0_Float3__TexCoord0_Float2__Normal0_Float3);
        vdecl.AddSubPart(FVertexSemantic::Position, 0, &Vertex::FPosition0_Float3__TexCoord0_Float2__Normal0_Float3::Position0);
        vdecl.AddSubPart(FVertexSemantic::TexCoord, 0, &Vertex::FPosition0_Float3__TexCoord0_Float2__Normal0_Float3::TexCoord0);
        vdecl.AddSubPart(FVertexSemantic::Normal, 0, &Vertex::FPosition0_Float3__TexCoord0_Float2__Normal0_Float3::Normal0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(FPosition0_Float3__TexCoord0_Float2__Normal0_Float3__Tangent0_Float3__Binormal0_Float3);
        vdecl.AddSubPart(FVertexSemantic::Position, 0, &Vertex::FPosition0_Float3__TexCoord0_Float2__Normal0_Float3__Tangent0_Float3__Binormal0_Float3::Position0);
        vdecl.AddSubPart(FVertexSemantic::TexCoord, 0, &Vertex::FPosition0_Float3__TexCoord0_Float2__Normal0_Float3__Tangent0_Float3__Binormal0_Float3::TexCoord0);
        vdecl.AddSubPart(FVertexSemantic::Normal, 0, &Vertex::FPosition0_Float3__TexCoord0_Float2__Normal0_Float3__Tangent0_Float3__Binormal0_Float3::Normal0);
        vdecl.AddSubPart(FVertexSemantic::Tangent, 0, &Vertex::FPosition0_Float3__TexCoord0_Float2__Normal0_Float3__Tangent0_Float3__Binormal0_Float3::Tangent0);
        vdecl.AddSubPart(FVertexSemantic::Binormal, 0, &Vertex::FPosition0_Float3__TexCoord0_Float2__Normal0_Float3__Tangent0_Float3__Binormal0_Float3::Binormal0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(FPosition0_Float3__TexCoord0_Half2__Normal0_UByte4N__Tangent0_UByte4N__Binormal0_UByte4N);
        vdecl.AddSubPart(FVertexSemantic::Position, 0, &Vertex::FPosition0_Float3__TexCoord0_Half2__Normal0_UByte4N__Tangent0_UByte4N__Binormal0_UByte4N::Position0);
        vdecl.AddSubPart(FVertexSemantic::TexCoord, 0, &Vertex::FPosition0_Float3__TexCoord0_Half2__Normal0_UByte4N__Tangent0_UByte4N__Binormal0_UByte4N::TexCoord0);
        vdecl.AddSubPart(FVertexSemantic::Normal, 0, &Vertex::FPosition0_Float3__TexCoord0_Half2__Normal0_UByte4N__Tangent0_UByte4N__Binormal0_UByte4N::Normal0);
        vdecl.AddSubPart(FVertexSemantic::Tangent, 0, &Vertex::FPosition0_Float3__TexCoord0_Half2__Normal0_UByte4N__Tangent0_UByte4N__Binormal0_UByte4N::Tangent0);
        vdecl.AddSubPart(FVertexSemantic::Binormal, 0, &Vertex::FPosition0_Float3__TexCoord0_Half2__Normal0_UByte4N__Tangent0_UByte4N__Binormal0_UByte4N::Binormal0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(FPosition0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N);
        vdecl.AddSubPart(FVertexSemantic::Position, 0, &Vertex::FPosition0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N::Position0);
        vdecl.AddSubPart(FVertexSemantic::TexCoord, 0, &Vertex::FPosition0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N::TexCoord0);
        vdecl.AddSubPart(FVertexSemantic::Normal, 0, &Vertex::FPosition0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N::Normal0);
        vdecl.AddSubPart(FVertexSemantic::Tangent, 0, &Vertex::FPosition0_Float3__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N::Tangent0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N);
        vdecl.AddSubPart(FVertexSemantic::Position, 0, &Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N::Position0);
        vdecl.AddSubPart(FVertexSemantic::Color, 0, &Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N::Color0);
        vdecl.AddSubPart(FVertexSemantic::TexCoord, 0, &Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N::TexCoord0);
        vdecl.AddSubPart(FVertexSemantic::Normal, 0, &Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N::Normal0);
    }
    {
        CREATE_VERTEXDECL_BUILTINTYPE(FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N);
        vdecl.AddSubPart(FVertexSemantic::Position, 0, &Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N::Position0);
        vdecl.AddSubPart(FVertexSemantic::Color, 0, &Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N::Color0);
        vdecl.AddSubPart(FVertexSemantic::TexCoord, 0, &Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N::TexCoord0);
        vdecl.AddSubPart(FVertexSemantic::Normal, 0, &Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N::Normal0);
        vdecl.AddSubPart(FVertexSemantic::Tangent, 0, &Vertex::FPosition0_Float3__Color0_UByte4N__TexCoord0_Half2__Normal0_UX10Y10Z10W2N__Tangent0_UX10Y10Z10W2N::Tangent0);
    }

#undef CREATE_VERTEXDECL_BUILTINTYPE
}
//----------------------------------------------------------------------------
void VertexTypes_Shutdown() {
#define DESTROY_VERTEXDECL_BUILTINTYPE(_Name) \
    Assert(Vertex::_Name::Declaration); \
    AddRef(Vertex::_Name::Declaration); \
    UnregisterVertexType(Vertex::_Name::Declaration); \
    Assert((void *)&CONCAT(GVertexDeclarationPOD_, _Name) == Vertex::_Name::Declaration); \
    RemoveRef_AssertReachZero_NoDelete(Vertex::_Name::Declaration);

    FOREACH_VERTEXDECL_BUILTINTYPE(DESTROY_VERTEXDECL_BUILTINTYPE)

#undef DESTROY_VERTEXDECL_BUILTINTYPE

    FVertexDeclarationDico_::Destroy();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void VertexTypes_OnDeviceCreate(FDeviceEncapsulator *device) {
#define CREATEWDEVICE_VERTEXDECL_BUILTINTYPE(_NAME) \
    remove_const(Vertex::_NAME::Declaration)->Create(device->Device());

    FOREACH_VERTEXDECL_BUILTINTYPE(CREATEWDEVICE_VERTEXDECL_BUILTINTYPE)

#undef CREATEWDEVICE_VERTEXDECL_BUILTINTYPE
}
//----------------------------------------------------------------------------
void VertexTypes_OnDeviceDestroy(FDeviceEncapsulator *device) {
#define DESTROYWDEVICE_VERTEXDECL_BUILTINTYPE(_NAME) \
    remove_const(Vertex::_NAME::Declaration)->Destroy(device->Device());

    FOREACH_VERTEXDECL_BUILTINTYPE(DESTROYWDEVICE_VERTEXDECL_BUILTINTYPE)

#undef DESTROYWDEVICE_VERTEXDECL_BUILTINTYPE
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RegisterVertexType(const FVertexDeclaration* vdecl) {
    Assert(vdecl);
    Assert(vdecl->Frozen());
    Assert(vdecl->ResourceName() == VertexTypeName_(vdecl));

    PCVertexDeclaration pcvdecl(vdecl);
    Insert_AssertUnique(FVertexDeclarationDico_::Instance(), MakeStringView(vdecl->ResourceName()), pcvdecl);
}
//----------------------------------------------------------------------------
void UnregisterVertexType(const FVertexDeclaration* vdecl) {
    Assert(vdecl);
    Assert(vdecl->Frozen());
    Assert(vdecl->ResourceName() == VertexTypeName_(vdecl));

    PCVertexDeclaration pcvdecl(vdecl);
    Remove_AssertExistsAndSameValue(FVertexDeclarationDico_::Instance(), MakeStringView(vdecl->ResourceName()), pcvdecl);
}
//----------------------------------------------------------------------------
const FVertexDeclaration* VertexTypeByName(const FStringView& name) {
    PCVertexDeclaration vdecl;
    TryGetValue(FVertexDeclarationDico_::Instance(), name, &vdecl);
    return vdecl.get();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVertexDeclarator::FVertexDeclarator(FVertexDeclaration* vdecl) : _vdecl(vdecl) {
    Assert(_vdecl);
    Assert(!_vdecl->Frozen());
}
//----------------------------------------------------------------------------
FVertexDeclarator::~FVertexDeclarator() {
    Assert(_vdecl);
    Assert(!_vdecl->Frozen());

    _vdecl->SetResourceName(VertexTypeName_(_vdecl.get()));
    _vdecl->Freeze();

    RegisterVertexType(_vdecl.get());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
