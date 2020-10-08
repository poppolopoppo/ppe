#pragma once

#include "Serialize_fwd.h"

#include "IO/StringView.h"
#include "Memory/MemoryView.h"

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FFATFormat {
    struct FOffset {
        u32 Offset;
    };
    STATIC_ASSERT(sizeof(u32) == sizeof(FOffset));

    struct FNamedOffset : FOffset {
        char* Name;
    };

    struct FHeaders {
        TMemoryView<FOffset> Offsets;
        TMemoryView<FNamedOffset> Exports;
        TMemoryView<FNamedOffset> Imports;
    };

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
