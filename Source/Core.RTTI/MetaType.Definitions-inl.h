
// Here goes the list of all BASIC types supported by RTTI
// Use MetaTypeTraits.h to project your custom type to one of these

#define FOREACH_CORE_RTTI_NATIVE_TYPES(_Macro, ...) \
    COMMA_PROTECT(_Macro(bool,       bool,                           1, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(i8,         i8,                             2, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(i16,        i16,                            3, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(i32,        i32,                            4, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(i64,        i64,                            5, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(u8,         u8,                             6, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(u16,        u16,                            7, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(u32,        u32,                            8, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(u64,        u64,                            9, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(float,      float,                         10, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(double,     double,                        11, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(byte2,      byte2,                         12, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(byte4,      byte4,                         13, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(ubyte2,     ubyte2,                        14, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(ubyte4,     ubyte4,                        15, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(short2,     short2,                        16, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(short4,     short4,                        17, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(ushort2,    ushort2,                       18, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(ushort4,    ushort4,                       19, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(word2,      word2,                         20, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(word4,      word4,                         21, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(uword2,     uword2,                        22, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(uword4,     uword4,                        23, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(float2,     float2,                        24, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(float3,     float3,                        25, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(float4,     float4,                        26, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(float2x2,   float2x2,                      27, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(float3x3,   float3x3,                      28, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(float4x3,   float4x3,                      30, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(float4x4,   float4x4,                      31, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(String,     Core::String,                  32, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(WString,    Core::WString,                 33, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(MetaAtom,   Core::RTTI::PMetaAtom,         34, ##__VA_ARGS__)) \
    COMMA_PROTECT(_Macro(MetaObject, Core::RTTI::PMetaObject,       35, ##__VA_ARGS__))
