# Source/Extensions/ApplicationUI/Private/

## Responsibility
Private application UI extension details containing implementation behind the public module API, module lifecycle management, and integration hooks that the public module interface conceals from application callers. Handles ImGui context management, widget registry, and event routing that the public API (Source/Extensions/ApplicationUI/Public/) conceals from direct application use.

## Design
Private module details support IModuleInterface lifecycle (Start/Shutdown/DutyCycle/ReleaseMemory). ImGui context ownership and lifecycle. Settings persistence handler (ImGuiSettingsHandler) for per-widget state. Event dispatch to registered widget callbacks. Thread-safe access for duty-cycle polling. Hides concrete widget types behind abstract factory interface. The private module is the single owner of the ImGui context and manages its entire lifecycle.

## Flow
Module Start() → private init → ImGui context creation → widget library load → Module DutyCycle() → NewFrame → widget rendering → Shutdown → cleanup → ReleaseMemory → resource release. Settings loaded on context creation, saved on shutdown. The private module dispatches events to registered widget callbacks during each duty cycle, translating ImGui events into application-meaningful actions.

## Integration
Source/Extensions/ApplicationUI/Public/ (public module API), Source/Extensions/ApplicationUI/Private/UI/ (concrete widget implementations and registry), Source/Runtime/Application/Public/Window/ (dockspace host for ImGui context)