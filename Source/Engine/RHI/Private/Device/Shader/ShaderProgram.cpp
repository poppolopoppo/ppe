#include "stdafx.h"

#include "ShaderProgram.h"

#include "ConstantBufferLayout.h"
#include "Name.h"

#include "Device/DeviceEncapsulator.h"
#include "Device/DeviceEncapsulatorException.h"
#include "Device/Geometry/VertexDeclaration.h"

#include "ShaderCompiled.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FShaderProgram::FShaderProgram(
    const Graphics::FVertexDeclaration* vertexDeclaration,
    EShaderProgramType programType,
    EShaderProfileType profileType,
    const FShaderCompiled* compiled,
    bool sharable )
:   FDeviceResourceSharable(EDeviceResourceType::FShaderProgram, sharable)
,   _data(0)
,   _vertexDeclaration(vertexDeclaration)
,   _compiled(compiled) {
    Assert(_compiled);
    Assert(false == _compiled->Blob().empty());
    bitprogram_type::InplaceSet(_data, static_cast<size_t>(programType));
    bitprofile_type::InplaceSet(_data, static_cast<size_t>(profileType));
}
//----------------------------------------------------------------------------
FShaderProgram::~FShaderProgram() {
    Assert(!_deviceAPIDependantProgram);
}
//----------------------------------------------------------------------------
bool FShaderProgram::Available() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    return nullptr != _deviceAPIDependantProgram;
}
//----------------------------------------------------------------------------
FDeviceAPIDependantEntity *FShaderProgram::TerminalEntity() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    return _deviceAPIDependantProgram.get();
}
//----------------------------------------------------------------------------
void FShaderProgram::Create(IDeviceAPIEncapsulator* device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(!_deviceAPIDependantProgram);

    _deviceAPIDependantProgram = device->CreateShaderProgram(this);

    Assert(_deviceAPIDependantProgram);
}
//----------------------------------------------------------------------------
void FShaderProgram::Destroy(IDeviceAPIEncapsulator* device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(_deviceAPIDependantProgram);

    device->DestroyShaderProgram(this, _deviceAPIDependantProgram);

    Assert(!_deviceAPIDependantProgram);
}
//----------------------------------------------------------------------------
size_t FShaderProgram::VirtualSharedKeyHashValue() const {
    return size_t(_compiled->Fingerprint());
}
//----------------------------------------------------------------------------
bool FShaderProgram::VirtualMatchTerminalEntity(const FDeviceAPIDependantEntity *entity) const {
    const Graphics::FDeviceAPIDependantShaderProgram *shaderProgram =
        checked_cast<const Graphics::FDeviceAPIDependantShaderProgram *>(entity);
    return  shaderProgram->VertexDeclaration() == VertexDeclaration() &&
            shaderProgram->ProgramType() == ProgramType() &&
            shaderProgram->ProfileType() == ProfileType() &&
            shaderProgram->Compiled() == _compiled;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDeviceAPIDependantShaderProgram::FDeviceAPIDependantShaderProgram(
    IDeviceAPIEncapsulator* device,
    const FShaderProgram* resource  )
:   TTypedDeviceAPIDependantEntity<FShaderProgram>(device->APIEncapsulator(), resource)
,   _data(0)
,   _compiled(resource->Compiled())
,   _vertexDeclaration(resource->VertexDeclaration()) {
    Assert(resource);
    Assert(_compiled);
    bitprogram_type::InplaceSet(_data, static_cast<size_t>(resource->ProgramType()));
    bitprofile_type::InplaceSet(_data, static_cast<size_t>(resource->ProfileType()));
}
//----------------------------------------------------------------------------
FDeviceAPIDependantShaderProgram::~FDeviceAPIDependantShaderProgram() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
