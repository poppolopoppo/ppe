#pragma once

#include "Core.Logic/Logic.h"

#include "Core/Memory/RefPtr.h"
#include "Core/Meta/StronglyTyped.h"

namespace Core {
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
CORE_STRONGLYTYPED_NUMERIC_DEF(size_t,  ComponentTag);
CORE_STRONGLYTYPED_NUMERIC_DEF(u32,     ComponentID);
CORE_STRONGLYTYPED_NUMERIC_DEF(u32,     ComponentFlag);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
