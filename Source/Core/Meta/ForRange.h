#pragma once

#include "Core/Core.h"

#include "Core/Meta/TypeTraits.h"

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define forrange(_Variable, _Start, _End) \
    for (   Core::Meta::Decay<decltype(_End)> _Variable = _Start, CONCAT(_Variable, _END) = _End; \
            Assert(CONCAT(_Variable, _END) == _End), _Variable != CONCAT(_Variable, _END); \
            ++ _Variable )
//----------------------------------------------------------------------------
#define reverseforrange(_Variable, _Start, _End) \
    for (   Core::Meta::Decay<decltype(_End)> CONCAT(_Variable, _REV) = _Start, CONCAT(_Variable, _END) = _End, _Variable = CONCAT(_Variable, _END) - 1; \
            Assert(CONCAT(_Variable, _END) == _End), CONCAT(_Variable, _REV) != CONCAT(_Variable, _END); \
            ++ CONCAT(_Variable, _REV), -- _Variable )
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define foreachitem(_Variable, _ContainerLike) \
    forrange(_Variable, std::begin(_ContainerLike), std::end(_ContainerLike))
//----------------------------------------------------------------------------
#define reverseforeachitem(_Variable, _ContainerLike) \
    forrange(_Variable, std::rbegin(_ContainerLike), std::rend(_ContainerLike))
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
