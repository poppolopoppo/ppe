## Responsibility
This folder handles interop operations for Asset.Geometry private pipeline components, enabling seamless integration between the content pipeline and external systems that require geometry data exposure. It manages type translation, format bridging, and platform-specific geometry metadata required by downstream consumers such as renderers and editor tools.

## Design
Key patterns include the use of `FBuildGraph` for dependency tracking of interop artifacts, `FContentImporterContext` for source-to-destination mapping, and `TContentImporter<_Import>` templated importers for type-safe geometry serialization. Architectural decisions favor RTTI-enabled base classes (`FMetaObject`) so that interop nodes can be dynamically dispatched during the Scan phase, while specialization constants are handled via `FMeshPipelineDesc`-derived descriptors for Vulkan geometry pipeline configuration.

## Flow
During Scan, `FBuildGraph::AppendNodes()` discovers interop geometry nodes that reference source mesh files under `Source/ContentPipeline/Asset.Geometry/`. Each node's `Import()` method resolves via dynamic cast to the appropriate `TContentImporter`, producing intermediate geometry artifacts stored in private intermediate directories. Build phase executes `node.Process()`, which triggers `FContentProcessor::Process(ctx, dst)` to convert source formats into the target interop representation. Clean phase runs `node.Clean()` to remove generated interop files.

## Integration
Connects to `Source/ContentPipeline/Asset.Geometry/Public/` for public-facing geometry symbols, `Source/ContentPipeline/MeshBuilder/Private/Mesh/` for source mesh importing, and `Source/ContentPipeline/PipelineCompiler/Private/Vulkan/Pipeline/` for downstream Vulkan geometry pipeline compilation. Downstream dependents include `Source/ContentPipeline/Texture/Private/Texture/` for mesh-associated texture generation and `Source/ContentPipeline/BuildGraph/Private/` for graph execution orchestration.