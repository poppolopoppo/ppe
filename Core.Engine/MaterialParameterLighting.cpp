#include "stdafx.h"

#include "MaterialParameterLighting.h"

#include "LightingEnvironment.h"
#include "MaterialContext.h"
#include "MaterialDatabase.h"
#include "Scene.h"
#include "World.h"

#include "Core.Graphics/BindName.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool MaterialParameterLighting_SunColor::Memoize_ReturnIfChanged_(float3 *cached, const MaterialContext& context) {
    const float3& value = context.Scene->World()->Lighting()->SunColor();

    const bool changed = (value != *cached);
    *cached = value;

    return changed;
}
//----------------------------------------------------------------------------
bool MaterialParameterLighting_SunDirection::Memoize_ReturnIfChanged_(float3 *cached, const MaterialContext& context) {
    const float3& value = context.Scene->World()->Lighting()->SunDirection();

    const bool changed = (value != *cached);
    *cached = value;

    return changed;
}
//----------------------------------------------------------------------------
bool MaterialParameterLighting_Exposure::Memoize_ReturnIfChanged_(float *cached, const MaterialContext& context) {
    const float value = context.Scene->World()->Lighting()->Exposure();

    const bool changed = (value != *cached);
    *cached = value;

    return changed;
}
//----------------------------------------------------------------------------
bool MaterialParameterLighting_WhitePoint::Memoize_ReturnIfChanged_(float *cached, const MaterialContext& context) {
    const float value = context.Scene->World()->Lighting()->WhitePoint();

    const bool changed = (value != *cached);
    *cached = value;

    return changed;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RegisterLightingMaterialParameters(MaterialDatabase *database) {
    Assert(database);

    database->BindParameter("uniSunColor",      new MaterialParameterLighting_SunColor());
    database->BindParameter("uniSunDirection",  new MaterialParameterLighting_SunDirection());
    database->BindParameter("uniExposure",      new MaterialParameterLighting_Exposure());
    database->BindParameter("uniWhitePoint",    new MaterialParameterLighting_WhitePoint());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
