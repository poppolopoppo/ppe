#include "stdafx.h"

#include "ShaderEffect.h"

#include "DeviceEncapsulator.h"
#include "VertexDeclaration.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ShaderEffect::ShaderEffect(const Graphics::VertexDeclaration *vertexDeclaration)
:   _vertexDeclaration(vertexDeclaration) {
    Assert(vertexDeclaration);
}
//----------------------------------------------------------------------------
ShaderEffect::~ShaderEffect() {}
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

    for (size_t i = 0; i < size_t(ShaderProgramType::__Count); ++i) {
        PShaderProgram& program = _stagePrograms[i];
        if (!program)
            continue;

        if (program->RefCount() == 1) {
            program->Destroy(device->Encapsulator()->Compiler());
            RemoveRef_AssertReachZero(program);
        }
        else {
            program = nullptr;
        }
    }

    Assert(!_deviceAPIDependantEffect);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceAPIDependantShaderEffect::DeviceAPIDependantShaderEffect(
    IDeviceAPIEncapsulator *device, ShaderEffect *owner)
:   DeviceAPIDependantEntity(device)
,   _owner(owner) {
    Assert(owner);
}
//----------------------------------------------------------------------------
DeviceAPIDependantShaderEffect::~DeviceAPIDependantShaderEffect() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
