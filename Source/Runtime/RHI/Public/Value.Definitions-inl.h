
// Here goes the list of all types supported by Graphics::FValue

#define FOREACH_PPE_GRAPHIC_VALUETYPE(_Macro, ...)                           \
    COMMA_PROTECT(_Macro(Float          , float                 ,  1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Float2         , PPE::float2          ,  2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Float3         , PPE::float3          ,  3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Float4         , PPE::float4          ,  4, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Float3x3       , PPE::float3x3        ,  5, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Float4x3       , PPE::float4x3        ,  6, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Float4x4       , PPE::float4x4        ,  7, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Bool           , bool                  ,  8, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Byte           , PPE::byte            ,  9, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Byte2          , PPE::byte2           , 10, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Byte4          , PPE::byte4           , 11, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UByte          , PPE::ubyte           , 12, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UByte2         , PPE::ubyte2          , 13, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UByte4         , PPE::ubyte4          , 14, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Short          , short                 , 15, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Short2         , PPE::short2          , 16, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Short4         , PPE::short4          , 17, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UShort         , PPE::ushort          , 18, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UShort2        , PPE::ushort2         , 19, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UShort4        , PPE::ushort4         , 20, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Word           , PPE::word            , 21, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Word2          , PPE::word2           , 22, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Word3          , PPE::word3           , 23, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Word4          , PPE::word4           , 24, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UWord          , PPE::uword           , 25, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UWord2         , PPE::uword2          , 26, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UWord3         , PPE::uword3          , 27, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UWord4         , PPE::uword4          , 28, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Half           , PPE::half            , 29, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Half2          , PPE::half2           , 30, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Half4          , PPE::half4           , 31, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Byte2N         , PPE::byte2n          , 32, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Byte4N         , PPE::byte4n          , 33, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UByte2N        , PPE::ubyte2n         , 34, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UByte4N        , PPE::ubyte4n         , 35, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Short2N        , PPE::short2n         , 36, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Short4N        , PPE::short4n         , 37, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UShort2N       , PPE::ushort2n        , 38, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UShort4N       , PPE::ushort4n        , 39, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UX10Y10Z10W2N  , PPE::UX10Y10Z10W2N   , 40, ##__VA_ARGS__))
