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
class FGraphicsException : public FException {
public:
    FGraphicsException(const char* what) : FException(what) {}
};
//----------------------------------------------------------------------------
class FDeviceEncapsulatorException : public FGraphicsException {
public:
    explicit FDeviceEncapsulatorException(
        const char *what,
        const IDeviceAPIEncapsulator *encapsulator,
        const FDeviceResource *optionalResource = nullptr);
    ~FDeviceEncapsulatorException();

    const IDeviceAPIEncapsulator *Encapsulator() const { return _encapsulator; }
    const PCDeviceResource& OptionalResource() const { return _optionalResource; }

private:
    const IDeviceAPIEncapsulator *_encapsulator;
    PCDeviceResource _optionalResource;
};
//----------------------------------------------------------------------------
class FShaderCompilerException : public FGraphicsException {
public:
    explicit FShaderCompilerException(
        const char *what,
        const IDeviceAPIShaderCompiler *encapsulator,
        const Graphics::FShaderSource *shaderSource);
    ~FShaderCompilerException();

    const IDeviceAPIShaderCompiler *Encapsulator() const { return _encapsulator; }
    const PCShaderSource& FShaderSource() const { return _shaderSource; }

private:
    const IDeviceAPIShaderCompiler *_encapsulator;
    PCShaderSource _shaderSource;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
