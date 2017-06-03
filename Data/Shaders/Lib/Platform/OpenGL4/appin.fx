#ifndef _LIB_PLATFORM_OPENGL4_APPIN_FX_INCLUDED
#define _LIB_PLATFORM_OPENGL4_APPIN_FX_INCLUDED

// called by generated accessors
#define AppIn_Get_Identity(v) v

#define AppIn_Get_PositionX_float2(v) AppIn_Get_Identity(v)
#define AppIn_Get_PositionX_float3(v) AppIn_Get_Identity(v)
#define AppIn_Get_PositionX_float4(v) AppIn_Get_Identity(v)
#define AppIn_Get_PositionX_half2(v) AppIn_Get_Identity(v)
#define AppIn_Get_PositionX_half4(v) AppIn_Get_Identity(v)

#define AppIn_Get_TexCoordX_float2(v) AppIn_Get_Identity(v)
#define AppIn_Get_TexCoordX_short2(v) AppIn_Get_Identity(v)
#define AppIn_Get_TexCoordX_half2(v) AppIn_Get_Identity(v)

#define AppIn_Get_ColorX_float4(v) AppIn_Get_Identity(v)
#define AppIn_Get_ColorX_UByte4N(v) AppIn_Get_Identity(v)

#define AppIn_Get_NormalX_float3(v) AppIn_Get_Identity(v)
#define AppIn_Get_NormalX_UByte4N(v) ((v).xyz * 2 - 1)
#define AppIn_Get_NormalX_UX10Y10Z10W2N(v) ((v).xyz * 2 - 1)

#define AppIn_Get_TangentX_float3(v) AppIn_Get_Identity(v)
#define AppIn_Get_TangentX_UByte4N(v) ((v).xyz * 2 - 1)
#define AppIn_Get_TangentX_UX10Y10Z10W2N(v) ((v).xyz * 2 - 1)

#define AppIn_Get_BinormalX_float3(v) AppIn_Get_Identity(v)
#define AppIn_Get_BinormalX_UByte4N(v) ((v).xyz * 2 - 1)
#define AppIn_Get_BinormalX_UX10Y10Z10W2N(v) ((v).xyz * 2 - 1)

#define AppIn_Get_BinormalX_PackedInTangentW(normal_, tangent_, winding_) \
    (cross((normal_), (tangent_)) * ((winding_) != 0 ? -1 : 1))

#endif //!_LIB_PLATFORM_OPENGL4_APPIN_FX_INCLUDED
