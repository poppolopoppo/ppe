#pragma once

#include "Core.h"

#ifdef EXPORT_PPE_RUNTIME_APPLICATION
#   define PPE_APPLICATION_API DLL_EXPORT
#else
#   define PPE_APPLICATION_API DLL_IMPORT
#endif

#include "Memory/RefPtr.h"
#include "Memory/UniquePtr.h"

namespace PPE {
class FApplicationModule;
FWD_INTEFARCE_UNIQUEPTR(InputService);
FWD_INTEFARCE_UNIQUEPTR(UIService);
FWD_INTEFARCE_UNIQUEPTR(WindowService);
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EGamepadButton : u8;
enum class EKeyboardKey : u8;
enum class EMouseButton : u8;
class FKeyboardState;
class FGamepadState;
class FMouseState;
//----------------------------------------------------------------------------
class FGenericWindow;
class IWindowListener;
FWD_REFPTR(MainWindow);
//----------------------------------------------------------------------------
class FGenericApplication;
class FApplicationBase;
class FApplicationConsole;
class FApplicationWindow;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
