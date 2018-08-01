#include "stdafx.h"

#include "VertexDeclaration.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/IO/Format.h"
#include "Core/IO/Stream.h"
#include "Core/Memory/MemoryView.h"
#include "Core/Meta/AlignedStorage.h"

#include "Device/DeviceEncapsulator.h"
#include "VertexTypes.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Graphics, FVertexDeclaration, );
//----------------------------------------------------------------------------
FVertexDeclaration::FVertexDeclaration()
:   FDeviceResource(EDeviceResourceType::FVertexDeclaration) {}
//----------------------------------------------------------------------------
FVertexDeclaration::~FVertexDeclaration() {
    Assert(!_deviceAPIDependantDeclaration);
}
//----------------------------------------------------------------------------
bool FVertexDeclaration::Available() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    return nullptr != _deviceAPIDependantDeclaration;
}
//----------------------------------------------------------------------------
FDeviceAPIDependantEntity *FVertexDeclaration::TerminalEntity() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    return _deviceAPIDependantDeclaration.get();
}
//----------------------------------------------------------------------------
void FVertexDeclaration::AddSubPart(const FVertexSemantic& semantic, size_t index, EValueType type, size_t offset) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!Frozen());
    Assert(!SubPartBySemanticIFP(semantic, index));
    Assert(_block.size() < MaxSubPartCount);

    _block.Add(semantic, type, offset, index);
}
//----------------------------------------------------------------------------
const FValueField& FVertexDeclaration::SubPartBySemantic(const FVertexSemantic& semantic, size_t index) const {
    return _block.FindByNameAndIndex(semantic, index);
}
//----------------------------------------------------------------------------
const FValueField* FVertexDeclaration::SubPartBySemanticIFP(const FVertexSemantic& semantic, size_t index) const {
    return _block.FindByNameAndIndexIFP(semantic, index);
}
//----------------------------------------------------------------------------
void FVertexDeclaration::CopyVertex(const TMemoryView<u8>& dst, const TMemoryView<const u8>& src) const {
    Assert(Frozen());
    _block.Copy(dst, src);
}
//----------------------------------------------------------------------------
void FVertexDeclaration::FillSubstitutions(VECTOR(Shader, TPair<FString COMMA FString>)& substitutions) const {
    UNUSED(substitutions);
}
//----------------------------------------------------------------------------
void FVertexDeclaration::Create(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(!_deviceAPIDependantDeclaration);

    _deviceAPIDependantDeclaration = device->CreateVertexDeclaration(this);

    Assert(_deviceAPIDependantDeclaration);
}
//----------------------------------------------------------------------------
void FVertexDeclaration::Destroy(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(_deviceAPIDependantDeclaration);

    device->DestroyVertexDeclaration(this, _deviceAPIDependantDeclaration);

    Assert(!_deviceAPIDependantDeclaration);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDeviceAPIDependantVertexDeclaration::FDeviceAPIDependantVertexDeclaration(IDeviceAPIEncapsulator *device, const FVertexDeclaration *resource)
:   TTypedDeviceAPIDependantEntity<FVertexDeclaration>(device->APIEncapsulator(), resource) {
    Assert(resource);
}
//----------------------------------------------------------------------------
FDeviceAPIDependantVertexDeclaration::~FDeviceAPIDependantVertexDeclaration() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define FOREACH_VERTEXSEMANTIC_NAME(_Macro) \
    _Macro(Position) \
    _Macro(TexCoord) \
    _Macro(Color) \
    _Macro(Normal) \
    _Macro(Tangent) \
    _Macro(Binormal)
//----------------------------------------------------------------------------
#define DEF_VERTEXSEMANTIC_ACCESSOR(_Name) \
    const FVertexSemantic FVertexSemantic::_Name;
FOREACH_VERTEXSEMANTIC_NAME(DEF_VERTEXSEMANTIC_ACCESSOR)
#undef DEF_VERTEXSEMANTIC_ACCESSOR
//----------------------------------------------------------------------------
FVertexSemantic FVertexSemantic::FromName(const FName& name) {
#define DEF_VERTEXSEMANTIC_ASSERT(_Name) FVertexSemantic::_Name == name ||
    Assert(
        FOREACH_VERTEXSEMANTIC_NAME(DEF_VERTEXSEMANTIC_ASSERT)
        true
    );
#undef DEF_VERTEXSEMANTIC_ASSERT
    return FVertexSemantic(name);
}
//----------------------------------------------------------------------------
void FVertexDeclaration::Start() {
#define DEF_VERTEXSEMANTIC_START(_Name) \
    INPLACE_NEW(std::addressof(FVertexSemantic::_Name), FVertexSemantic)(FName(STRINGIZE(_Name)));
    FOREACH_VERTEXSEMANTIC_NAME(DEF_VERTEXSEMANTIC_START)
#undef DEF_VERTEXSEMANTIC_START

    VertexTypes_Start();
}
//----------------------------------------------------------------------------
void FVertexDeclaration::Shutdown() {
    VertexTypes_Shutdown();

#define DEF_VERTEXSEMANTIC_SHUTDOWN(_Name) \
    remove_const(&FVertexSemantic::_Name)->~FVertexSemantic();
    FOREACH_VERTEXSEMANTIC_NAME(DEF_VERTEXSEMANTIC_SHUTDOWN)
#undef DEF_VERTEXSEMANTIC_SHUTDOWN
}
//----------------------------------------------------------------------------
#undef FOREACH_VERTEXSEMANTIC_NAME
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FVertexDeclaration::OnDeviceCreate(FDeviceEncapsulator *device) {
    VertexTypes_OnDeviceCreate(device);
}
//----------------------------------------------------------------------------
void FVertexDeclaration::OnDeviceDestroy(FDeviceEncapsulator *device) {
    VertexTypes_OnDeviceDestroy(device);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
