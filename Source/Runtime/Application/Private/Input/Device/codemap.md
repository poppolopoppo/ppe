# Source/Runtime/Application/Private/Input/Device/

## Responsibility
Private concrete device implementations behind the public input API. Wraps OS-specific device handles and provides consistent state tracking across platforms. These are the real device objects owned by the private input service, supporting the same device types as the public mirror but with full implementation details including configuration, calibration, and hot-plug management hidden behind the public accessor interface.

## Design
FPrivateKeyboardDevice, FPrivateMouseDevice, FPrivateGamepadDevice each encapsulate OS handle and maintain frame-accurate state buffers. Input state snapshot pattern provides consistent read view across polling cycles. Dead zones, normalization, and device configuration stored per-device. RTTI-enabled for reflection and editor integration. The private prefix distinguishes these implementation types from the public mirror classes in Source/Runtime/Application/Public/Input/Device/. All classes use TPtrRef for ownership and provide const reference accessors for read-only consumption by the input service.

## Flow
OS device query → snapshot fill → action mapping → application consumption. State delta from previous frame computed via timestamp comparison. Device loss recovery through re-enumeration. Between frames, state is preserved unless explicitly cleared, ensuring stable input evaluation during the duty cycle. The snapshot pattern ensures that application code reading device state gets a consistent view even as OS events arrive asynchronously.

## Integration
Source/Runtime/Application/Private/Input/ (service owner), Source/Runtime/Application/Public/Input/Device/ (public mirror), Source/Runtime/Application/Public/HAL/ (OS underneath providing device handles)