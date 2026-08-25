# Source/Runtime/Application/Public/HAL/GLFW/

## Responsibility
GLFW-based implementation of application misc hooks. GLFW window management and event polling provide the foundation for application lifecycle event delivery in contexts where GLFW is the primary windowing system. This layer maps GLFW callbacks to the FGenericPlatformApplicationMisc interface, providing a platform-agnostic abstraction layer above raw GLFW for the application layer.

## Design
GLFW callback-driven event model: glfwSetWindowCloseCallback, glfwSetWindowFocusCallback, glfwSetWindowSizeCallback, glfwSetWindowIconifyCallback. Maps GLFW callbacks to FGenericPlatformApplicationMisc interface methods (OnWindowShow, OnWindowFocus, OnWindowResize, OnWindowClose). Platform-agnostic abstraction layer above raw GLFW, supporting the same interface contract as Windows and Linux implementations. GLFW error callback also captured for diagnostics. The design assumes a single GLFWwindow* is the primary application window.

## Flow
GLFW callback → map to platform method → OnWindowShow/OnWindowFocus/OnWindowResize/OnWindowClose → application callback. GLFW error callback also captured for diagnostics. During app initialization, GLFW callbacks are registered; during shutdown, callbacks are deregistered. The mapping is many-to-one: multiple GLFW events may contribute to a single abstract lifecycle method call.

## Integration
Source/Runtime/Application/Public/HAL/Generic/ (interface contract), Source/Runtime/Application/Private/HAL/GLFW/ (private implementation), Source/Programs/ShaderToy/ (GLFW-based app with ImGui dockspace uses GLFW window events)