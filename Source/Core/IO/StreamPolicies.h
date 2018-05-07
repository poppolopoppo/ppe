#pragma once

#include "Core/Core.h"

#include "Core/IO/TextWriter_fwd.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EOpenPolicy : size_t {
    Readable        = 1 << 0,
    Writable        = 1 << 1,
    ReadWritable    = Readable | Writable,
};
ENUM_FLAGS(EOpenPolicy);
CORE_API FTextWriter& operator <<(FTextWriter& oss, EOpenPolicy policy);
CORE_API FWTextWriter& operator <<(FWTextWriter& oss, EOpenPolicy policy);
//----------------------------------------------------------------------------
enum class EAccessPolicy : size_t {

    None            = 0,

    Binary          = 1 << 0,
    Text            = 1 << 1,
    TextU8          = 1 << 2,
    TextU16         = 1 << 3,
    TextW           = 1 << 4,

    Create          = 1 << 5,
    Append          = 1 << 6,
    Truncate        = 1 << 7,

    Random          = 1 << 8,   // Optimized for random access
    Sequential      = 1 << 9,   // Optimized for sequential access, but random still available

    ShortLived      = 1 << 10,  // Temporary file, try as much as possible to don't flush to disk (use with |Create)
    Temporary       = 1 << 11,  // Delete files after when last descriptor is closed (use with |Create)
    Exclusive       = 1 << 12,  // Fail if the file already exists (use with |Create)
    Compress        = 1 << 13,  // Uses Core::Compression
    ShareRead       = 1 << 14,  // Allow other process to read the file while writing

    Create_Binary   = Create | Binary,
    Create_Text     = Create | Text,

    Truncate_Binary = Truncate | Binary,
    Truncate_Text   = Truncate | Text,
};
ENUM_FLAGS(EAccessPolicy);
CORE_API FTextWriter& operator <<(FTextWriter& oss, EAccessPolicy policy);
CORE_API FWTextWriter& operator <<(FWTextWriter& oss, EAccessPolicy policy);
//----------------------------------------------------------------------------
enum class EExistPolicy : size_t {
    Exists          = 1 << 0,
    WriteOnly       = 1 << 1,
    ReadOnly        = 1 << 2,
    ReadWrite       = 1 << 3,
};
ENUM_FLAGS(EExistPolicy);
//----------------------------------------------------------------------------
enum class ESeekOrigin : size_t {
    Begin = 0,
    Relative,
    End,
    All,
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
