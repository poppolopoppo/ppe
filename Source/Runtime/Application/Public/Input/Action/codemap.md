# Source/Runtime/Application/Public/Input/Action/

## Responsibility
Input action definitions and mappings that decouple device state from application logic. Supports composite actions, chords, and value types for analog control. This folder provides the type definitions and mapping tables that allow applications to work in terms of semantic actions (e.g., "Jump", "Fire") rather than raw device events, enabling input remapping and controller abstraction.

## Design
FInputAction holds an action name, type (boolean or axis), binding list, and state tracker. FActionChord represents multi-key combinations evaluated with logical AND/OR semantics. Action mappings are registered in global tables keyed by action name, supporting per-action dead zones and normalization curves. RTTI_DECL enables reflection and editor integration. The mapping system supports dynamic remapping at runtime, allowing user-configurable control schemes without recompilation. Value actions accumulate axis input over the frame, providing smooth analog control.

## Flow
Action definition → binding registration during startup → during input evaluation, bindings are evaluated against current device state (see Source/Runtime/Application/Public/Input/Device/) → state computed → result delivered to caller. Boolean actions check press/release/hold state; value actions return accumulated axis input normalized by dead zone and curve. Composite actions (FActionChord) evaluate sub-actions and combine results with logical operators. Results are delivered to the application via the input service query interface.

## Integration
Source/Runtime/Application/Public/Input/Device/ (provides raw device state for binding evaluation), Source/Programs/ShaderToy/, Source/Programs/VoxelCube/, Source/Programs/WindowTest/ (consume input actions for movement, shooting, and UI interaction)