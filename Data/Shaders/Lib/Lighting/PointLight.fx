#ifndef _LIB_LIGHTING_POINTLIGHT_FX_INCLUDED
#define _LIB_LIGHTING_POINTLIGHT_FX_INCLUDED

#include "Lib/Lighting/BRDF.fx"
#include "Lib/Lighting/Environment.fx"
#include "Lib/Lighting/Geometry.fx"
#include "Lib/Lighting/Material.fx"

namespace Lighting {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PointLight {
    float3  Color;
    float   Cutoff;
    float   Intensity;
    float3  Position;
    float   Radius;
};
//----------------------------------------------------------------------------
float3 Shade(   Geometry g, Material m, Environment e,
                PointLight l ) {

    float3 direction = l.Position - g.Position;
    float distanceToLight = length(direction);
    direction /= distanceToLight;

    float d = max(distanceToLight - l.Radius, 0);

    float denom = d/l.Radius + 1;
    float attenuation = 1 / (denom * denom);

    attenuation = (attenuation - l.Cutoff) / (1.0001 - l.Cutoff);
    attenuation = max(attenuation, 0);

    return Shade(   g, m, e,
                    direction, l.Color, attenuation, l.Intensity );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lighting

#endif //!_LIB_LIGHTING_POINTLIGHT_FX_INCLUDED
