# Source/Runtime/Application/Public/HAL/Linux/

## Responsibility
Linux platform implementation of application misc hooks. Adapts application lifecycle events to Linux window system conventions (X11/Wayland) while honoring the FGenericPlatformApplicationMisc interface contract. This layer provides the concrete Linux realization of the generic app lifecycle abstraction, enabling windowed applications to receive consistent OnWindowShow, OnWindowFocus, OnWindowResize, and OnWindowClose callbacks across different Linux display servers.

## Design
Linux-specific overrides of OnWindowShow, OnWindowFocus, OnWindowResize, OnWindowClose. Uses X11/Wayland protocol for window state tracking. Maintains app state machine (activated, deactivated, minimized, restored) analogous to the Windows variant but adapted to ICCCM/NETWM window property conventions. EXTERN_LOG_CATEGORY for diagnostics. Conditional compilation behind PLATFORM_LINUX. The class is consumed via the same FGenericPlatformApplicationMisc interface pointer, ensuring uniform access across platforms.

## Flow
Linux window system event (X11/Wayland) → OnWindowShow/OnWindowFocus/OnWindowResize/OnWindowClose → application callback. State machine tracks minimized/maximized/normal modes specific to X11/Wayland semantics. Property notifications (WM_PROTOCOLS, WM_TAKE_FOCUS) are intercepted and translated to the abstract interface methods. Between frames, state is preserved for consistent duty-cycle evaluation.

## Integration
Source/Runtime/Application/Public/HAL/Generic/ (base interface contract), Source/Runtime/Application/Private/HAL/ (private implementation details), Source/Programs/ (Linux test targets and cross-platform consumers)