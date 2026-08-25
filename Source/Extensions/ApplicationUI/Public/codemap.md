# Source/Extensions/ApplicationUI/Public/

## Responsibility
Public application UI extension module providing the ImGui-based UI service and widget library that windowed applications can integrate into their duty cycle. Handles ImGui context creation, dockspace setup, and widget library loading. This folder is the public API that application code and concrete programs (such as ShaderToy) depend on for ImGui integration, exposing the module interface, context management, and the widget registry. The module implements IModuleInterface with StartShutdownDutyCycleReleaseMemory lifecycle and RTTI_MODULE_DECL for registration.

## Design
ImGui context hooks for extensibility, multi-viewport support (experimental), and ImGuiSettingsHandler for persistence of window positions and settings. The widget library is organized by functional groups (import, file dialog, log, memory). The public API conceals private implementation details behind a clean interface, allowing applications to request widgets by ID and receive concrete types through the registry. Widget base classes follow a typed derived pattern with RTTI-enabled reflection for editor integration and runtime type checking.

## Flow
Module Start() → ImGui context creation → dockspace setup → widget library loaded → Module DutyCycle() → ImGui::NewFrame() → widget rendering → Module Shutdown() → ImGui context destruction. Settings persistence across sessions via ImGuiSettingsHandler. Each frame, the duty cycle calls NewFrame, widgets render their UI, and the dockspace remains available for additional widgets or program-specific ImGui code. Widget state is updated based on input service queries and persisted between sessions.

## Integration
Source/Runtime/Application/Public/Window/ (ImGui dockspace host for windowed apps), Source/Runtime/Application/Public/HAL/ (platform behind ImGui), Source/Programs/ShaderToy/ (ImGui UI consumer: dockspace, widgets, log viewer, memory usage), Source/Extensions/ApplicationUI/Private/ (private module details and widget implementations), Source/Extensions/ApplicationUI/Public/UI/ (public widget API), Source/Extensions/ApplicationUI/Public/UI/Widgets/ (concrete widget implementations), Source/Extensions/ApplicationUI/Private/UI/ (service layer behind public API)