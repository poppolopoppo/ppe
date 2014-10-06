#ifndef _LIB_PLATFORM_DIRECTX11_TEXTURE_FX_INCLUDED
#define _LIB_PLATFORM_DIRECTX11_TEXTURE_FX_INCLUDED

#define TEXTURE2D(_Name) \
    Texture2D _Name; \
    SamplerState Sampler_##_Name

#define TEXTURE2DPARAM_DECL(_Name) \
    Texture2D _Name, SamplerState Sampler_##_Name

#define TEXTURE2DPARAM_CALL(_Name) \
    _Name, Sampler_##_Name

#define TEX2D(_Name, _UV) \
    _Name.Sample(Sampler_##_Name, _UV)

#define TEX2DLOD(_Name, _UV, _LOD) \
    _Name.SampleLevel(Sampler_##_Name, _UV, _LOD)

#define TEX2DGRAD(_Name, _UV, _DDX, _DDY) \
    _Name.SampleGrad(Sampler_##_Name, _UV, _DDX, _DDY)

#endif //!_LIB_PLATFORM_DIRECTX11_TEXTURE_FX_INCLUDED
