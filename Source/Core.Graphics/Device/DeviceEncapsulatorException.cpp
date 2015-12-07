#include "stdafx.h"

#include "DeviceEncapsulatorException.h"

#include "DeviceAPI.h"
#include "DeviceResource.h"
#include "Shader/ShaderProgram.h"
#include "Shader/ShaderSource.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceEncapsulatorException::DeviceEncapsulatorException(
    const char *what,
    const IDeviceAPIEncapsulator *encapsulator,
    const DeviceResource *optionalResource/* = nullptr */)
:   GraphicsException(what)
,   _encapsulator(encapsulator)
,   _optionalResource(optionalResource)
{}
//----------------------------------------------------------------------------
DeviceEncapsulatorException::~DeviceEncapsulatorException() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ShaderCompilerException::ShaderCompilerException(
    const char *what,
    const IDeviceAPIShaderCompiler *encapsulator,
    const Graphics::ShaderSource *shaderSource)
:   GraphicsException(what)
,   _encapsulator(encapsulator)
,   _shaderSource(shaderSource)
{}
//----------------------------------------------------------------------------
ShaderCompilerException::~ShaderCompilerException() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
