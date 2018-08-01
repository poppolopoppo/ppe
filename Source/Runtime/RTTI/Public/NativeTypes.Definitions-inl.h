#pragma once

// Here goes the list of all BASIC types supported by RTTI

#include "RTTI_fwd.h"
#include "Typedefs.h"

#include "IO/FileSystem_fwd.h"
#include "IO/String_fwd.h"

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
    COMMA_PROTECT(_Macro(String,        Core::FString,                 12 )) \
    COMMA_PROTECT(_Macro(WString,       Core::FWString,                13 )) \
    COMMA_PROTECT(_Macro(Name,          Core::RTTI::FName,             14 )) \
    COMMA_PROTECT(_Macro(Dirpath,       Core::FDirpath,                15 )) \
    COMMA_PROTECT(_Macro(Filename,      Core::FFilename,               16 )) \
    COMMA_PROTECT(_Macro(MetaObject,    Core::RTTI::PMetaObject,       17 )) \
    COMMA_PROTECT(_Macro(BinaryData,    Core::RTTI::FBinaryData,       18 )) \
    COMMA_PROTECT(_Macro(Any,           Core::RTTI::FAny,              19 ))
