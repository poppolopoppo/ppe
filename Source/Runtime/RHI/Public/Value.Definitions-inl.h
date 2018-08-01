
// Here goes the list of all types supported by Graphics::FValue

#define FOREACH_PPE_GRAPHIC_VALUETYPE(_Macro, ...)                           \
    COMMA_PROTECT(_Macro(Float          , float                 ,  1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Float2         , Core::float2          ,  2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Float3         , Core::float3          ,  3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Float4         , Core::float4          ,  4, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Float3x3       , Core::float3x3        ,  5, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Float4x3       , Core::float4x3        ,  6, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Float4x4       , Core::float4x4        ,  7, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Bool           , bool                  ,  8, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Byte           , Core::byte            ,  9, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Byte2          , Core::byte2           , 10, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Byte4          , Core::byte4           , 11, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UByte          , Core::ubyte           , 12, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UByte2         , Core::ubyte2          , 13, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UByte4         , Core::ubyte4          , 14, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Short          , short                 , 15, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Short2         , Core::short2          , 16, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Short4         , Core::short4          , 17, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UShort         , Core::ushort          , 18, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UShort2        , Core::ushort2         , 19, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UShort4        , Core::ushort4         , 20, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Word           , Core::word            , 21, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Word2          , Core::word2           , 22, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Word3          , Core::word3           , 23, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Word4          , Core::word4           , 24, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UWord          , Core::uword           , 25, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UWord2         , Core::uword2          , 26, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UWord3         , Core::uword3          , 27, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UWord4         , Core::uword4          , 28, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Half           , Core::half            , 29, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Half2          , Core::half2           , 30, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Half4          , Core::half4           , 31, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Byte2N         , Core::byte2n          , 32, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Byte4N         , Core::byte4n          , 33, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UByte2N        , Core::ubyte2n         , 34, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UByte4N        , Core::ubyte4n         , 35, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Short2N        , Core::short2n         , 36, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(Short4N        , Core::short4n         , 37, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UShort2N       , Core::ushort2n        , 38, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UShort4N       , Core::ushort4n        , 39, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(UX10Y10Z10W2N  , Core::UX10Y10Z10W2N   , 40, ##__VA_ARGS__))
