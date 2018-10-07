#pragma once

#include "Core.h"

#ifdef EXPORT_PPE_APPLICATION
#   define PPE_APPLICATION_API DLL_EXPORT
#else
#   define PPE_APPLICATION_API DLL_IMPORT
#endif

#include "Memory/RefPtr.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EGamepadButton : u8;
enum class EKeyboardKey : u8;
enum class EMouseButton : u8;
//----------------------------------------------------------------------------
class FKeyboardState;
class FGamepadState;
class FMouseState;
class FGenericApplication;
class FGenericWindow;
//----------------------------------------------------------------------------
class FApplicationBase;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
