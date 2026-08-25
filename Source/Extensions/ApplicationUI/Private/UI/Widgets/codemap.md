# Source/Extensions/ApplicationUI/Private/UI/Widgets/

## Responsibility
Concrete ImGui widget implementations private to the ApplicationUI extension. Provides the actual widget logic that runs inside the ImGui dockspace, behind the public UI service API. These are the implementation types that the public widget registry resolves to, with the private prefix distinguishing them from the public API mirror in Source/Extensions/ApplicationUI/Public/UI/Widgets/.

## Design
FFileDialogWidget - file import with texture loading and sampler setup. FLogViewerWidget - scrollable log display with filtering and search. FMemoryUsageWidget - heap snapshot with category breakdown. Each widget handles its own ImGui::Begin/End, interaction, and state persistence. RTTI-enabled for registry reflection. Private prefix distinguishes from public API widgets in Source/Extensions/ApplicationUI/Public/UI/Widgets/. Each widget manages its own settings persistence via ImGuiSettingsHandler across sessions.

## Flow
Application requests widget by ID → private registry resolves concrete type → widget ImGui::Begin/End → user interaction → widget logic → state update → ImGui::End → next frame re-renders with updated state. Settings persisted via ImGuiSettingsHandler across sessions. The widget logic is driven by the application's duty cycle, which calls ImGui::NewFrame() before widget rendering.

## Integration
Source/Extensions/ApplicationUI/Private/UI/ (service layer), Source/Extensions/ApplicationUI/Public/UI/Widgets/ (public API mirror), Source/Programs/ShaderToy/ (consumer: ImportTexture, FileDialog, LogViewer, MemoryUsage)