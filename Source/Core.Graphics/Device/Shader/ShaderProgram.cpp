#include "stdafx.h"

#include "ShaderProgram.h"

#include "ConstantBufferLayout.h"
#include "Device/BindName.h"
#include "Device/DeviceEncapsulator.h"
#include "Device/DeviceEncapsulatorException.h"
#include "Device/Geometry/VertexDeclaration.h"

#include "ShaderCompiled.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ShaderProgram::ShaderProgram(
    const Graphics::VertexDeclaration* vertexDeclaration,
    ShaderProgramType programType,
    ShaderProfileType profileType,
    const ShaderCompiled* compiled,
    bool sharable )
:   DeviceResourceSharable(DeviceResourceType::ShaderProgram, sharable)
,   _data(0)
,   _vertexDeclaration(vertexDeclaration)
,   _compiled(compiled) {
    Assert(_compiled);
    Assert(false == _compiled->Blob().empty());
    bitprogram_type::InplaceSet(_data, static_cast<size_t>(programType));
    bitprofile_type::InplaceSet(_data, static_cast<size_t>(profileType));
}
//----------------------------------------------------------------------------
ShaderProgram::~ShaderProgram() {
    Assert(!_deviceAPIDependantProgram);
}
//----------------------------------------------------------------------------
bool ShaderProgram::Available() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    return nullptr != _deviceAPIDependantProgram;
}
//----------------------------------------------------------------------------
DeviceAPIDependantEntity *ShaderProgram::TerminalEntity() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    return _deviceAPIDependantProgram.get();
}
//----------------------------------------------------------------------------
void ShaderProgram::Create(IDeviceAPIEncapsulator* device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(!_deviceAPIDependantProgram);

    _deviceAPIDependantProgram = device->CreateShaderProgram(this);

    Assert(_deviceAPIDependantProgram);
}
//----------------------------------------------------------------------------
void ShaderProgram::Destroy(IDeviceAPIEncapsulator* device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(device);
    Assert(_deviceAPIDependantProgram);

    device->DestroyShaderProgram(this, _deviceAPIDependantProgram);

    Assert(!_deviceAPIDependantProgram);
}
//----------------------------------------------------------------------------
size_t ShaderProgram::VirtualSharedKeyHashValue() const {
    return size_t(_compiled->Fingerprint());
}
//----------------------------------------------------------------------------
bool ShaderProgram::VirtualMatchTerminalEntity(const DeviceAPIDependantEntity *entity) const {
    const Graphics::DeviceAPIDependantShaderProgram *shaderProgram =
        checked_cast<const Graphics::DeviceAPIDependantShaderProgram *>(entity);
    return  shaderProgram->VertexDeclaration() == VertexDeclaration() &&
            shaderProgram->ProgramType() == ProgramType() &&
            shaderProgram->ProfileType() == ProfileType() &&
            shaderProgram->Compiled() == _compiled;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceAPIDependantShaderProgram::DeviceAPIDependantShaderProgram(
    IDeviceAPIEncapsulator* device,
    const ShaderProgram* resource  )
:   TypedDeviceAPIDependantEntity<ShaderProgram>(device->APIEncapsulator(), resource)
,   _data(0)
,   _compiled(resource->Compiled())
,   _vertexDeclaration(resource->VertexDeclaration()) {
    Assert(resource);
    Assert(_compiled);
    bitprogram_type::InplaceSet(_data, static_cast<size_t>(resource->ProgramType()));
    bitprofile_type::InplaceSet(_data, static_cast<size_t>(resource->ProfileType()));
}
//----------------------------------------------------------------------------
DeviceAPIDependantShaderProgram::~DeviceAPIDependantShaderProgram() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
