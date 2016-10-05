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
FDeviceEncapsulatorException::FDeviceEncapsulatorException(
    const char *what,
    const IDeviceAPIEncapsulator *encapsulator,
    const FDeviceResource *optionalResource/* = nullptr */)
:   FGraphicsException(what)
,   _encapsulator(encapsulator)
,   _optionalResource(optionalResource)
{}
//----------------------------------------------------------------------------
FDeviceEncapsulatorException::~FDeviceEncapsulatorException() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FShaderCompilerException::FShaderCompilerException(
    const char *what,
    const IDeviceAPIShaderCompiler *encapsulator,
    const Graphics::FShaderSource *shaderSource)
:   FGraphicsException(what)
,   _encapsulator(encapsulator)
,   _shaderSource(shaderSource)
{}
//----------------------------------------------------------------------------
FShaderCompilerException::~FShaderCompilerException() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
