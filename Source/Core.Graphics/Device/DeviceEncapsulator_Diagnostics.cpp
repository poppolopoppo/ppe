#include "stdafx.h"

#include "DeviceEncapsulator.h"

#include "AbstractDeviceAPIEncapsulator.h"

#include "Geometry/IndexBuffer.h"
#include "Geometry/PrimitiveType.h"
#include "Geometry/VertexBuffer.h"
#include "Geometry/VertexDeclaration.h"

#include "Shader/ConstantBuffer.h"
#include "Shader/ShaderEffect.h"
#include "Shader/ShaderProgram.h"

#include "State/BlendState.h"
#include "State/DepthStencilState.h"
#include "State/RasterizerState.h"
#include "State/SamplerState.h"

#include "Texture/DepthStencil.h"
#include "Texture/RenderTarget.h"
#include "Texture/Texture.h"
#include "Texture/Texture2D.h"
#include "Texture/TextureCube.h"

#include "Core/Diagnostic/Logger.h"

// IDeviceAPIDiagnostics

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Diagnostics
//----------------------------------------------------------------------------
#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
bool DeviceEncapsulator::IsProfilerAttached() const {
    THIS_THREADRESOURCE_CHECKACCESS();

    return _deviceAPIEncapsulator->Diagnostics()->IsProfilerAttached();
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::BeginEvent(const wchar_t *name) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(name);
    Assert(IsProfilerAttached());

    return _deviceAPIEncapsulator->Diagnostics()->BeginEvent(name);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::EndEvent() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(IsProfilerAttached());

    return _deviceAPIEncapsulator->Diagnostics()->EndEvent();
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetMarker(const wchar_t *name) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(name);
    Assert(IsProfilerAttached());

    return _deviceAPIEncapsulator->Diagnostics()->SetMarker(name);
}
//----------------------------------------------------------------------------
#endif //!WITH_CORE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
