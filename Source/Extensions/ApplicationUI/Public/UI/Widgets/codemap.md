# Source/Extensions/ApplicationUI/Public/UI/Widgets/

## Responsibility
Concrete ImGui widget implementations for the application UI service. Provides ready-to-use widget components (import texture, file dialog, log viewer, memory usage) that applications can embed in their ImGui dockspace. These are the public API types that the widget registry resolves to when applications request widgets by ID.

## Design
FFileDialogWidget - file import with texture loading and sampler setup. FLogViewerWidget - scrollable log display with filtering and search. FMemoryUsageWidget - heap snapshot with category breakdown. Each widget implements default layout, interaction logic, and settings persistence via ImGuiSettingsHandler. RTTI-enabled for reflection and registry lookup. Widget size and position configurable per-session. The public API widgets are thin wrappers that delegate to the private implementations in Source/Extensions/ApplicationUI/Private/UI/Widgets/.

## Flow
Application requests widget by ID → registry resolves concrete type → widget ImGui::Begin/End frame → user interaction (button clicks, file drops, text input) → widget state updated → ImGui::End → next frame widget re-renders with updated state. Settings persisted via ImGuiSettingsHandler across sessions. Each widget handles its own interaction logic within the ImGui immediate-mode paradigm.

## Integration
Source/Extensions/ApplicationUI/Private/UI/ (service layer behind public API), Source/Programs/ShaderToy/ (widget consumer: ImportTexture, FileDialog, LogViewer, MemoryUsage), Source/Extensions/ApplicationUI/Public/UI/ (public API mirror)