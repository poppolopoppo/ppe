#pragma once

#include "Core.h"

#ifdef EXPORT_PPE_RUNTIME_SERIALIZE
#   define PPE_SERIALIZE_API DLL_EXPORT
#else
#   define PPE_SERIALIZE_API DLL_IMPORT
#endif

#include "Memory/RefPtr.h"

namespace PPE {
template <typename T>
struct TInSituPtr;
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ESerializeFlags : u32;
enum class ESerializeFormat : u32;
//----------------------------------------------------------------------------
class ISerializer;
using PSerializer = TInSituPtr<ISerializer>;
//----------------------------------------------------------------------------
class FTransactionLinker;
class FTransactionSaver;
FWD_REFPTR(TransactionSerializer);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
