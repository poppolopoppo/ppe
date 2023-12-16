#pragma once

#include "Core.Logic/Logic.h"

#include "Memory/RefPtr.h"
#include "Meta/StronglyTyped.h"

namespace PPE {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ISystem;
typedef TRefPtr<ISystem> PSystem;
typedef TRefPtr<const ISystem> PCSystem;
typedef TSafePtr<ISystem> SSystem;
typedef TSafePtr<const ISystem> SCSystem;
//----------------------------------------------------------------------------
template <typename T>
class ITypedSystem;
template <typename T>
using TPTypedSystem = TRefPtr<ITypedSystem<T>>;
template <typename T>
using TPCTypedSystem = TRefPtr<const ITypedSystem<T>>;
template <typename T>
using TSTypedSystem = TSafePtr<ITypedSystem<T>>;
template <typename T>
using TSCTypedSystem = TSafePtr<const ITypedSystem<T>>;
//----------------------------------------------------------------------------
enum class ESystemExecution;
//----------------------------------------------------------------------------
class FSystemLayer;
FWD_REFPTR(SystemLayer);
//----------------------------------------------------------------------------
class FSystemContainer;
//----------------------------------------------------------------------------
PPE_STRONGLYTYPED_NUMERIC_DEF(u32, MessageID);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace PPE
