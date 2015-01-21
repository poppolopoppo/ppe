#ifndef _LIB_PLATFORM_DIRECTX11_APPIN_FX_INCLUDED
#define _LIB_PLATFORM_DIRECTX11_APPIN_FX_INCLUDED

// called by generated accessors
#define AppIn_Get_Identity(v) v

#define AppIn_Get_PositionX_Float2(v) AppIn_Get_Identity(v)
#define AppIn_Get_PositionX_Float3(v) AppIn_Get_Identity(v)
#define AppIn_Get_PositionX_Float4(v) AppIn_Get_Identity(v)
#define AppIn_Get_PositionX_Half2(v) AppIn_Get_Identity(v)
#define AppIn_Get_PositionX_Half4(v) AppIn_Get_Identity(v)

#define AppIn_Get_TexCoordX_Float2(v) AppIn_Get_Identity(v)
#define AppIn_Get_TexCoordX_Short2(v) AppIn_Get_Identity(v)
#define AppIn_Get_TexCoordX_Half2(v) AppIn_Get_Identity(v)

#define AppIn_Get_ColorX_Float4(v) AppIn_Get_Identity(v)
#define AppIn_Get_ColorX_UByte4N(v) AppIn_Get_Identity(v)

#define AppIn_Get_NormalX_Float3(v) AppIn_Get_Identity(v)
#define AppIn_Get_NormalX_UByte4N(v) ((v).xyz * 2 - 1)
#define AppIn_Get_NormalX_UX10Y10Z10W2N(v) ((v).xyz * 2 - 1)

#define AppIn_Get_TangentX_Float3(v) AppIn_Get_Identity(v)
#define AppIn_Get_TangentX_UByte4N(v) ((v).xyz * 2 - 1)
#define AppIn_Get_TangentX_UX10Y10Z10W2N(v) ((v).xyz * 2 - 1)

#define AppIn_Get_BinormalX_Float3(v) AppIn_Get_Identity(v)
#define AppIn_Get_BinormalX_UByte4N(v) ((v).xyz * 2 - 1)
#define AppIn_Get_BinormalX_UX10Y10Z10W2N(v) ((v).xyz * 2 - 1)

#define AppIn_Get_BinormalX_PackedInTangentW(normal_, tangent_, winding_) \
    (cross((normal_), (tangent_)) * ((winding_) != 0 ? 1 : -1))

#endif //!_LIB_PLATFORM_DIRECTX11_APPIN_FX_INCLUDED
