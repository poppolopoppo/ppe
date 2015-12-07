#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core/Diagnostic/Exception.h"

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
class GraphicsException : public Exception {
public:
    GraphicsException(const char* what) : Exception(what) {}
};
//----------------------------------------------------------------------------
class DeviceEncapsulatorException : public GraphicsException {
public:
    explicit DeviceEncapsulatorException(
        const char *what,
        const IDeviceAPIEncapsulator *encapsulator,
        const DeviceResource *optionalResource = nullptr);
    ~DeviceEncapsulatorException();

    const IDeviceAPIEncapsulator *Encapsulator() const { return _encapsulator; }
    const PCDeviceResource& OptionalResource() const { return _optionalResource; }

private:
    const IDeviceAPIEncapsulator *_encapsulator;
    PCDeviceResource _optionalResource;
};
//----------------------------------------------------------------------------
class ShaderCompilerException : public GraphicsException {
public:
    explicit ShaderCompilerException(
        const char *what,
        const IDeviceAPIShaderCompiler *encapsulator,
        const Graphics::ShaderSource *shaderSource);
    ~ShaderCompilerException();

    const IDeviceAPIShaderCompiler *Encapsulator() const { return _encapsulator; }
    const PCShaderSource& ShaderSource() const { return _shaderSource; }

private:
    const IDeviceAPIShaderCompiler *_encapsulator;
    PCShaderSource _shaderSource;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
