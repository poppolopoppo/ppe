#pragma once

#include "Core.h"

#ifdef EXPORT_PPE_SERIALIZE
#   define PPE_SERIALIZE_API DLL_EXPORT
#else
#   define PPE_SERIALIZE_API DLL_IMPORT
#endif

namespace PPE {
template <typename T>
struct TInSituPtr;
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ISerializer;
using PTypeTraits = TInSituPtr<ISerializer>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
