# Source/Runtime/Application/Private/HAL/Generic/

## Responsibility
Generic/platform-agnostic base defining the private HAL contract for FGenericPlatformApplicationMisc. All platform-specific private HAL implementations must honor this interface, ensuring a uniform contract across all supported platforms while hiding the details of message hook installation, window procedure management, and OS event pipeline from the public header consumers.

## Design
Pure virtual interface declaring FGenericPlatformApplicationMisc methods with no default implementation. Concrete subclasses (FWindowsPlatformApplicationMisc, FLinuxPlatformApplicationMisc) provide overrides. Pure virtual = delete ensures every platform implements the full contract. The base class serves as the contract that the application, public HAL, and private details all honor. No method bodies exist in the base — each pure virtual must be overridden in a concrete translation unit. Supports EXTERN_LOG_CATEGORY for diagnostics.

## Flow
Pure virtual interface — no flow through this layer alone. Concrete subclasses define the execution path. Used via interface pointer to polymorphic call from the application layer. The base class serves as the contract that the application layer depends on without knowing the concrete platform details.

## Integration
Source/Runtime/Application/Private/HAL/ (parent umbrella), Source/Runtime/Application/Private/HAL/Windows/, Source/Runtime/Application/Private/HAL/Linux/, Source/Runtime/Application/Public/HAL/Generic/ (public contract)