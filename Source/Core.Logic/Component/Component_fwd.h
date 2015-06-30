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
typedef RefPtr<IComponent> PComponent;
typedef RefPtr<const IComponent> PCComponent;
typedef SafePtr<IComponent> SComponent;
typedef SafePtr<const IComponent> SCComponent;
//----------------------------------------------------------------------------
template <typename T>
class ITypedComponent;
template <typename T>
using PTypedComponent = RefPtr<ITypedComponent<T>>;
template <typename T>
using PCTypedComponent = RefPtr<const ITypedComponent<T>>;
template <typename T>
using STypedComponent = SafePtr<ITypedComponent<T>>;
template <typename T>
using SCTypedComponent = SafePtr<const ITypedComponent<T>>;
//----------------------------------------------------------------------------
CORE_STRONGLYTYPED_NUMERIC_DEF(size_t,  ComponentTag);
CORE_STRONGLYTYPED_NUMERIC_DEF(u32,     ComponentID);
CORE_STRONGLYTYPED_NUMERIC_DEF(u32,     ComponentFlag);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
