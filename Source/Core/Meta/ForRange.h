#pragma once

#include "Core/Core.h"

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define forrange(_Variable, _Start, _End) \
    for (   std::remove_const<decltype(_End)>::type _Variable = _Start, CONCAT(_Variable, _END) = _End; \
            Assert(CONCAT(_Variable, _END) == _End), _Variable != CONCAT(_Variable, _END); \
            ++ _Variable )
//----------------------------------------------------------------------------
#define reverseforrange(_Variable, _Start, _End) \
    for (   std::remove_const<decltype(_End)>::type CONCAT(_Variable, _REV) = _Start, CONCAT(_Variable, _END) = _End, _Variable = CONCAT(_Variable, _END) - 1; \
            Assert(CONCAT(_Variable, _END) == _End), CONCAT(_Variable, _REV) != CONCAT(_Variable, _END); \
            ++ CONCAT(_Variable, _REV), -- _Variable )
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
