# Source/Extensions/ApplicationUI/Public/UI/

## Responsibility
Public UI service layer exposing ImGui-based widget interfaces to application code. Handles widget creation, layout, and event routing through the ImGui dockspace framework. Provides the bridge between application logic and the ImGui immediate-mode GUI, allowing applications to embed ready-to-use UI components without dealing with ImGui internals directly.

## Design
Widget base class pattern with typed derived classes (FFileDialogWidget, FLogViewerWidget, FMemoryUsageWidget). ImGui dockspace as root window. Widget registry for lookup by ID. RTTI-enabled for reflection. Uses ImGuiSettingsHandler for per-widget state persistence. The public API defines the interface that applications use; concrete implementations live in the private folder.

## Flow
Application requests widget by ID → registry lookup → widget creation → ImGui::Begin/End frame → widget renders UI → event callbacks routed back to application → ImGui::End → next frame widget re-renders with updated state. Widget state persisted via ImGuiSettingsHandler across sessions. The flow is driven by the application's duty cycle, which calls ImGui::NewFrame() before widget rendering.

## Integration
Source/Extensions/ApplicationUI/Private/UI/ (service layer), Source/Extensions/ApplicationUI/Private/UI/Widgets/ (concrete widget implementations), Source/Runtime/Application/Public/Window/ (dockspace host), Source/Programs/ShaderToy/ (consumer of widget services: ImportTexture, FileDialog, LogViewer, MemoryUsage)