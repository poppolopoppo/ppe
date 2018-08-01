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
class FShaderEffect : public FDeviceResource {
public:
    explicit FShaderEffect(const Graphics::FVertexDeclaration *vertexDeclaration);
    virtual ~FShaderEffect();

    virtual bool Available() const override;
    virtual FDeviceAPIDependantEntity *TerminalEntity() const override;

    const PDeviceAPIDependantShaderEffect& DeviceAPIDependantEffect() const {
        Assert(Frozen()); return _deviceAPIDependantEffect;
    }

    const PCVertexDeclaration& VertexDeclaration() const { return _vertexDeclaration; }

    FShaderProgram *StageProgram(EShaderProgramType stage) { return _stagePrograms[size_t(stage)].get(); }
    const FShaderProgram *StageProgram(EShaderProgramType stage) const { return _stagePrograms[size_t(stage)].get(); }

    void SetStageProgram(EShaderProgramType stage, PShaderProgram&& program);
    void ResetStageProgram(EShaderProgramType stage);

    virtual void Create(IDeviceAPIEncapsulator *device);
    virtual void Destroy(IDeviceAPIEncapsulator *device);

private:
    PCVertexDeclaration _vertexDeclaration;
    PShaderProgram _stagePrograms[size_t(EShaderProgramType::__Count)];

    PDeviceAPIDependantShaderEffect _deviceAPIDependantEffect;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDeviceAPIDependantShaderEffect : public TTypedDeviceAPIDependantEntity<FShaderEffect> {
public:
    FDeviceAPIDependantShaderEffect(IDeviceAPIEncapsulator *device, const FShaderEffect *resource);
    virtual ~FDeviceAPIDependantShaderEffect();

    virtual size_t VideoMemorySizeInBytes() const override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
