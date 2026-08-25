# Source/Runtime/Application/Public/HAL/

## Responsibility
Hardware Abstraction Layer interface for application-platform interaction. Defines the pure virtual contract for platform-specific application misc hooks that the application layer depends on without knowing the concrete platform. This abstraction boundary allows application code to call platform lifecycle methods (window show, focus, resize, close) through a generic interface while the concrete implementation handles OS-specific details.

## Design
FGenericPlatformApplicationMisc declares pure virtual methods for app lifecycle events (OnWindowShow, OnWindowFocus, OnWindowResize, OnWindowClose) with pure virtual = delete, enforcing concrete implementation in platform-specific subclasses. FWindowsPlatformApplicationMisc provides the Windows override. FGenericPlatformApplicationMisc is consumed via interface pointer (FGenericPlatformApplicationMisc*) to polymorphic call. EXTERN_LOG_CATEGORY is used for structured tracing of app lifecycle events. The pure virtual interface contract ensures every platform implements the full set of lifecycle methods, while concrete subclasses fill in OS-specific details.

## Flow
Application calls platform service methods through the FGenericPlatformApplicationMisc interface → HAL routes to concrete implementation → platform-specific handling of window events and app lifecycle → callbacks delivered to application via IWindowListener or direct function objects. During app initialization, the concrete platform instance is set as the global service. On shutdown, the interface pointer is cleared. Pure virtual methods have no default implementation, so each platform subclass provides the complete flow.

## Integration
Source/Runtime/Application/Public/HAL/Windows/ (Windows implementation), Source/Runtime/Application/Public/HAL/Linux/ (Linux implementation), Source/Runtime/Application/Private/HAL/ (private implementation details), Source/Runtime/Application/Public/Window/ (event source through window management)