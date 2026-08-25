# Source/Runtime/Application/Public/Input/

## Responsibility
This folder exposes the abstracted input service to application-layer code. It defines input action types, device state interfaces, and the mapping layer that decouples raw device events from application logic. The public API allows applications to query action states (pressed, released, held) and axis values without depending on concrete device implementations, supporting hot-plug device handling and composite action evaluation during the duty cycle.

## Design
FInputAction represents a bound action with a semantic name (e.g., "MoveForward", "Jump"), a type (boolean or analog axis), and a state tracker. FInputDevice is the abstract base for concrete device implementations. Action mappings are defined in registration tables that map device events to action names, supporting per-action dead zones and normalization curves. TActionMap holds the collection of registered actions, while TActionState tracks per-action press/release/hold semantics. RTTI_DECL enables reflection and editor integration. The design supports composite actions via FActionChord, which evaluates multiple primitive actions combined with logical operators.

## Flow
The application queries the input service for action state during the duty cycle. The input service reads device state from concrete implementations (see Source/Runtime/Application/Public/Input/Device/), evaluates action bindings, and returns the current state (true/false for boolean, float value for axis). Asynchronous callbacks deliver state changes between polling cycles. Hot-plug events are processed during device enumeration, and action mappings are re-evaluated when the device list changes. The flow is: device poll → state fill → action binding evaluation → result delivery to caller.

## Integration
Source/Runtime/Application/Public/Input/Device/ (concrete device implementations), Source/Runtime/Application/Private/Input/ (private service details), Source/Runtime/Application/Public/Window/ (window-linked input context for keyboard focus and mouse capture), Source/Programs/ShaderToy/, Source/Programs/VoxelCube/, Source/Programs/WindowTest/ (consumers of input action service)