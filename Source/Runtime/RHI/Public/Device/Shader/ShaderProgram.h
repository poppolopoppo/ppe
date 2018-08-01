#pragma once

#include "Graphics.h"

#include "Name.h"
#include "Device/DeviceAPIDependantEntity.h"
#include "Device/DeviceResourceBuffer.h"
#include "Device/Pool/DeviceResourceSharable.h"

#include "Device/Shader/ShaderProfile.h"

namespace PPE {
namespace Graphics {
FWD_REFPTR(DeviceAPIDependantShaderProgram);
FWD_REFPTR(ShaderCompiled);
FWD_REFPTR(VertexDeclaration);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(ShaderProgram);
class FShaderProgram : public FDeviceResourceSharable {
public:
    FShaderProgram(  const Graphics::FVertexDeclaration* vertexDeclaration,
                    EShaderProgramType programType,
                    EShaderProfileType profileType,
                    const FShaderCompiled* compiled,
                    bool sharable );
    virtual ~FShaderProgram();

    virtual bool Available() const override;
    virtual FDeviceAPIDependantEntity *TerminalEntity() const override;

    const PDeviceAPIDependantShaderProgram& DeviceAPIDependantProgram() const {
        Assert(Frozen()); return _deviceAPIDependantProgram;
    }

    const Graphics::FVertexDeclaration* VertexDeclaration() const { return _vertexDeclaration; }

    EShaderProgramType ProgramType() const { return static_cast<EShaderProgramType>(bitprogram_type::Get(_data)); }
    EShaderProfileType ProfileType() const { return static_cast<EShaderProfileType>(bitprofile_type::Get(_data)); }

    const FShaderCompiled* Compiled() const { return _compiled; }

    virtual void Create(IDeviceAPIEncapsulator *device);
    virtual void Destroy(IDeviceAPIEncapsulator *device);

protected:
    virtual size_t VirtualSharedKeyHashValue() const override;
    virtual bool VirtualMatchTerminalEntity(const FDeviceAPIDependantEntity *entity) const override;

private:
    typedef Meta::TBit<size_t>::TFirst<2>::type bitprogram_type;
    typedef Meta::TBit<size_t>::TAfter<bitprogram_type>::FRemain::type bitprofile_type;

    size_t _data;
    SCVertexDeclaration _vertexDeclaration;
    PDeviceAPIDependantShaderProgram _deviceAPIDependantProgram;
    SCShaderCompiled _compiled;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDeviceAPIDependantShaderProgram : public TTypedDeviceAPIDependantEntity<FShaderProgram> {
public:
    FDeviceAPIDependantShaderProgram(IDeviceAPIEncapsulator *device, const FShaderProgram *resource);
    virtual ~FDeviceAPIDependantShaderProgram();

    const Graphics::FVertexDeclaration* VertexDeclaration() const { return _vertexDeclaration; }

    EShaderProgramType ProgramType() const { return static_cast<EShaderProgramType>(bitprogram_type::Get(_data)); }
    EShaderProfileType ProfileType() const { return static_cast<EShaderProfileType>(bitprofile_type::Get(_data)); }

    const FShaderCompiled* Compiled() const { return _compiled; }

private:
    typedef Meta::TBit<size_t>::TFirst<2>::type bitprogram_type;
    typedef Meta::TBit<size_t>::TAfter<bitprogram_type>::FRemain::type bitprofile_type;

    size_t _data;
    SCShaderCompiled _compiled;
    SCVertexDeclaration _vertexDeclaration;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
