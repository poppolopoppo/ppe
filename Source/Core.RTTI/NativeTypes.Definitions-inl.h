#pragma once

// Here goes the list of all BASIC types supported by RTTI
// Use TMetaTypeTraits.h to project your custom type to one of these

#include "Core.RTTI/RTTI_fwd.h"
#include "Core.RTTI/Typedefs.h"

#include "Core/IO/FileSystem_fwd.h"
#include "Core/IO/String_fwd.h"
#include "Core/Maths/ScalarMatrix_fwd.h"
#include "Core/Maths/ScalarVector_fwd.h"

#define FOREACH_RTTI_NATIVETYPES(_Macro) \
    COMMA_PROTECT(_Macro(Bool,          bool,                           1 )) \
    COMMA_PROTECT(_Macro(Int8,          i8,                             2 )) \
    COMMA_PROTECT(_Macro(Int16,         i16,                            3 )) \
    COMMA_PROTECT(_Macro(Int32,         i32,                            4 )) \
    COMMA_PROTECT(_Macro(Int64,         i64,                            5 )) \
    COMMA_PROTECT(_Macro(UInt8,         u8,                             6 )) \
    COMMA_PROTECT(_Macro(UInt16,        u16,                            7 )) \
    COMMA_PROTECT(_Macro(UInt32,        u32,                            8 )) \
    COMMA_PROTECT(_Macro(UInt64,        u64,                            9 )) \
    COMMA_PROTECT(_Macro(Float,         float,                         10 )) \
    COMMA_PROTECT(_Macro(Double,        double,                        11 )) \
    COMMA_PROTECT(_Macro(Byte2,         Core::byte2,                   12 )) \
    COMMA_PROTECT(_Macro(Byte4,         Core::byte4,                   13 )) \
    COMMA_PROTECT(_Macro(Ubyte2,        Core::ubyte2,                  14 )) \
    COMMA_PROTECT(_Macro(Ubyte4,        Core::ubyte4,                  15 )) \
    COMMA_PROTECT(_Macro(Short2,        Core::short2,                  16 )) \
    COMMA_PROTECT(_Macro(Short4,        Core::short4,                  17 )) \
    COMMA_PROTECT(_Macro(UShort2,       Core::ushort2,                 18 )) \
    COMMA_PROTECT(_Macro(UShort4,       Core::ushort4,                 19 )) \
    COMMA_PROTECT(_Macro(Word2,         Core::word2,                   20 )) \
    COMMA_PROTECT(_Macro(Word3,         Core::word3,                   21 )) \
    COMMA_PROTECT(_Macro(Word4,         Core::word4,                   22 )) \
    COMMA_PROTECT(_Macro(UWord2,        Core::uword2,                  23 )) \
    COMMA_PROTECT(_Macro(UWord3,        Core::uword3,                  24 )) \
    COMMA_PROTECT(_Macro(UWord4,        Core::uword4,                  25 )) \
    COMMA_PROTECT(_Macro(Float2,        Core::float2,                  26 )) \
    COMMA_PROTECT(_Macro(Float3,        Core::float3,                  27 )) \
    COMMA_PROTECT(_Macro(Float4,        Core::float4,                  28 )) \
    COMMA_PROTECT(_Macro(Float2x2,      Core::float2x2,                29 )) \
    COMMA_PROTECT(_Macro(Float3x3,      Core::float3x3,                30 )) \
    COMMA_PROTECT(_Macro(Float4x3,      Core::float4x3,                31 )) \
    COMMA_PROTECT(_Macro(Float4x4,      Core::float4x4,                32 )) \
    COMMA_PROTECT(_Macro(String,        Core::FString,                 33 )) \
    COMMA_PROTECT(_Macro(WString,       Core::FWString,                34 )) \
    COMMA_PROTECT(_Macro(Name,          Core::RTTI::FName,             35 )) \
    COMMA_PROTECT(_Macro(Dirpath,       Core::FDirpath,                36 )) \
    COMMA_PROTECT(_Macro(Filename,      Core::FFilename,               37 )) \
    COMMA_PROTECT(_Macro(MetaObject,    Core::RTTI::PMetaObject,       38 )) \
    COMMA_PROTECT(_Macro(BinaryData,    Core::RTTI::FBinaryData,       39 )) \
    COMMA_PROTECT(_Macro(Any,           Core::RTTI::FAny,              40 ))
