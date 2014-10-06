#pragma once

#include "Graphics.h"

#include "DeviceResource.h"

#include "ShaderProgram.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
class IDeviceAPIShaderCompilerEncapsulator;
FWD_REFPTR(DeviceAPIDependantShaderEffect);
FWD_REFPTR(VertexDeclaration);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(ShaderEffect);
class ShaderEffect : public TypedDeviceResource<DeviceResourceType::ShaderEffect> {
public:
    explicit ShaderEffect(const Graphics::VertexDeclaration *vertexDeclaration);
    virtual ~ShaderEffect();

    virtual bool Available() const override { return nullptr != _deviceAPIDependantEffect; }
    const PDeviceAPIDependantShaderEffect& DeviceAPIDependantEffect() const {
        Assert(Frozen()); return _deviceAPIDependantEffect;
    }

    const PCVertexDeclaration& VertexDeclaration() const { return _vertexDeclaration; }

    ShaderProgram *StageProgram(ShaderProgramType stage) { return _stagePrograms[size_t(stage)].get(); }
    const ShaderProgram *StageProgram(ShaderProgramType stage) const { return _stagePrograms[size_t(stage)].get(); }

    void SetStageProgram(ShaderProgramType stage, PShaderProgram&& program);
    void ResetStageProgram(ShaderProgramType stage);

    virtual void Create(IDeviceAPIEncapsulator *device);
    virtual void Destroy(IDeviceAPIEncapsulator *device);

private:
    PCVertexDeclaration _vertexDeclaration;
    PShaderProgram _stagePrograms[size_t(ShaderProgramType::__Count)];

    PDeviceAPIDependantShaderEffect _deviceAPIDependantEffect;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DeviceAPIDependantShaderEffect : public DeviceAPIDependantEntity {
public:
    DeviceAPIDependantShaderEffect(IDeviceAPIEncapsulator *device, ShaderEffect *owner);
    virtual ~DeviceAPIDependantShaderEffect();

    const ShaderEffect *Owner() const { return _owner; }

private:
    ShaderEffect *_owner;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
