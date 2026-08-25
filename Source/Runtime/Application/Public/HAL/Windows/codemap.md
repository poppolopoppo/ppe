# Source/Runtime/Application/Public/HAL/Windows/

## Responsibility
Windows-specific implementation of application misc hooks. Routes OS window messages and app lifecycle events to the application layer through the FWindowsPlatformApplicationMisc concrete class. This layer handles message pump integration, window procedure management, and provides the concrete realization of the FGenericPlatformApplicationMisc interface for Windows platforms.

## Design
FWindowsPlatformApplicationMisc implements ITargetPlatform::GetApplicationMisc() override. Installs window procedure hooks, handles WM_SIZE, WM_MOVE, WM_CLOSE, WM_ACTIVATE messages. Maintains app state machine (activated, deactivated, minimized, restored). Uses EXTERN_LOG_CATEGORY for trace output. Message hook patterns may employ Detours or direct function hooking for transparent message interception. The class is designed for single-instance global access during the application lifetime, with hook installation during app initialization and cleanup on shutdown.

## Flow
Windows message pump → FWindowsPlatformApplicationMisc message handler → OnWindowShow/OnWindowFocus/OnWindowResize/OnWindowClose → application callback → IWindowListener notification. Message hooks are installed during app initialization via the platform service locator. Each message type routes to a specific lifecycle method: WM_SIZE → OnWindowResize (with new dimensions), WM_MOVE → position update, WM_CLOSE → OnWindowClose, WM_ACTIVATE → OnWindowFocus with active/inactive state. Hook cleanup occurs on app shutdown to restore original window procedures.

## Integration
Source/Runtime/Application/Public/HAL/Generic/ (base interface contract), Source/Runtime/Application/Private/HAL/Windows/ (private implementation details), Source/Programs/WindowTest/ (consumer of window event service), Source/Legacy/RHI/Private/Window (ImGui dockspace and message integration)