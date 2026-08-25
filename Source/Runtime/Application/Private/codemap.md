# Source/Runtime/Application/Private/

## Responsibility
Private application implementation details supporting the modular domain-driven component lifecycle and serving as the umbrella folder for all private implementation sub-layers. Contains the non-public details that support the public FApplicationBase/FApplicationWindow abstraction, including service registration, module activation ordering, and the hidden internals of the modular domain integration that the public API conceals from application authors. This folder groups private implementation details for input, HAL, window, and viewport systems.

## Design
Private application module supports FModularDomain-driven component activation. Supports service registration (IInputService, IRHIService, IWindowState), reference-counted module ownership via TPtrRef, and startup/shutdown ordering. The modular domain drives which modules are active and their initialization order. Hides implementation details behind the FApplicationBase public interface, exposing only the service acquisition methods and lifecycle hooks that concrete applications need. Module Start()/Shutdown() calls are sequenced by the modular domain. Private details are organized into sub-folders (Input, HAL, Window, Viewport) each with their own scope and responsibility.

## Flow
App startup → FModularDomain::Initialize → private app init → service registration → module Start() calls in defined order → duty cycle → module Shutdown() calls in reverse order → cleanup → FApplicationWindow owns the public façade that application code interacts with. The private module hides the modular domain graph, service dependency resolution, and module activation ordering from the public API. Each sub-folder (Input, HAL, etc.) contributes its own flow to the overall application lifecycle.

## Integration
Source/Runtime/Application/Public/Application/ (public façade), Source/Runtime/Application/Private/Input/ (input service implementation), Source/Runtime/Application/Private/HAL/ (private HAL implementations), Source/Runtime/Application/Private/Window/ (private window details), Source/Runtime/Application/Private/Viewport/ (private viewport details), Source/Extensions/ApplicationUI/Public/ (module integration and ImGui service), Source/Extensions/ApplicationUI/Private/ (private UI details)