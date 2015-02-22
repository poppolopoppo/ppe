#include "stdafx.h"

#include "MaterialParameterLighting.h"

#include "Lighting/LightingEnvironment.h"
#include "Material/MaterialContext.h"
#include "Material/MaterialDatabase.h"
#include "Scene/Scene.h"
#include "World/World.h"

#include "Core.Graphics/Device/BindName.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool MaterialParameterLighting_SunColor::Memoize_ReturnIfChanged_(float3 *cached, const MaterialContext& context) {
    const ColorRGBA& color = context.Scene->World()->Lighting()->Sun().Color();
    const float3 value = ColorRGBAF(color).Data().xyz();

    const bool changed = (value != *cached);
    *cached = value;

    return changed;
}
//----------------------------------------------------------------------------
bool MaterialParameterLighting_SunDirection::Memoize_ReturnIfChanged_(float3 *cached, const MaterialContext& context) {
    const float3& value = context.Scene->World()->Lighting()->Sun().Direction();

    const bool changed = (value != *cached);
    *cached = value;

    return changed;
}
//----------------------------------------------------------------------------
bool MaterialParameterLighting_SunIntensity::Memoize_ReturnIfChanged_(float *cached, const MaterialContext& context) {
    const float value = context.Scene->World()->Lighting()->Sun().Intensity();

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
    database->BindParameter("uniSunIntensity",  new MaterialParameterLighting_SunIntensity());
    database->BindParameter("uniExposure",      new MaterialParameterLighting_Exposure());
    database->BindParameter("uniWhitePoint",    new MaterialParameterLighting_WhitePoint());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
