# Source/Runtime/Application/Public/Viewport/

## Responsibility
Viewport management and camera abstraction for the application layer. Defines the viewport rectangle, camera controller interface, and coordinate space conversion between world and screen space. This folder provides the FViewport class and related types that the rendering system uses to determine what portion of the scene is visible and how the camera transforms world coordinates to screen coordinates. Part of the public application layer that rendering and UI systems depend on.

## Design
FViewport class owns rectangle (x, y, width, height) and camera controller (FFreeLookCameraController, FViewportClient). Projection matrix computation, aspect ratio handling, and view offset management. Supports multiple viewport instances for split-screen or multi-monitor configurations. Camera controller interface provides view and projection matrices to the rendering system. Uses EXTERN_LOG_CATEGORY for diagnostics. The viewport rectangle defines the scissor and render target region for the graphics pipeline. Aspect ratio is computed from width/height, and is updated on resize. RTTI-enabled for reflection.

## Flow
FViewport::SetCameraController() → controller provides view/projection matrices → rendering system uses matrices for shader uniform upload → viewport rectangle defines scissor and render target region → OnResize updates projection when dimensions change → camera controller handles input for camera movement → world-to-screen and screen-to-world coordinate conversions supported via camera inverse transform. During the duty cycle, the application updates the viewport based on window resize events.

## Integration
Source/Runtime/Application/Public/Window/ (window size source for viewport dimensions), Source/Runtime/Application/Private/Application/ (camera controller ownership), Source/Programs/WindowTest/Private/Drawing/ (rendering consumers using viewport matrices), Source/Extensions/ApplicationUI/Public/UI/Widgets/ (widgets may display viewport info or camera-relative UI), Source/ContentPipeline/ (content pipeline modules that may use viewport definitions)