#ifndef _LIB_LIGHTING_DIRECTIONALLIGHT_FX_INCLUDED
#define _LIB_LIGHTING_DIRECTIONALLIGHT_FX_INCLUDED

#include "Lib/Lighting/BRDF.fx"
#include "Lib/Lighting/Environment.fx"
#include "Lib/Lighting/Geometry.fx"
#include "Lib/Lighting/Material.fx"

namespace Lighting {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct DirectionalLight {
    float3  Color;
    float3  Direction;
    float   Intensity;
};
//----------------------------------------------------------------------------
float3 Shade(   Geometry g, Material m, Environment e,
                DirectionalLight l ) {
    // default for directional light :
    float lightAttenuation = 1.0;

    return Shade(   g, m, e,
                    l.Direction, l.Color, lightAttenuation, l.Intensity );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lighting

#endif //!_LIB_LIGHTING_DIRECTIONALLIGHT_FX_INCLUDED
