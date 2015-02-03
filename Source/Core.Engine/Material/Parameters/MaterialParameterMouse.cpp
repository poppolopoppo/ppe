#include "stdafx.h"

#include "MaterialParameterMouse.h"

#include "Material/MaterialContext.h"
#include "Material/MaterialDatabase.h"
#include "Scene/Scene.h"
#include "Service/MouseService.h"
#include "Service/Service_fwd.h"
#include "Service/IService.h"
#include "Service/IServiceProvider.h"
#include "World/World.h"

#include "Core.Graphics/Device/BindName.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool MaterialParameterMouse_Position::Memoize_ReturnIfChanged_(float4 *cached, const MaterialContext& context) {

    ENGINESERVICE_PROVIDE(IMouseService, mouseService, context.Scene->World()->ServiceProvider());

    float4 value(0);
    value.x() = float(mouseService->MouseInputHandler()->ClientX());
    value.y() = float(mouseService->MouseInputHandler()->ClientY());
    value.z() = mouseService->MouseInputHandler()->RelativeX();
    value.w() = mouseService->MouseInputHandler()->RelativeY();

    const bool changed = (value != *cached);
    *cached = value;

    return changed;
}
//----------------------------------------------------------------------------
bool MaterialParameterMouse_Buttons::Memoize_ReturnIfChanged_(float4 *cached, const MaterialContext& context) {
    
    ENGINESERVICE_PROVIDE(IMouseService, mouseService, context.Scene->World()->ServiceProvider());

    float4 value(0);
    value.x() = mouseService->MouseInputHandler()->IsButtonPressed(MouseButton::Button0) ? 1.0f : 0.0f;
    value.y() = mouseService->MouseInputHandler()->IsButtonPressed(MouseButton::Button1) ? 1.0f : 0.0f;
    value.z() = mouseService->MouseInputHandler()->IsButtonPressed(MouseButton::Button2) ? 1.0f : 0.0f;
    value.w() = mouseService->MouseInputHandler()->IsButtonPressed(MouseButton::Wheel)   ? 1.0f : 0.0f;

    const bool changed = (value != *cached);
    *cached = value;

    return changed;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RegisterMouseMaterialParameters(MaterialDatabase *database) {
    Assert(database);

    database->BindParameter("uniMousePosition",   new MaterialParameterMouse_Position());
    database->BindParameter("uniMouseButtons",    new MaterialParameterMouse_Buttons());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
