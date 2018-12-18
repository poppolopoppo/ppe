#pragma once

#include "Serialize_fwd.h"

#include "Misc/FourCC.h"

#define USE_PPE_BINA_MARKERS 0//%_NOCOMMIT%

#if USE_PPE_BINA_MARKERS
#   include "IO/TextWriter.h"
#   include "IO/String.h"
#   include "IO/StringBuilder.h"
#endif

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
    STATIC_ASSERT(sizeof(FContents) == sizeof(u32) * 8);

    struct FRawData {
        u32 Offset;
        u32 Size;
        u32 End() const { return (Offset + Size); }
    };
    STATIC_ASSERT(sizeof(FRawData) == sizeof(u32) * 2);

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
    STATIC_ASSERT(sizeof(FSections) == sizeof(FRawData) * 12);

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
        u32 Fingerprint;
        FContents Contents;
        FSections Sections;
    };
    STATIC_ASSERT(Meta::IsAligned(16, sizeof(FHeaders)));
    STATIC_ASSERT(sizeof(FHeaders) == sizeof(FFourCC) * 2 + sizeof(EHeaderFlags) + sizeof(u32) + sizeof(FContents) + sizeof(FSections));

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
    STATIC_ASSERT(sizeof(FSignature) == sizeof(u128) * 13);

#if USE_PPE_BINA_MARKERS
    static NO_INLINE FWString DumpInfos(const FHeaders& h, const FSignature& s) {
        FWStringBuilder oss;
        oss << FTextFormat::Hexadecimal
            << L"File info :" << Eol
            << Tab << L"Magic = " << h.Magic.MakeView() << Eol
            << Tab << L"Version = " << h.Version.MakeView() << Eol
            << Tab << L"Flags = " << u32(h.Flags) << Eol
            << Tab << L"Contents =" << Eol
            << Tab << Tab << L"NumNames = " << h.Contents.NumNames << Eol
            << Tab << Tab << L"NumClasses = " << h.Contents.NumClasses << Eol
            << Tab << Tab << L"NumProperties = " << h.Contents.NumProperties << Eol
            << Tab << Tab << L"NumImports = " << h.Contents.NumImports << Eol
            << Tab << Tab << L"NumObjects = " << h.Contents.NumObjects << Eol
            << Tab << Tab << L"NumDirpaths = " << h.Contents.NumDirpaths << Eol
            << Tab << Tab << L"NumBasenameNoExts = " << h.Contents.NumBasenameNoExts << Eol
            << Tab << Tab << L"NumExtnames = " << h.Contents.NumExtnames << Eol
            << Tab << L"Sections =" << Eol
            << Tab << Tab << L"Names = @" << h.Sections.Names.Offset << L" [" << h.Sections.Names.Size << L"]" << Eol
            << Tab << Tab << L"Classes = @" << h.Sections.Classes.Offset << L" [" << h.Sections.Classes.Size << L"]" << Eol
            << Tab << Tab << L"Properties = @" << h.Sections.Properties.Offset << L" [" << h.Sections.Properties.Size << L"]" << Eol
            << Tab << Tab << L"Imports = @" << h.Sections.Imports.Offset << L" [" << h.Sections.Imports.Size << L"]" << Eol
            << Tab << Tab << L"Dirpaths = @" << h.Sections.Dirpaths.Offset << L" [" << h.Sections.Dirpaths.Size << L"]" << Eol
            << Tab << Tab << L"BasenameNoExts = @" << h.Sections.BasenameNoExts.Offset << L" [" << h.Sections.BasenameNoExts.Size << L"]" << Eol
            << Tab << Tab << L"Extnames = @" << h.Sections.Extnames.Offset << L" [" << h.Sections.Extnames.Size << L"]" << Eol
            << Tab << Tab << L"Strings = @" << h.Sections.Strings.Offset << L" [" << h.Sections.Strings.Size << L"]" << Eol
            << Tab << Tab << L"WStrings = @" << h.Sections.WStrings.Offset << L" [" << h.Sections.WStrings.Size << L"]" << Eol
            << Tab << Tab << L"Text = @" << h.Sections.Text.Offset << L" [" << h.Sections.Text.Size << L"]" << Eol
            << Tab << Tab << L"Data = @" << h.Sections.Data.Offset << L" [" << h.Sections.Data.Size << L"]" << Eol
            << Tab << Tab << L"Bulk = @" << h.Sections.Bulk.Offset << L" [" << h.Sections.Bulk.Size << L"]" << Eol
            << Tab << L"Signature = " << h.Fingerprint << Eol
            << Tab << Tab << L"Headers = " << s.Headers.hi << L"-" << s.Headers.lo << Eol
            << Tab << Tab << L"Names = " << s.Names.hi << L"-" << s.Names.lo << Eol
            << Tab << Tab << L"Classes = " << s.Classes.hi << L"-" << s.Classes.lo << Eol
            << Tab << Tab << L"Imports = " << s.Imports.hi << L"-" << s.Imports.lo << Eol
            << Tab << Tab << L"Properties = " << s.Properties.hi << L"-" << s.Properties.lo << Eol
            << Tab << Tab << L"Dirpaths = " << s.Dirpaths.hi << L"-" << s.Dirpaths.lo << Eol
            << Tab << Tab << L"BasenameNoExts = " << s.BasenameNoExts.hi << L"-" << s.BasenameNoExts.lo << Eol
            << Tab << Tab << L"Extnames = " << s.Extnames.hi << L"-" << s.Extnames.lo << Eol
            << Tab << Tab << L"Strings = " << s.Strings.hi << L"-" << s.Strings.lo << Eol
            << Tab << Tab << L"WStrings = " << s.WStrings.hi << L"-" << s.WStrings.lo << Eol
            << Tab << Tab << L"Text = " << s.Text.hi << L"-" << s.Text.lo << Eol
            << Tab << Tab << L"Data = " << s.Data.hi << L"-" << s.Data.lo << Eol
            << Tab << Tab << L"Bulk = " << s.Bulk.hi << L"-" << s.Bulk.lo << Eol;
        return oss.ToString();
    }
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
