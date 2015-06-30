#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceResource.h"

#include "Core.Graphics/Device/Shader/ShaderProgram.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
class IDeviceAPIShaderCompiler;
FWD_REFPTR(DeviceAPIDependantShaderEffect);
FWD_REFPTR(VertexDeclaration);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(ShaderEffect);
class ShaderEffect : public DeviceResource {
public:
    explicit ShaderEffect(const Graphics::VertexDeclaration *vertexDeclaration);
    virtual ~ShaderEffect();

    virtual bool Available() const override;
    virtual DeviceAPIDependantEntity *TerminalEntity() const override;

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
class DeviceAPIDependantShaderEffect : public TypedDeviceAPIDependantEntity<ShaderEffect> {
public:
    DeviceAPIDependantShaderEffect(IDeviceAPIEncapsulator *device, const ShaderEffect *resource);
    virtual ~DeviceAPIDependantShaderEffect();

    virtual size_t VideoMemorySizeInBytes() const override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
