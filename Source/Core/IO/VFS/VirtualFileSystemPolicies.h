#pragma once

#include "Core/Core.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace AccessPolicy {
    enum EMode {
        None        = 0,
        Text        = 0, // default behavior
        Create      = 1 << 0,
        Truncate    = 1 << 1,
        Binary      = 1 << 2,
        Ate         = 1 << 3,
        Random      = 1 << 4,
        ShortLived  = 1 << 5,
        Temporary   = 1 << 6,

        Ate_Binary      = Ate|Binary,
        Truncate_Binary = Truncate|Binary,
    };
}
ENUM_FLAGS(AccessPolicy::EMode);
//----------------------------------------------------------------------------
namespace ExistPolicy {
    enum EMode {
        Exists      = 0,
        WriteOnly   = 2,
        ReadOnly    = 4,
        ReadWrite   = 6,
    };
}
ENUM_FLAGS(ExistPolicy::EMode);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
