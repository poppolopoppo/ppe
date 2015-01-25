#ifndef _LIB_PLATFORM_DIRECTX11_TEXTURE_FX_INCLUDED
#define _LIB_PLATFORM_DIRECTX11_TEXTURE_FX_INCLUDED

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TEXTURE2D
//----------------------------------------------------------------------------
#define TEXTURE2D(_Name) \
    Texture2D _Name; \
    SamplerState Sampler_##_Name
//----------------------------------------------------------------------------
#define TEXTURE2DPARAM_DECL(_Name) \
    Texture2D _Name, SamplerState Sampler_##_Name
//----------------------------------------------------------------------------
#define TEXTURE2DPARAM_CALL(_Name) \
    _Name, Sampler_##_Name
//----------------------------------------------------------------------------
#define TEXTURE2DSTRUCT_DECL(_Name) \
    Texture2D _Name; SamplerState Sampler_##_Name
//----------------------------------------------------------------------------
#define TEXTURE2DSTRUCT_CALL(_Struct, _Name) \
    (_Struct)._Name, (_Struct).Sampler_##_Name
//----------------------------------------------------------------------------
#define TEXTURE2DSTRUCT_ASSIGN(_Struct, _Name, _Value) \
    (_Struct)._Name = _Value; (_Struct).Sampler_##_Name = Sampler_##_Value
//----------------------------------------------------------------------------
#define TEX2D(_Name, _UV) \
    _Name.Sample(Sampler_##_Name, _UV)
//----------------------------------------------------------------------------
#define TEX2DLOD(_Name, _UV, _LOD) \
    _Name.SampleLevel(Sampler_##_Name, _UV, _LOD)
//----------------------------------------------------------------------------
#define TEX2DGRAD(_Name, _UV, _DDX, _DDY) \
    _Name.SampleGrad(Sampler_##_Name, _UV, _DDX, _DDY)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TEXTURECUBE
//----------------------------------------------------------------------------
#define TEXTURECUBE(_Name) \
    TextureCube _Name; \
    SamplerState Sampler_##_Name
//----------------------------------------------------------------------------
#define TEXTURECUBEPARAM_DECL(_Name) \
    TextureCube _Name, SamplerState Sampler_##_Name
//----------------------------------------------------------------------------
#define TEXTURECUBEPARAM_CALL(_Name) \
    _Name, Sampler_##_Name
//----------------------------------------------------------------------------
#define TEXTURECUBESTRUCT_DECL(_Name) \
    TextureCube _Name; SamplerState Sampler_##_Name
//----------------------------------------------------------------------------
#define TEXTURECUBESTRUCT_CALL(_Struct, _Name) \
    (_Struct)._Name, (_Struct).Sampler_##_Name
//----------------------------------------------------------------------------
#define TEXTURECUBESTRUCT_ASSIGN(_Struct, _Name, _Value) \
    (_Struct)._Name = _Value; (_Struct).Sampler_##_Name = Sampler_##_Value
//----------------------------------------------------------------------------
#define TEXCUBE(_Name, _UVW) \
    _Name.Sample(Sampler_##_Name, _UVW)
//----------------------------------------------------------------------------
#define TEXCUBELOD(_Name, _UVW, _LOD) \
    _Name.SampleLevel(Sampler_##_Name, _UVW, _LOD)
//----------------------------------------------------------------------------
#define TEXCUBEGRAD(_Name, _UVW, _DDX, _DDY, _DDZ) \
    _Name.SampleGrad(Sampler_##_Name, _UVW, _DDX, _DDY, _DDZ)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------

#endif //!_LIB_PLATFORM_DIRECTX11_TEXTURE_FX_INCLUDED
