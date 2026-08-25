# Source/Runtime/Application/Private/HAL/GLFW/

## Responsibility
GLFW-based private HAL implementation wrapping window management and event polling behind the FGenericPlatformApplicationMisc interface contract. Conceals the GLFWwindow* handle and callback details from the public header, providing a platform-agnostic abstraction layer above raw GLFW for the application layer.

## Design
GLFW callback-driven event model mapped to FGenericPlatformApplicationMisc interface methods: glfwSetWindowCloseCallback, glfwSetWindowFocusCallback, glfwSetWindowSizeCallback, glfwSetWindowIconifyCallback. Platform-agnostic abstraction layer above raw GLFW, supporting the same interface contract as Windows and Linux implementations. GLFW error callback also captured for diagnostics. The design assumes a single GLFWwindow* is the primary application window, with the handle concealed via the pimpl pattern.

## Flow
GLFW callback → private handler → FGenericPlatformApplicationMisc interface methods → OnWindowShow/OnWindowFocus/OnWindowResize/OnWindowClose → application callback. GLFW error callback also captured for diagnostics. During app initialization, GLFW callbacks are registered; during shutdown, callbacks are deregistered. The mapping is many-to-one: multiple GLFW events may contribute to a single abstract lifecycle method call.

## Integration
Source/Runtime/Application/Public/HAL/Generic/ (interface contract), Source/Runtime/Application/Public/HAL/GLFW/ (public mirror), Source/Programs/ShaderToy/ (GLFW-based app with ImGui dockspace uses GLFW window events)