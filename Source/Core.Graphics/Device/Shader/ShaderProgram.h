#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceAPIDependantEntity.h"
#include "Core.Graphics/Device/DeviceResource.h"
#include "Core.Graphics/Device/DeviceResourceBuffer.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/Container/RawStorage.h"
#include "Core/Meta/BitField.h"

namespace Core {
namespace Graphics {
class BindName;
FWD_REFPTR(ConstantBufferLayout);
class IDeviceAPIShaderCompilerEncapsulator;
FWD_REFPTR(DeviceAPIDependantShaderProgram);
class ShaderSource;
class VertexDeclaration;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ShaderCompilerFlags {
    None        = 0,
    Debug       = 1 << 0,
    Optimize    = 1 << 1,
    NoOptimize  = 1 << 2,
    Pedantic    = 1 << 3,
    WError      = 1 << 4,

    Default = Optimize|Pedantic|WError,
    DefaultForDebug = Debug|NoOptimize|Pedantic|WError,
};
//----------------------------------------------------------------------------
void ShaderCompilerFlagsToCStr(char *cstr, size_t capacity, ShaderCompilerFlags flags);
//----------------------------------------------------------------------------
enum class ShaderProfileType {
    ShaderModel5 = 0,
    ShaderModel4_1,
    ShaderModel4,
    ShaderModel3,
};
//----------------------------------------------------------------------------
const char *ShaderProfileTypeToCStr(ShaderProfileType profile);
//----------------------------------------------------------------------------
enum class ShaderProgramType {
    Vertex = 0,
    Hull,
    Domain,
    Pixel,
    Geometry,
    Compute,

    __Count,
};
//----------------------------------------------------------------------------
MemoryView<const ShaderProgramType> EachShaderProgramType();
const char *ShaderProgramTypeToCStr(ShaderProgramType program);
const char *ShaderProgramTypeToEntryPoint(ShaderProgramType program);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(ShaderProgram);
class ShaderProgram : public TypedDeviceResource<DeviceResourceType::ShaderProgram> {
public:
    ShaderProgram(ShaderProfileType profile, ShaderProgramType type);
    virtual ~ShaderProgram();

    virtual bool Available() const override { return nullptr != _deviceAPIDependantProgram; }
    const PDeviceAPIDependantShaderProgram& DeviceAPIDependantProgram() const {
        Assert(Frozen()); return _deviceAPIDependantProgram;
    }

    ShaderProfileType ProfileType() const { return static_cast<ShaderProfileType>(bitprofile_type::Get(_data)); }
    ShaderProgramType ProgramType() const { return static_cast<ShaderProgramType>(bitprogram_type::Get(_data)); }

    virtual void Create(    IDeviceAPIShaderCompilerEncapsulator *compiler,
                            const char *entryPoint,
                            ShaderCompilerFlags flags,
                            const ShaderSource *source,
                            const VertexDeclaration *vertexDeclaration);

    virtual void Destroy(   IDeviceAPIShaderCompilerEncapsulator *compiler);

    void Preprocess(IDeviceAPIShaderCompilerEncapsulator *compiler,
                    RAWSTORAGE(Shader, char)& output,
                    const ShaderSource *source,
                    const VertexDeclaration *vertexDeclaration) const;

    void Reflect(   IDeviceAPIShaderCompilerEncapsulator *compiler,
                    ASSOCIATIVE_VECTOR(Shader, BindName, PCConstantBufferLayout)& constants,
                    VECTOR(Shader, BindName)& textures ) const;

private:
    typedef Meta::Bit<size_t>::First<2>::type bitprofile_type;
    typedef Meta::Bit<size_t>::After<bitprofile_type>::Remain::type bitprogram_type;

    size_t _data;
    PDeviceAPIDependantShaderProgram _deviceAPIDependantProgram;
};
//----------------------------------------------------------------------------
class ShaderProgramMetaData : public RefCountable {
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DeviceAPIDependantShaderProgram : public DeviceAPIDependantEntity {
public:
    DeviceAPIDependantShaderProgram(
        IDeviceAPIShaderCompilerEncapsulator *device,
        Graphics::ShaderProgram *owner,
        const char *sourceName,
        ShaderCompilerFlags flags,
        const ShaderSource *source,
        const VertexDeclaration *vertexDeclaration);
    virtual ~DeviceAPIDependantShaderProgram();

    const ShaderProgram *Owner() const { return _owner; }

private:
    ShaderProgram *_owner;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void CompileShaderProgram(
    IDeviceAPIShaderCompilerEncapsulator *compiler,
    ShaderProgram *program,
    const char *entryPoint,
    ShaderCompilerFlags flags,
    const Filename& filename,
    const VertexDeclaration *vertexDeclaration,
    const MemoryView<const Pair<String, String>>& defines );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
