#include "stdafx.h"

#include "ShaderEffect.h"

#include "Device/DeviceEncapsulator.h"
#include "Device/Geometry/VertexDeclaration.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ShaderEffect::ShaderEffect(const Graphics::VertexDeclaration *vertexDeclaration)
:   DeviceResource(DeviceResourceType::ShaderEffect)
,   _vertexDeclaration(vertexDeclaration) {
    Assert(vertexDeclaration);
}
//----------------------------------------------------------------------------
ShaderEffect::~ShaderEffect() {}
//----------------------------------------------------------------------------
bool ShaderEffect::Available() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    return nullptr != _deviceAPIDependantEffect;
}
//----------------------------------------------------------------------------
DeviceAPIDependantEntity *ShaderEffect::TerminalEntity() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    return _deviceAPIDependantEffect.get();
}
//----------------------------------------------------------------------------
void ShaderEffect::SetStageProgram(ShaderProgramType stage, PShaderProgram&& program) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!Frozen());
    Assert(program);
    Assert(program->Frozen());
    Assert(size_t(stage) < lengthof(_stagePrograms));

    _stagePrograms[size_t(stage)] = std::move(program);
}
//----------------------------------------------------------------------------
void ShaderEffect::ResetStageProgram(ShaderProgramType stage) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(size_t(stage) < lengthof(_stagePrograms));

    if (_stagePrograms[size_t(stage)])
        RemoveRef_AssertReachZero(_stagePrograms[size_t(stage)]);
}
//----------------------------------------------------------------------------
void ShaderEffect::Create(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(!_deviceAPIDependantEffect);

    _deviceAPIDependantEffect = device->CreateShaderEffect(this);

    Assert(_deviceAPIDependantEffect);
}
//----------------------------------------------------------------------------
void ShaderEffect::Destroy(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(_deviceAPIDependantEffect);

    device->DestroyShaderEffect(this, _deviceAPIDependantEffect);

    Assert(!_deviceAPIDependantEffect);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceAPIDependantShaderEffect::DeviceAPIDependantShaderEffect(
    IDeviceAPIEncapsulator *device, const ShaderEffect *resource)
:   TypedDeviceAPIDependantEntity<ShaderEffect>(device->APIEncapsulator(), resource) {
    Assert(resource);
}
//----------------------------------------------------------------------------
DeviceAPIDependantShaderEffect::~DeviceAPIDependantShaderEffect() {}
//----------------------------------------------------------------------------
size_t DeviceAPIDependantShaderEffect::VideoMemorySizeInBytes() const {
    return 0; // TODO ?
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
