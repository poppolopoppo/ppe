# Source/Runtime/Application/Public/Window/

## Responsibility
Window management and platform abstraction. Creates, owns, and destroys platform windows. Routes window messages and events to application callbacks through the IWindowListener interface. This folder defines the FWindow class and IWindowListener abstract callback interface that all concrete programs use for window lifecycle management, ensuring a consistent window ownership model across platforms.

## Design
FWindow class owns the platform-specific window handle (HWND on Windows, Display* on Linux, GLFWwindow* on GLFW). IWindowListener is the abstract callback interface for OnShow, OnFocus, OnResize, OnClose. Platform window creation factory selects implementation based on target platform. Uses EXTERN_LOG_CATEGORY for creation/destruction tracing. The ownership model has FWindow as the sole owner; when FWindow is destroyed, the platform window is destroyed. Multiple listener registration is supported but the primary listener receives all callbacks.

## Flow
FWindow::Create() → platform-specific implementation → show() → register listener → message pump → OnShow/OnFocus/OnResize/OnClose callbacks → OnDestroy → cleanup of platform resources. Message pump routes through FWindowsPlatformApplicationMisc on Windows, forwarding messages to the HAL layer. OnResize triggers projection matrix recalculation via the viewport. Listener deregistration occurs during window destruction. The flow ensures that window events are delivered to application code in a predictable, well-ordered sequence.

## Integration
Source/Runtime/Application/Public/HAL/ (platform hooks that source window events), Source/Runtime/Application/Private/Window/ (private window details, if present), Source/Programs/WindowTest/ (consumer of window service, extensive RHI test harness), Source/Extensions/ApplicationUI/Public/UI/ (ImGui dockspace host, widgets consume window events), Source/Programs/ShaderToy/, Source/Programs/VoxelCube/ (inherit FApplicationWindow and create FWindow instances)