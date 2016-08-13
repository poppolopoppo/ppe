#include "stdafx.h"

#include "VertexDeclaration.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/IO/Format.h"
#include "Core/IO/Stream.h"
#include "Core/Memory/AlignedStorage.h"
#include "Core/Memory/MemoryView.h"

#include "Device/DeviceEncapsulator.h"
#include "VertexTypes.h"

#include <sstream>

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Graphics, VertexDeclaration, );
//----------------------------------------------------------------------------
VertexDeclaration::VertexDeclaration()
:   DeviceResource(DeviceResourceType::VertexDeclaration) {}
//----------------------------------------------------------------------------
VertexDeclaration::~VertexDeclaration() {
    Assert(!_deviceAPIDependantDeclaration);
}
//----------------------------------------------------------------------------
bool VertexDeclaration::Available() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    return nullptr != _deviceAPIDependantDeclaration;
}
//----------------------------------------------------------------------------
DeviceAPIDependantEntity *VertexDeclaration::TerminalEntity() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    return _deviceAPIDependantDeclaration.get();
}
//----------------------------------------------------------------------------
void VertexDeclaration::AddSubPart(const VertexSemantic& semantic, size_t index, ValueType type, size_t offset) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!Frozen());
    Assert(!SubPartBySemanticIFP(semantic, index));
    Assert(_block.size() < MaxSubPartCount);

    _block.Add(semantic, type, offset, index);
}
//----------------------------------------------------------------------------
const ValueBlock::Field& VertexDeclaration::SubPartBySemantic(const VertexSemantic& semantic, size_t index) const {
    return _block.FindByNameAndIndex(semantic, index);
}
//----------------------------------------------------------------------------
const ValueBlock::Field* VertexDeclaration::SubPartBySemanticIFP(const VertexSemantic& semantic, size_t index) const {
    return _block.FindByNameAndIndexIFP(semantic, index);
}
//----------------------------------------------------------------------------
void VertexDeclaration::CopyVertex(const MemoryView<u8>& dst, const MemoryView<const u8>& src) const {
    Assert(Frozen());
    _block.Copy(dst, src);
}
//----------------------------------------------------------------------------
void VertexDeclaration::FillSubstitutions(VECTOR_THREAD_LOCAL(Shader, Pair<String COMMA String>)& substitutions) const {
    UNUSED(substitutions);
}
//----------------------------------------------------------------------------
void VertexDeclaration::Create(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(!_deviceAPIDependantDeclaration);

    _deviceAPIDependantDeclaration = device->CreateVertexDeclaration(this);

    Assert(_deviceAPIDependantDeclaration);
}
//----------------------------------------------------------------------------
void VertexDeclaration::Destroy(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(_deviceAPIDependantDeclaration);

    device->DestroyVertexDeclaration(this, _deviceAPIDependantDeclaration);

    Assert(!_deviceAPIDependantDeclaration);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceAPIDependantVertexDeclaration::DeviceAPIDependantVertexDeclaration(IDeviceAPIEncapsulator *device, const VertexDeclaration *resource)
:   TypedDeviceAPIDependantEntity<VertexDeclaration>(device->APIEncapsulator(), resource) {
    Assert(resource);
}
//----------------------------------------------------------------------------
DeviceAPIDependantVertexDeclaration::~DeviceAPIDependantVertexDeclaration() {}
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
    const VertexSemantic VertexSemantic::_Name;
FOREACH_VERTEXSEMANTIC_NAME(DEF_VERTEXSEMANTIC_ACCESSOR)
#undef DEF_VERTEXSEMANTIC_ACCESSOR
//----------------------------------------------------------------------------
void VertexDeclaration::Start() {
#define DEF_VERTEXSEMANTIC_START(_Name) \
    new ((void*)&VertexSemantic::_Name) VertexSemantic(STRINGIZE(_Name));
    FOREACH_VERTEXSEMANTIC_NAME(DEF_VERTEXSEMANTIC_START)
#undef DEF_VERTEXSEMANTIC_START

    VertexTypes_Start();
}
//----------------------------------------------------------------------------
void VertexDeclaration::Shutdown() {
    VertexTypes_Shutdown();

#define DEF_VERTEXSEMANTIC_SHUTDOWN(_Name) \
    const_cast<VertexSemantic*>(&VertexSemantic::_Name)->~VertexSemantic();
    FOREACH_VERTEXSEMANTIC_NAME(DEF_VERTEXSEMANTIC_SHUTDOWN)
#undef DEF_VERTEXSEMANTIC_SHUTDOWN
}
//----------------------------------------------------------------------------
#undef FOREACH_VERTEXSEMANTIC_NAME
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void VertexDeclaration::OnDeviceCreate(DeviceEncapsulator *device) {
    VertexTypes_OnDeviceCreate(device);
}
//----------------------------------------------------------------------------
void VertexDeclaration::OnDeviceDestroy(DeviceEncapsulator *device) {
    VertexTypes_OnDeviceDestroy(device);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
