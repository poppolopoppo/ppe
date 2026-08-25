## Responsibility
This folder implements public Vulkan pipeline compiler components, exposing the Vulkan backend's primary API surface for shader compilation and pipeline generation. It provides type-safe access to Vulkan pipeline compilation capabilities without exposing private implementation details.

## Design
Key patterns publicly expose `IPipelineCompiler` as the compiler abstraction interface and `FVulkanPipelineCompiler` as the concrete Vulkan backend type. `FVulkanSpirvCompiler` public operations handle GLSL → SPIR-V compilation pipeline. Pipeline descriptor types (`FMeshPipelineDesc`, `FGraphicsPipelineDesc`, `FComputePipelineDesc`, `FRayTracingPipelineDesc`) are available for pipeline configuration. `FMeshPipelineDesc` is the primary descriptor type for graphics pipeline compilation. Shader module creation with `VkShaderModule` caching and format negotiation via `HighestPriorityShaderFormat_()` are publicly accessible. Debug/profiling/trace support through `FRayTracingDebuggableShaderModule` is also exposed.

## Flow
Public consumers call `IPipelineCompiler::Compile()` with pipeline descriptor and format configuration, dispatching to the Vulkan backend compilation flow. The GLSL → SPIR-V → shader module → pipeline layout sequence executes internally. Reflection data is made available through public interfaces. Output shader modules and pipeline objects are registered with `FBuildEnvironment::AddFiles()` for downstream consumption. Clean operations remove public pipeline artifacts.

## Integration
Integrates with `Source/ContentPipeline/PipelineCompiler/Private/Vulkan/Pipeline/` for private Vulkan pipeline compilation details, `Source/ContentPipeline/BuildGraph/Public/` for build graph orchestration, `Source/ContentPipeline/MeshBuilder/Public/Mesh/` for mesh data input, and `Source/ContentPipeline/Texture/Public/Texture` for texture-associated pipeline compilation. Also connects to `*.cpp` for test validation.