#include "stdafx.h"

#include "MaterialParameterMouse.h"

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
namespace MaterialParameterMouse {
//----------------------------------------------------------------------------
EACH_MATERIALPARAMETER_MOUSE(MATERIALPARAMETER_FN_DEF)
//----------------------------------------------------------------------------
void RegisterMaterialParameters(FMaterialDatabase *database) {
    Assert(database);

#define BIND_MATERIALPARAMETER(_Variability, _Type, _Name) \
    database->BindParameter("uni" STRINGIZE(_Name), new MATERIALPARAMETER_FN(_Variability, _Type, _Name)() );

    EACH_MATERIALPARAMETER_MOUSE(BIND_MATERIALPARAMETER)

#undef BIND_MATERIALPARAMETER
}
//----------------------------------------------------------------------------
} //!MaterialParameterMouse
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace MaterialParameterMouse {
//----------------------------------------------------------------------------
void MousePosition(const FMaterialParameterContext& context, float4& dst) {
    ENGINESERVICE_PROVIDE(IMouseService, mouseService, context.Scene->World()->ServiceProvider());

    dst.x() = float(mouseService->MouseInputHandler()->ClientX());
    dst.y() = float(mouseService->MouseInputHandler()->ClientY());
    dst.z() = mouseService->MouseInputHandler()->RelativeX();
    dst.w() = mouseService->MouseInputHandler()->RelativeY();
}
//----------------------------------------------------------------------------
void MouseButtons(const FMaterialParameterContext& context, float4& dst) {
    ENGINESERVICE_PROVIDE(IMouseService, mouseService, context.Scene->World()->ServiceProvider());

    dst.x() = mouseService->MouseInputHandler()->IsButtonPressed(EMouseButton::Button0) ? 1.0f : 0.0f;
    dst.y() = mouseService->MouseInputHandler()->IsButtonPressed(EMouseButton::Button1) ? 1.0f : 0.0f;
    dst.z() = mouseService->MouseInputHandler()->IsButtonPressed(EMouseButton::Button2) ? 1.0f : 0.0f;
    dst.w() = mouseService->MouseInputHandler()->IsButtonPressed(EMouseButton::Wheel)   ? 1.0f : 0.0f;
}
//----------------------------------------------------------------------------
} //!MaterialParameterMouse
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
