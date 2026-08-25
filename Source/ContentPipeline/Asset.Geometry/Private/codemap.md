## Responsibility
This folder implements private geometry content pipeline components for Asset.Geometry, handling the low-level processing, compression, and intermediate representation generation that supports the public geometry pipeline. It operates behind the scenes, producing build artifacts consumed by public-facing modules while keeping implementation details encapsulated.

## Design
Key patterns rely on `FBuildGraph` for DAG-based build orchestration, `FBuildNode` as the base type for all geometry pipeline nodes, and `FPipelineContext` variants (`FScanContext`, `FBuildContext`, `FCleanContext`) to manage the three-phase pipeline lifecycle. The design uses `FContentPipelineNode` as the RTTI-enabled base for geometry-specific nodes, with `TContentToolchain` coordinating importer and processor execution. Memory layout follows the repo's ref-counted smart pointer conventions (`TPtrRef`, `U*`) for deterministic cleanup.

## Flow
Scan phase invokes `FScanContext::Scan()` on each geometry node, which calls `node.Scan()` to stat source files and register dependencies under `FBuildEnvironment`. Build phase triggers `FBuildContext::Build()`, which calls `node.Import()` followed by `node.Process()`. Import resolves the appropriate `IContentImporter` via dynamic cast; Process executes type-specific geometry processing. Clean phase calls `FCleanContext::Clean()` on each node to remove generated intermediate files and dependency tracking artifacts.

## Integration
Integrates with `Source/ContentPipeline/Asset.Geometry/Public/` for public geometry symbol exposure, `Source/ContentPipeline/MeshBuilder/Private/Mesh/` for source mesh data, and `Source/ContentPipeline/PipelineCompiler/Private/Vulkan/Pipeline/` for Vulkan geometry pipeline compilation. Also connects to `Source/ContentPipeline/BuildGraph/Private/` for graph execution and `Source/ContentPipeline/Asset.Texture/Private/` for associated texture generation from geometry meshes.