# Source/Runtime/Application/Public/Input/Device/

## Responsibility
Concrete input device implementation tracking keyboard, mouse, and gamepad state. Provides raw device data and derived action inputs to the application layer. Each device type wraps an OS-specific handle and maintains frame-accurate state buffers, supporting dead zones, axis normalization, and device enumeration. The public API ensures a consistent interface across platforms while hiding OS-specific details behind abstracted state accessors.

## Design
FKeyboardDevice wraps an OS keyboard handle and maintains a 256-key state buffer with timestamp tracking. FMouseDevice tracks button states (left, right, middle, side), X/Y axis values with dead-zone application, and scroll delta. FGamepadDevice encapsulates a gamepad handle, implements state snapshot patterns for consistent read views across polling cycles, and supports axis normalization and dead zones per stick. All device classes use TPtrRef for ownership by the input service and expose const reference accessors for read-only consumption. Device enumeration and hot-plug handling are managed at this layer.

## Flow
OS device query → FillState() populates the frame-accurate state buffer → action mapping evaluation (see Source/Runtime/Application/Public/Input/Action/) → application consumption via const reference accessors. State changes are detected via timestamp comparison against the previous frame. Device loss or disconnection triggers re-enumeration callbacks. Between frames, state is preserved unless explicitly cleared, ensuring stable input evaluation during the duty cycle.

## Integration
Source/Runtime/Application/Public/Input/ (action mappings and service), Source/Runtime/Application/Private/Input/Device/ (private implementation), Source/Runtime/Application/Public/HAL/ (OS device hooks and underlying handles)