﻿#pragma once

#include "Core.h"

#ifdef EXPORT_PPE_SERIALIZE
#   define PPE_SERIALIZE_API DLL_EXPORT
#else
#   define PPE_SERIALIZE_API DLL_IMPORT
#endif

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE