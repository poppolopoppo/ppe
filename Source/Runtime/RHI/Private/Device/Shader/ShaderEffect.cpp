#include "stdafx.h"

#include "ShaderEffect.h"

#include "Device/DeviceEncapsulator.h"
#include "Device/Geometry/VertexDeclaration.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FShaderEffect::FShaderEffect(const Graphics::FVertexDeclaration *vertexDeclaration)
:   FDeviceResource(EDeviceResourceType::FShaderEffect)
,   _vertexDeclaration(vertexDeclaration) {
    Assert(vertexDeclaration);
}
//----------------------------------------------------------------------------
FShaderEffect::~FShaderEffect() {}
//----------------------------------------------------------------------------
bool FShaderEffect::Available() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    return nullptr != _deviceAPIDependantEffect;
}
//----------------------------------------------------------------------------
FDeviceAPIDependantEntity *FShaderEffect::TerminalEntity() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    return _deviceAPIDependantEffect.get();
}
//----------------------------------------------------------------------------
void FShaderEffect::SetStageProgram(EShaderProgramType stage, PShaderProgram&& program) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!Frozen());
    Assert(program);
    Assert(program->Frozen());
    Assert(size_t(stage) < lengthof(_stagePrograms));

    _stagePrograms[size_t(stage)] = std::move(program);
}
//----------------------------------------------------------------------------
void FShaderEffect::ResetStageProgram(EShaderProgramType stage) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(size_t(stage) < lengthof(_stagePrograms));

    if (_stagePrograms[size_t(stage)])
        RemoveRef_AssertReachZero(_stagePrograms[size_t(stage)]);
}
//----------------------------------------------------------------------------
void FShaderEffect::Create(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(!_deviceAPIDependantEffect);

    _deviceAPIDependantEffect = device->CreateShaderEffect(this);

    Assert(_deviceAPIDependantEffect);
}
//----------------------------------------------------------------------------
void FShaderEffect::Destroy(IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(Frozen());
    Assert(_deviceAPIDependantEffect);

    device->DestroyShaderEffect(this, _deviceAPIDependantEffect);

    Assert(!_deviceAPIDependantEffect);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDeviceAPIDependantShaderEffect::FDeviceAPIDependantShaderEffect(
    IDeviceAPIEncapsulator *device, const FShaderEffect *resource)
:   TTypedDeviceAPIDependantEntity<FShaderEffect>(device->APIEncapsulator(), resource) {
    Assert(resource);
}
//----------------------------------------------------------------------------
FDeviceAPIDependantShaderEffect::~FDeviceAPIDependantShaderEffect() {}
//----------------------------------------------------------------------------
size_t FDeviceAPIDependantShaderEffect::VideoMemorySizeInBytes() const {
    return 0; // TODO ?
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
