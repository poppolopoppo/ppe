# Source/Runtime/Application/Private/Input/Action/

## Responsibility
Private input action definitions and mappings behind the public API. Decouples device state from application logic at the implementation level. Supports composite actions, chords, and value types for analog control, with the design concealing concrete action types and binding tables behind the public FInputAction interface. This folder provides the implementation details that the public API (Source/Runtime/Application/Public/Input/Action/) conceals from application callers.

## Design
FPrivateInputAction holds an action name, type (boolean or axis), binding list, and state tracker. FActionChord represents multi-key combinations evaluated with logical AND/OR semantics. Action mappings are registered in global tables keyed by action name, supporting per-action dead zones and normalization curves. RTTI_DECL enables reflection and editor integration. The design supports composite actions and dynamic remapping at runtime, allowing user-configurable control schemes without recompilation. Value actions accumulate axis input over the frame, providing smooth analog control. The private prefix distinguishes these implementation types from the public mirror classes.

## Flow
Action definition → binding registration during startup → during input evaluation, bindings are evaluated against current device state (see Source/Runtime/Application/Private/Input/Device/) → state computed → result delivered to caller. Boolean actions check press/release/hold state; value actions return accumulated axis input normalized by dead zone and curve. Composite actions (FActionChord) evaluate sub-actions and combine results with logical operators. Results are delivered to the application via the input service query interface.

## Integration
Source/Runtime/Application/Private/Input/Device/ (provides raw device state for binding evaluation), Source/Runtime/Application/Public/Input/Action/ (public API mirror), Source/Programs/ShaderToy/, Source/Programs/VoxelCube/, Source/Programs/WindowTest/ (consume input actions for movement, shooting, and UI interaction)