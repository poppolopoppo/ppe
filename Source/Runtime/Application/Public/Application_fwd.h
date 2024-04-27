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
FWD_INTEFARCE_UNIQUEPTR(ApplicationService);
FWD_INTEFARCE_UNIQUEPTR(InputService);
FWD_INTEFARCE_UNIQUEPTR(UIService);
FWD_INTEFARCE_UNIQUEPTR(WindowService);
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EGamepadAxis : u8;
enum class EGamepadButton : u8;
enum class EKeyboardKey : u8;
enum class EMouseAxis : u8;
enum class EMouseButton : u8;
class FKeyboardState;
class FGamepadState;
class FMouseState;
//----------------------------------------------------------------------------
PPE_STRONGLYTYPED_NUMERIC_DEF(u32, FInputDeviceID);
enum EInputMessageEvent : u8;
class IInputDevice;
struct FInputKey;
struct FInputValue;
struct FInputMessage;
//----------------------------------------------------------------------------
enum class EInputActionFlags : u8;
enum class EInputValueType : u8;
class FInputActionInstance;
struct FInputActionKeyMapping;
FWD_REFPTR(InputAction);
FWD_REFPTR(InputMapping);
FWD_REFPTR(InputListener);
//----------------------------------------------------------------------------
class FGenericWindow;
class IWindowListener;
FWD_REFPTR(MainWindow);
//----------------------------------------------------------------------------
enum class ECameraProjection : u8;
enum class EViewportFlags : u8;
class FCameraModel;
class ICameraController;
class FViewport;
FWD_REFPTR(Camera);
FWD_REFPTR(ViewportClient);
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
