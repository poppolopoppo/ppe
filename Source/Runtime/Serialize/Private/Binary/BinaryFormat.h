#pragma once

#include "Serialize_fwd.h"

#include "Misc/FourCC.h"

#define USE_PPE_BINA_MARKERS 0//%_NOCOMMIT%

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FBinaryFormat {
    STATIC_ASSERT(sizeof(FFourCC) == sizeof(u32));

    static CONSTEXPR FFourCC FILE_MAGIC     = "BINA";
    static CONSTEXPR FFourCC FILE_VERSION   = "2.00";

    struct FContents {
        u32 NumNames;
        u32 NumClasses;
        u32 NumProperties;
        u32 NumImports;
        u32 NumObjects;
        u32 NumDirpaths;
        u32 NumBasenameNoExts;
        u32 NumExtnames;
    };

    struct FRawData {
        u32 Offset;
        u32 Size;
        u32 End() const { return (Offset + Size); }
    };

    using FDataIndex = TNumericDefault<u32, FBinaryFormat, u32(-1)>;

    struct FSections {
        FRawData Names;
        FRawData Classes;
        FRawData Properties;
        FRawData Imports;
        FRawData Dirpaths;
        FRawData BasenameNoExts;
        FRawData Extnames;
        FRawData Strings;
        FRawData WStrings;
        FRawData Text;
        FRawData Data;
        FRawData Bulk;
    };

    enum EObjectFlags : u16 {
        Default                 = 0,
        Export                  = 1<<0,
        TopObject               = 1<<1,
    };
    ENUM_FLAGS_FRIEND(EObjectFlags);

    struct FAnyData {
        RTTI::ENativeType Type;
    };
    STATIC_ASSERT(sizeof(FAnyData) == sizeof(u32));

    struct FArrayData {
        u32 NumElements;
    };
    STATIC_ASSERT(sizeof(FArrayData) == sizeof(u32));

    struct FBulkData {
        FRawData Stream;
    };
    STATIC_ASSERT(sizeof(FBulkData) == sizeof(u32)*2);

    struct FObjectData {
        EObjectFlags Flags;
        u16 NumProperties;
        u32 ClassIndex;
        u32 NameIndex;
    };
    STATIC_ASSERT(sizeof(FObjectData) == sizeof(u32)*3);

    struct FPathData {
        u32 Dirpath;
        u32 BasenameNoExt;
        u32 Extname;
    };
    STATIC_ASSERT(sizeof(FPathData) == sizeof(u32)*3);

    struct FPropertyData {
        u32 PropertyIndex;
    };
    STATIC_ASSERT(sizeof(FPropertyData) == sizeof(u32));

    struct FImportData {
        u32 TransactionIndex;
        u32 NameIndex;
    };
    STATIC_ASSERT(sizeof(FImportData) == sizeof(u64));

    struct FReferenceData {
        u32 IsImport : 1;
        u32 ObjectIndex : 31;
        bool IsNull() const { return ((FDataIndex::DefaultValue() >> 1) == ObjectIndex); }
    };
    STATIC_ASSERT(sizeof(FReferenceData) == sizeof(u32));

    enum EHeaderFlags : u32 {
        None                    = 0,
        HasBulkData             = 1<<0,
        HasExternalExports      = 1<<1,
        HasExternalImports      = 1<<2,
    };
    ENUM_FLAGS_FRIEND(EHeaderFlags);

    struct FHeaders {
        FFourCC Magic;
        FFourCC Version;
        EHeaderFlags Flags;
        FContents Contents;
        FSections Sections;
        u128 Fingerprint;
    };
    STATIC_ASSERT(Meta::IsAligned(16, sizeof(FHeaders)));

    struct FSignature {
        u128 Headers;
        u128 Names;
        u128 Classes;
        u128 Imports;
        u128 Properties;
        u128 Dirpaths;
        u128 BasenameNoExts;
        u128 Extnames;
        u128 Strings;
        u128 WStrings;
        u128 Text;
        u128 Data;
        u128 Bulk;
    };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
