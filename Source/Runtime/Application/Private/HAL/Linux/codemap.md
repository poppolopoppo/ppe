# Source/Runtime/Application/Private/HAL/Linux/

## Responsibility
Linux private HAL implementation adapting FGenericPlatformApplicationMisc to Linux window system conventions. Contains X11/Wayland-specific event handling and app lifecycle management behind the public abstraction boundary. This is the private detail class that satisfies the FGenericPlatformApplicationMisc contract on Linux, hiding the X11/Wayland protocol details from the public header.

## Design
Linux-specific implementation of FGenericPlatformApplicationMisc pure virtual methods. X11/Wayland window state tracking using XDisplay* and Window handles. Conditional compilation behind PLATFORM_LINUX. Uses XSelectInput, XChangeProperty, and related X11/Wayland APIs for window state queries. EXTERN_LOG_CATEGORY for diagnostics. The private class hides the X11/Wayland display connection management, property atom handling, and event loop integration details behind the uniform FGenericPlatformApplicationMisc interface.

## Flow
X11/Wayland event loop → state update → OnWindowShow/OnWindowFocus/OnWindowResize/OnWindowClose → application callback. State machine tracks ICCCM/NETWM-specific modes. Property notifications are intercepted and translated to the abstract interface methods. The private class manages the display connection lifecycle in sync with the application's startup and shutdown.

## Integration
Source/Runtime/Application/Public/HAL/Generic/ (contract), Source/Runtime/Application/Private/HAL/ (parent), Source/Programs/ (Linux test targets and cross-platform consumers)