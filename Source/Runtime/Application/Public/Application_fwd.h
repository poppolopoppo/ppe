#pragma once

#include "Core.h"

#ifdef EXPORT_PPE_RUNTIME_APPLICATION
#   define PPE_APPLICATION_API DLL_EXPORT
#else
#   define PPE_APPLICATION_API DLL_IMPORT
#endif

#include "Memory/RefPtr.h"

namespace PPE {
class FApplicationModule;
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
//----------------------------------------------------------------------------
class FGenericApplication;
class FApplicationBase;
//----------------------------------------------------------------------------
class FGenericWindow;
FWD_REFPTR(WindowBase);
FWD_REFPTR(WindowBare);
FWD_REFPTR(WindowRHI);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
