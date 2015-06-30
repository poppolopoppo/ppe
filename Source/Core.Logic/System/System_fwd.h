#pragma once

#include "Core.Logic/Logic.h"

#include "Core/Memory/RefPtr.h"
#include "Core/Meta/StronglyTyped.h"

namespace Core {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ISystem;
typedef RefPtr<ISystem> PSystem;
typedef RefPtr<const ISystem> PCSystem;
typedef SafePtr<ISystem> SSystem;
typedef SafePtr<const ISystem> SCSystem;
//----------------------------------------------------------------------------
template <typename T>
class ITypedSystem;
template <typename T>
using PTypedSystem = RefPtr<ITypedSystem<T>>;
template <typename T>
using PCTypedSystem = RefPtr<const ITypedSystem<T>>;
template <typename T>
using STypedSystem = SafePtr<ITypedSystem<T>>;
template <typename T>
using SCTypedSystem = SafePtr<const ITypedSystem<T>>;
//----------------------------------------------------------------------------
enum class SystemExecution;
//----------------------------------------------------------------------------
class SystemLayer;
FWD_REFPTR(SystemLayer);
//----------------------------------------------------------------------------
class SystemContainer;
//----------------------------------------------------------------------------
CORE_STRONGLYTYPED_NUMERIC_DEF(u32, MessageID);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
