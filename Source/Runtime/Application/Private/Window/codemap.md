# Source/Runtime/Application/Private/Window/

## Responsibility
Private window management implementation behind the public FWindow abstraction. Handles platform-specific window creation, message loop integration, and ownership semantics that the public API conceals from application authors. This folder contains the concrete window implementation details that support the FWindow public interface, including platform handle management and IWindowListener callback routing.

## Design
FPrivateWindow implements the platform-specific window creation factory, selecting the appropriate backend (Windows HWND, Linux Display*, GLFWwindow*) based on the target platform. Owns the platform window handle and manages its lifecycle from creation to destruction. Implements IWindowListener for callback routing, with the primary listener receiving all window events. Uses EXTERN_LOG_CATEGORY for creation/destruction tracing. The pimpl idiom may be used to conceal OS-specific handle details from the public header.

## Flow
FPrivateWindow::Create() → platform-specific impl → show() → register IWindowListener → message pump → OnShow/OnFocus/OnResize/OnClose callbacks → OnDestroy → cleanup of platform resources. Message pump routes through FWindowsPlatformApplicationMisc on Windows, forwarding messages to the HAL layer. Listener deregistration occurs during window destruction. The flow ensures that window events are delivered to application code in a predictable, well-ordered sequence.

## Integration
Source/Runtime/Application/Public/Window/ (public window API mirror), Source/Runtime/Application/Private/Application/ (owns private window instance), Source/Programs/WindowTest/ (consumer of window service, extensive RHI test harness), Source/Legacy/RHI/Private/Window (ImGui dockspace host, widgets consume window events), Source/Programs/ShaderToy/, Source/Programs/VoxelCube/ (inherit FApplicationWindow and create FWindow instances)