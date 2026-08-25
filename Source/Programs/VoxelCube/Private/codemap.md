# Source/Programs/VoxelCube/Private/

## Responsibility
This directory contains the private implementation of the VoxelCube voxel rendering demo, including the mesh generation, camera control, graphics pipeline, and rendering loop. These components are the operational core of the voxel demo.

## Design
The private implementation centers on the voxel mesh generation function, which creates a `FGenericMesh` instance with the specified voxel topology. The `FFreeLookCameraController` is implemented with keyboard and mouse input handling, providing smooth camera rotation and translation. The graphics pipeline is created with the desired rendering mode (point, line, or triangle) and the appropriate shader program. Uniform buffers are allocated and filled with the camera transform, light direction, and material parameters each frame. The render loop binds the pipeline, updates the uniform buffer, and issues a draw call to render the voxel mesh. Normal recomputation is implemented as a compute shader pass or a CPU-side pass, depending on the configuration. The module also handles window resize events, recreating the graphics pipeline and swap chain resources.

## Flow
The rendering flow begins when the application initializes the voxel mesh and camera controller. Each frame, the camera controller updates the camera transform based on input. The voxel mesh bounds are recalculated if the camera transform has changed. The graphics pipeline is bound, the uniform buffer is updated with the camera and light parameters, and a draw call is issued to render the voxel mesh. The result is presented to the screen via the swap chain. If normal recomputation is enabled, an additional compute shader pass recomputes face normals, and the results are stored in a separate buffer for use in the shading pass.

## Integration
The private implementation integrates with `Source/Runtime/Application/` (`Source/Runtime/Application`) for the `FApplicationWindow` base class and modular domain coordination. The mesh generation interfaces with `Source/ContentPipeline/Asset/` (`Source/ContentPipeline/Asset/codemap.md`) for imported voxel data. The graphics pipeline uses `Source/Runtime/RHI/` frame graph resources for render target and depth buffer management. The module shares the `FModularDomain` with all other programs, and the lifecycle is coordinated through the modular domain's duty cycle.