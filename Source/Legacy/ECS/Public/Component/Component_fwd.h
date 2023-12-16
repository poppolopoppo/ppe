#pragma once

#include "Core.Logic/Logic.h"

#include "Memory/RefPtr.h"
#include "Meta/StronglyTyped.h"

namespace PPE {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IComponent;
typedef TRefPtr<IComponent> PComponent;
typedef TRefPtr<const IComponent> PCComponent;
typedef TSafePtr<IComponent> SComponent;
typedef TSafePtr<const IComponent> SCComponent;
//----------------------------------------------------------------------------
template <typename T>
class ITypedComponent;
template <typename T>
using TPTypedComponent = TRefPtr<ITypedComponent<T>>;
template <typename T>
using TPCTypedComponent = TRefPtr<const ITypedComponent<T>>;
template <typename T>
using TSTypedComponent = TSafePtr<ITypedComponent<T>>;
template <typename T>
using TSCTypedComponent = TSafePtr<const ITypedComponent<T>>;
//----------------------------------------------------------------------------
PPE_STRONGLYTYPED_NUMERIC_DEF(size_t,  ComponentTag);
PPE_STRONGLYTYPED_NUMERIC_DEF(u32,     ComponentID);
PPE_STRONGLYTYPED_NUMERIC_DEF(u32,     ComponentFlag);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace PPE
