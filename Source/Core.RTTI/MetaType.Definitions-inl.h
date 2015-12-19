#ifndef DEF_METATYPE_SCALAR
#   define DEF_METATYPE_SCALAR(_Name, T, _TypeId)
#endif

// Here goes the list of all BASIC types supported by RTTI
// Use MetaTypeTraits.h to project your custom type to one of these

#ifndef DEF_METATYPE_SCALAR_ARITH
#   define DEF_METATYPE_SCALAR_ARITH_UNDEF
#   define DEF_METATYPE_SCALAR_ARITH(_Name, T, _TypeId) DEF_METATYPE_SCALAR(_Name, T, _TypeId)
#endif
#ifndef DEF_METATYPE_SCALAR_VECTOR_ARITH
#   define DEF_METATYPE_SCALAR_VECTOR_ARITH_UNDEF
#   define DEF_METATYPE_SCALAR_VECTOR_ARITH(_Name, T, _TypeId) DEF_METATYPE_SCALAR(_Name, T, _TypeId)
#endif
#ifndef DEF_METATYPE_SCALAR_STRING
#   define DEF_METATYPE_SCALAR_STRING_UNDEF
#   define DEF_METATYPE_SCALAR_STRING(_Name, T, _TypeId) DEF_METATYPE_SCALAR(_Name, T, _TypeId)
#endif
#ifndef DEF_METATYPE_SCALAR_ATOM
#   define DEF_METATYPE_SCALAR_ATOM_UNDEF
#   define DEF_METATYPE_SCALAR_ATOM(_Name, T, _TypeId) DEF_METATYPE_SCALAR(_Name, T, _TypeId)
#endif
#ifndef DEF_METATYPE_SCALAR_OBJECT
#   define DEF_METATYPE_SCALAR_OBJECT_UNDEF
#   define DEF_METATYPE_SCALAR_OBJECT(_Name, T, _TypeId) DEF_METATYPE_SCALAR(_Name, T, _TypeId)
#endif

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DEF_METATYPE_SCALAR_ARITH       (bool,       bool,                          1)
DEF_METATYPE_SCALAR_ARITH       (i8,         i8,                            2)
DEF_METATYPE_SCALAR_ARITH       (i16,        i16,                           3)
DEF_METATYPE_SCALAR_ARITH       (i32,        i32,                           4)
DEF_METATYPE_SCALAR_ARITH       (i64,        i64,                           5)
DEF_METATYPE_SCALAR_ARITH       (u8,         u8,                            6)
DEF_METATYPE_SCALAR_ARITH       (u16,        u16,                           7)
DEF_METATYPE_SCALAR_ARITH       (u32,        u32,                           8)
DEF_METATYPE_SCALAR_ARITH       (u64,        u64,                           9)
DEF_METATYPE_SCALAR_ARITH       (float,      float,                         10)
DEF_METATYPE_SCALAR_ARITH       (double,     double,                        11)
DEF_METATYPE_SCALAR_VECTOR_ARITH(byte2,      byte2,                         12)
DEF_METATYPE_SCALAR_VECTOR_ARITH(byte4,      byte4,                         13)
DEF_METATYPE_SCALAR_VECTOR_ARITH(ubyte2,     ubyte2,                        14)
DEF_METATYPE_SCALAR_VECTOR_ARITH(ubyte4,     ubyte4,                        15)
DEF_METATYPE_SCALAR_VECTOR_ARITH(short2,     short2,                        16)
DEF_METATYPE_SCALAR_VECTOR_ARITH(short4,     short4,                        17)
DEF_METATYPE_SCALAR_VECTOR_ARITH(ushort2,    ushort2,                       18)
DEF_METATYPE_SCALAR_VECTOR_ARITH(ushort4,    ushort4,                       19)
DEF_METATYPE_SCALAR_VECTOR_ARITH(word2,      word2,                         20)
DEF_METATYPE_SCALAR_VECTOR_ARITH(word4,      word4,                         21)
DEF_METATYPE_SCALAR_VECTOR_ARITH(uword2,     uword2,                        22)
DEF_METATYPE_SCALAR_VECTOR_ARITH(uword4,     uword4,                        23)
DEF_METATYPE_SCALAR_VECTOR_ARITH(float2,     float2,                        24)
DEF_METATYPE_SCALAR_VECTOR_ARITH(float3,     float3,                        25)
DEF_METATYPE_SCALAR_VECTOR_ARITH(float4,     float4,                        26)
DEF_METATYPE_SCALAR_VECTOR_ARITH(float2x2,   float2x2,                      27)
DEF_METATYPE_SCALAR_VECTOR_ARITH(float3x3,   float3x3,                      28)
DEF_METATYPE_SCALAR_VECTOR_ARITH(float4x3,   float4x3,                      30)
DEF_METATYPE_SCALAR_VECTOR_ARITH(float4x4,   float4x4,                      31)
DEF_METATYPE_SCALAR_STRING      (String,     Core::String,                  32)
DEF_METATYPE_SCALAR_STRING      (WString,    Core::WString,                 33)
DEF_METATYPE_SCALAR_ATOM        (MetaAtom,   Core::RTTI::PMetaAtom,         34)
DEF_METATYPE_SCALAR_OBJECT      (MetaObject, Core::RTTI::PMetaObject,       35)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------

#ifdef DEF_METATYPE_SCALAR_ARITH_UNDEF
#   undef DEF_METATYPE_SCALAR_ARITH_UNDEF
#   undef DEF_METATYPE_SCALAR_ARITH
#endif
#ifdef DEF_METATYPE_SCALAR_VECTOR_ARITH_UNDEF
#   undef DEF_METATYPE_SCALAR_VECTOR_ARITH_UNDEF
#   undef DEF_METATYPE_SCALAR_VECTOR_ARITH
#endif
#ifdef DEF_METATYPE_SCALAR_STRING_UNDEF
#   undef DEF_METATYPE_SCALAR_STRING_UNDEF
#   undef DEF_METATYPE_SCALAR_STRING
#endif
#ifdef DEF_METATYPE_SCALAR_ATOM_UNDEF
#   undef DEF_METATYPE_SCALAR_ATOM_UNDEF
#   undef DEF_METATYPE_SCALAR_ATOM
#endif
#ifdef DEF_METATYPE_SCALAR_OBJECT_UNDEF
#   undef DEF_METATYPE_SCALAR_OBJECT_UNDEF
#   undef DEF_METATYPE_SCALAR_OBJECT
#endif
