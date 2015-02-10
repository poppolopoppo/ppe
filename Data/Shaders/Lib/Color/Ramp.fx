#ifndef _LIB_COLOR_RAMP_FX_INCLUDED
#define _LIB_COLOR_RAMP_FX_INCLUDED

namespace Ramp {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float3 Rainbow(float x) {
    static const float3 gGradient[8] = {
        float3(1,0,0),      //Red
        float3(1,0.5f,0),   //Orange
        float3(1,1,0),      //Yellow
        float3(0,1,0),      //Green
        float3(0,0,1),      //Blue
        float3(75.0/255,0,130.0/255), //Indigo
        float3(143.0/255, 0, 1), //Violet
        float3(1,0,0)       //Red again
    };
    static const float gStepPerColor = 1.0/8;

    int color0 = (int)floor(x/gStepPerColor);
    int color1 = color0+1;
    if (color1 >= 8)
        return gGradient[7];

    float t = (x % gStepPerColor)/gStepPerColor;
    return lerp(gGradient[color0], gGradient[color1], t);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Ramp

#endif //!_LIB_COLOR_RAMP_FX_INCLUDED
