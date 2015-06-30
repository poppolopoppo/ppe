#pragma once

#include "Core.Graphics/Graphics.h"

#include <exception>

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
class IDeviceAPIShaderCompiler;
FWD_REFPTR(DeviceResource);
FWD_REFPTR(ShaderSource);
FWD_REFPTR(ShaderProgram);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DeviceEncapsulatorException : public std::exception {
public:
    explicit DeviceEncapsulatorException(
        const char *what,
        const IDeviceAPIEncapsulator *encapsulator,
        const DeviceResource *optionalResource = nullptr);
    ~DeviceEncapsulatorException();

    const IDeviceAPIEncapsulator *Encapsulator() const { return _encapsulator; }
    const DeviceResource *OptionalResource() const { return _optionalResource; }

private:
    const IDeviceAPIEncapsulator *_encapsulator;
    PCDeviceResource _optionalResource;
};
//----------------------------------------------------------------------------
class ShaderCompilerEncapsulatorException : public std::exception {
public:
    explicit ShaderCompilerEncapsulatorException(
        const char *what,
        const IDeviceAPIShaderCompiler *encapsulator,
        const Graphics::ShaderProgram *shaderProgram,
        const Graphics::ShaderSource *shaderSource);
    ~ShaderCompilerEncapsulatorException();

    const IDeviceAPIShaderCompiler *Encapsulator() const { return _encapsulator; }
    const Graphics::ShaderProgram *ShaderProgram() const { return _shaderProgram; }
    const Graphics::ShaderSource *ShaderSource() const { return _shaderSource; }

private:
    const IDeviceAPIShaderCompiler *_encapsulator;
    PCShaderProgram _shaderProgram;
    PCShaderSource _shaderSource;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
