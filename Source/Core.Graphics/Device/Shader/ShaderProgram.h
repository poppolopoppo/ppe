#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/BindName.h"
#include "Core.Graphics/Device/DeviceAPIDependantEntity.h"
#include "Core.Graphics/Device/DeviceResourceBuffer.h"
#include "Core.Graphics/Device/Pool/DeviceResourceSharable.h"

#include "Core.Graphics/Device/Shader/ShaderProfile.h"

namespace Core {
namespace Graphics {
FWD_REFPTR(DeviceAPIDependantShaderProgram);
FWD_REFPTR(ShaderCompiled);
FWD_REFPTR(VertexDeclaration);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(ShaderProgram);
class ShaderProgram : public DeviceResourceSharable {
public:
    ShaderProgram(  const Graphics::VertexDeclaration* vertexDeclaration,
                    ShaderProgramType programType,
                    ShaderProfileType profileType,
                    const ShaderCompiled* compiled,
                    bool sharable );
    virtual ~ShaderProgram();

    virtual bool Available() const override;
    virtual DeviceAPIDependantEntity *TerminalEntity() const override;

    const PDeviceAPIDependantShaderProgram& DeviceAPIDependantProgram() const {
        Assert(Frozen()); return _deviceAPIDependantProgram;
    }

    const Graphics::VertexDeclaration* VertexDeclaration() const { return _vertexDeclaration; }

    ShaderProgramType ProgramType() const { return static_cast<ShaderProgramType>(bitprogram_type::Get(_data)); }
    ShaderProfileType ProfileType() const { return static_cast<ShaderProfileType>(bitprofile_type::Get(_data)); }

    const ShaderCompiled* Compiled() const { return _compiled; }

    virtual void Create(IDeviceAPIEncapsulator *device);
    virtual void Destroy(IDeviceAPIEncapsulator *device);

protected:
    virtual size_t VirtualSharedKeyHashValue() const override;
    virtual bool VirtualMatchTerminalEntity(const DeviceAPIDependantEntity *entity) const override;

private:
    typedef Meta::Bit<size_t>::First<2>::type bitprogram_type;
    typedef Meta::Bit<size_t>::After<bitprogram_type>::Remain::type bitprofile_type;

    size_t _data;
    SCVertexDeclaration _vertexDeclaration;
    PDeviceAPIDependantShaderProgram _deviceAPIDependantProgram;
    SCShaderCompiled _compiled;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DeviceAPIDependantShaderProgram : public TypedDeviceAPIDependantEntity<ShaderProgram> {
public:
    DeviceAPIDependantShaderProgram(IDeviceAPIEncapsulator *device, const ShaderProgram *resource);
    virtual ~DeviceAPIDependantShaderProgram();

    const Graphics::VertexDeclaration* VertexDeclaration() const { return _vertexDeclaration; }

    ShaderProgramType ProgramType() const { return static_cast<ShaderProgramType>(bitprogram_type::Get(_data)); }
    ShaderProfileType ProfileType() const { return static_cast<ShaderProfileType>(bitprofile_type::Get(_data)); }

    const ShaderCompiled* Compiled() const { return _compiled; }

private:
    typedef Meta::Bit<size_t>::First<2>::type bitprogram_type;
    typedef Meta::Bit<size_t>::After<bitprogram_type>::Remain::type bitprofile_type;

    size_t _data;
    SCShaderCompiled _compiled;
    SCVertexDeclaration _vertexDeclaration;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
