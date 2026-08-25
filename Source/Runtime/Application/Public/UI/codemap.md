# Source/Runtime/Application/Public/UI/

## Responsibility
Public user interface layer for the application, providing the ImGui-based UI service that windowed applications integrate into their duty cycle. This folder exposes the ImGui context management, dockspace setup, and widget library loading through a clean public API that concrete applications (such as ShaderToy) depend on for their in-game UI. The design hides private widget implementation details behind the public registry interface, allowing applications to request widgets by ID without knowing concrete types.

## Design
ImGui context hooks for extensibility, multi-viewport support (experimental), and ImGuiSettingsHandler for persistence of window positions and settings across sessions. The widget library is organized by functional groups (import, file dialog, log, memory). The public API defines widget base classes and a registry for lookup by ID. RTTI-enabled for reflection and editor integration. The design conceals private widget implementations (prefixed FPrivateImGui) behind the public API mirror, allowing clean separation between the public interface and private implementation. All widgets use ImGui::Begin/End framing within the dockspace root window.

## Flow
Application requests widget by ID → registry lookup → widget creation → ImGui::Begin/End frame → widget renders UI → event callbacks routed back to application → ImGui::End → next frame widget re-renders with updated state. Settings persisted via ImGuiSettingsHandler across sessions. Each frame, the duty cycle calls ImGui::NewFrame() before widget rendering. The dockspace remains available for additional widgets or program-specific ImGui code between widget renders.

## Integration
Source/Runtime/Application/Public/Window/ (ImGui dockspace host for windowed apps), Source/Runtime/Application/Public/HAL/ (platform behind ImGui), Source/Programs/ShaderToy/ (ImGui UI consumer: dockspace, widgets, log viewer, memory usage), Source/Extensions/ApplicationUI/Private/UI/ (private widget implementations and registry), Source/Extensions/ApplicationUI/Private/UI/Widgets/ (concrete private widget types), Source/Extensions/ApplicationUI/Public/UI/Widgets/ (public API mirror widget types)