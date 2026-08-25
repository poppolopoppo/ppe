# Source/Extensions/ApplicationUI/Private/UI/

## Responsibility
Private UI service implementation behind the public UI API. Handles ImGui context management, widget registry, and event routing that the public UI service conceals from application callers. This is the implementation layer that the public API (Source/Extensions/ApplicationUI/Public/UI/) reflects from, hiding concrete widget types and registry details behind a clean interface.

## Design
Private widget registry maps string IDs to factory functions. ImGui context ownership and lifecycle. Settings persistence handler (ImGuiSettingsHandler) for per-widget state. Event dispatch to registered widget callbacks. Thread-safe access for duty-cycle polling. Hides concrete widget types behind abstract factory interface. The registry is the central lookup mechanism that translates string-based widget requests into concrete widget instantiation.

## Flow
Registry lookup → widget factory instantiation → ImGui::Begin/End → widget event callbacks → state update → next frame re-render. Settings loaded on context creation, saved on shutdown. The private service processes input service queries from application code, translating them into concrete device reads and action binding evaluations during the duty cycle.

## Integration
Source/Extensions/ApplicationUI/Private/UI/Widgets/ (concrete widget implementations), Source/Extensions/ApplicationUI/Public/UI/ (public API mirror), Source/Runtime/Application/Public/Window/ (dockspace host for ImGui context), Source/Programs/ShaderToy/ (consumer of widget services)