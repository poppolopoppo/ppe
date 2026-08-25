# Source/Runtime/Application/Private/HAL/Windows/

## Responsibility
Windows private HAL implementation of application misc hooks. Contains the concrete Windows-specific implementation of FGenericPlatformApplicationMisc with message hook installation, window procedure management, and app lifecycle event delivery. This is the private detail class that satisfies the FGenericPlatformApplicationMisc contract on Windows, hiding hook patterns and message filter chain details from the public header.

## Design
Windows-specific override of FGenericPlatformApplicationMisc methods. Window procedure subclassing for message interception. Message filter chain for WM_SIZE, WM_MOVE, WM_CLOSE, WM_ACTIVATE. App state machine (activated, deactivated, minimized, restored). Uses EXTERN_LOG_CATEGORY for trace. Detours or direct hook patterns for message capture may be employed. The class is designed for single-instance global access during the application lifetime, with hook installation during app initialization and cleanup on shutdown. Private details of message hook patterns are concealed from the public header.

## Flow
Message pump → window procedure hook → FWindowsPlatformApplicationMisc handler → OnWindowShow/OnWindowFocus/OnWindowResize/OnWindowClose → application callback → IWindowListener notification. Hook cleanup on app shutdown to restore original window procedures. Each message type routes to a specific lifecycle method. Message installation occurs during app init; deregistration during shutdown.

## Integration
Source/Runtime/Application/Public/HAL/Windows/ (public mirror), Source/Runtime/Application/Public/HAL/Generic/ (contract), Source/Programs/WindowTest/ (consumer of window event service), Source/Legacy/RHI/Private/Window (ImGui dockspace and message integration)