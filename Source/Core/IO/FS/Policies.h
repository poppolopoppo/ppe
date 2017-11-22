#pragma once

#include "Core/Core.h"

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

    Truncate_Binary = Truncate | Binary,
    Truncate_Text   = Truncate | Text,
};
ENUM_FLAGS(EAccessPolicy);
std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, EAccessPolicy policy);
std::basic_ostream<wchar_t>& operator <<(std::basic_ostream<wchar_t>& oss, EAccessPolicy policy);
//----------------------------------------------------------------------------
enum class EExistPolicy : size_t{
    Exists          = 1 << 0,
    WriteOnly       = 1 << 1,
    ReadOnly        = 1 << 2,
    ReadWrite       = 1 << 3,
};
ENUM_FLAGS(EExistPolicy);
//----------------------------------------------------------------------------
enum class ESeekOrigin : size_t{
    Begin = 0,
    Relative,
    End,
    All,
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
