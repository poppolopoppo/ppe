# Source/Runtime/Application/Private/HAL/

## Responsibility
Private HAL implementation details supporting the public FGenericPlatformApplicationMisc interface. Contains platform-specific concrete implementations hidden behind the public abstraction boundary. This folder provides the concrete realization of the generic app lifecycle contract while hiding the details of message hook installation, window procedure management, and OS event pipeline from the public header consumers.

## Design
Private HAL classes implement FGenericPlatformApplicationMisc pure virtual methods. Details of message hook installation, window procedure subclassing, and OS event pipeline are hidden from the public header. Pimpl idiom may be used in some contexts to further conceal implementation. The public header (Source/Runtime/Application/Public/HAL/) declares FGenericPlatformApplicationMisc; the private details satisfy the contract. Each platform has a private counterpart that fulfills the interface.

## Flow
Platform-specific initialization → hook installation → event loop integration → app lifecycle event delivery → cleanup on shutdown. Details vary by platform but the contract is uniform via FGenericPlatformApplicationMisc interface. The private classes are loaded and managed through the same service location as the public counterparts, ensuring the application sees a consistent interface regardless of which detail class is active.

## Integration
Source/Runtime/Application/Public/HAL/ (public interface), Source/Runtime/Application/Private/HAL/Windows/, Source/Runtime/Application/Private/HAL/Linux/, Source/Runtime/Application/Public/HAL/Generic/ (contract)