
// Here goes the list of all BASIC types supported by RTTI
// Use TMetaTypeTraits.h to project your custom type to one of these

#define FOREACH_CORE_RTTI_NATIVE_TYPES(_Macro, ...) \
    COMMA_PROTECT(_Macro(bool,          bool,                           1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(i8,            i8,                             2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(i16,           i16,                            3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(i32,           i32,                            4, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(i64,           i64,                            5, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(u8,            u8,                             6, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(u16,           u16,                            7, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(u32,           u32,                            8, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(u64,           u64,                            9, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(float,         float,                         10, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(double,        double,                        11, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(byte2,         Core::byte2,                   12, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(byte4,         Core::byte4,                   13, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(ubyte2,        Core::ubyte2,                  14, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(ubyte4,        Core::ubyte4,                  15, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(short2,        Core::short2,                  16, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(short4,        Core::short4,                  17, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(ushort2,       Core::ushort2,                 18, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(ushort4,       Core::ushort4,                 19, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(word2,         Core::word2,                   20, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(word3,         Core::word3,                   21, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(word4,         Core::word4,                   22, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(uword2,        Core::uword2,                  23, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(uword3,        Core::uword3,                  24, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(uword4,        Core::uword4,                  25, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(float2,        Core::float2,                  26, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(float3,        Core::float3,                  27, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(float4,        Core::float4,                  28, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(float2x2,      Core::float2x2,                29, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(float3x3,      Core::float3x3,                30, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(float4x3,      Core::float4x3,                31, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(float4x4,      Core::float4x4,                32, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(FString,       Core::FString,                 33, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(FWString,      Core::FWString,                34, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(FMetaAtom,     Core::RTTI::PMetaAtom,         35, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(FMetaObject,   Core::RTTI::PMetaObject,       36, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(FName,         Core::RTTI::FName,             37, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(FBinaryData,   Core::RTTI::FBinaryData,       38, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(FOpaqueData,   Core::RTTI::FOpaqueData,       39, ##__VA_ARGS__))
