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
:   std::exception(what)
,   _encapsulator(encapsulator)
,   _optionalResource(optionalResource)
{}
//----------------------------------------------------------------------------
DeviceEncapsulatorException::~DeviceEncapsulatorException() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ShaderCompilerEncapsulatorException::ShaderCompilerEncapsulatorException(
    const char *what,
    const IDeviceAPIShaderCompiler *encapsulator,
    const Graphics::ShaderProgram *shaderProgram,
    const Graphics::ShaderSource *shaderSource)
:   std::exception(what)
,   _encapsulator(encapsulator)
,   _shaderProgram(shaderProgram)
,   _shaderSource(shaderSource)
{}
//----------------------------------------------------------------------------
ShaderCompilerEncapsulatorException::~ShaderCompilerEncapsulatorException() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
