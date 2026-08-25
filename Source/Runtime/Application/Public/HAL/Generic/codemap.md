# Source/Runtime/Application/Public/HAL/Generic/

## Responsibility
Generic/platform-agnostic base defining the FGenericPlatformApplicationMisc contract. All platform-specific HAL implementations must honor this interface contract; pure virtual methods enforce override in concrete subclasses. This folder provides the abstract base that the application layer depends on, ensuring a uniform contract across all supported platforms while hiding the details of OS-specific app lifecycle management.

## Design
FGenericPlatformApplicationMisc declares pure virtual methods for app lifecycle events with no default implementation. Concrete subclasses (FWindowsPlatformApplicationMisc, FLinuxPlatformApplicationMisc) provide overrides. Pure virtual = delete ensures every platform implements the full contract. The class supports EXTERN_LOG_CATEGORY for diagnostics and is consumed via interface pointer (FGenericPlatformApplicationMisc*) to polymorphic call. No method bodies exist in the base — each pure virtual must be overridden in a concrete translation unit.

## Flow
Pure virtual interface — no flow through this layer alone. Concrete subclasses define the execution path. Used via interface pointer to polymorphic call from the application layer. The base class serves as the contract that the application, HAL platform implementations, and private details all honor. Static type checking ensures the full interface contract is satisfied by any concrete subclass.

## Integration
Source/Runtime/Application/Public/HAL/ (parent umbrella), Source/Runtime/Application/Public/HAL/Windows/, Source/Runtime/Application/Public/HAL/Linux/, Source/Runtime/Application/Private/HAL/ (private implementation details)