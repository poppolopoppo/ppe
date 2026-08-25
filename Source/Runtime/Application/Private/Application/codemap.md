# Source/Runtime/Application/Private/Application/

## Responsibility
Private application implementation details supporting the modular domain-driven component lifecycle. Contains the non-public details that support the public FApplicationBase/FApplicationWindow abstraction, including service registration, module activation ordering, and the hidden internals of the modular domain integration that the public API conceals from application authors.

## Design
Private application module supports FModularDomain-driven component activation. Supports service registration (IInputService, IRHIService, IWindowState), reference-counted module ownership via TPtrRef, and startup/shutdown ordering. The modular domain drives which modules are active and their initialization order. Hides implementation details behind the FApplicationBase public interface, exposing only the service acquisition methods and lifecycle hooks that concrete applications need. Module Start()/Shutdown() calls are sequenced by the modular domain.

## Flow
App startup → FModularDomain::Initialize → private app init → service registration → module Start() calls in defined order → duty cycle → module Shutdown() calls in reverse order → cleanup → FApplicationWindow owns the public façade that application code interacts with. The private module hides the modular domain graph, service dependency resolution, and module activation ordering from the public API.

## Integration
Source/Runtime/Application/Public/Application/ (public façade), Source/Runtime/Application/Private/Input/ (input service implementation), Source/Runtime/Application/Private/Window/ (private window details), Source/Extensions/ApplicationUI/Public/ (module integration and ImGui service)