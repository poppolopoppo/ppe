# Source/Runtime/Application/Private/Input/

## Responsibility
Private input service implementation supporting the abstract input service interface. Handles device enumeration, state tracking, and action binding evaluation behind the public input API boundary. This folder contains the concrete service ownership, device management, and the implementation details that the public API (Source/Runtime/Application/Public/Input/) conceals from application callers.

## Design
Private input service owns concrete device implementations (FKeyboardDevice, FMouseDevice, FGamepadDevice). Maintains device list, handles hot-plug events, provides action state evaluation via FInputAction bindings. Thread-safe state access for duty-cycle polling. Uses TPtrRef for device ownership. The public API mirror (Source/Runtime/Application/Public/Input/) exposes only query methods; the private service manages the full device lifecycle including enumeration, configuration, and hot-plug handling.

## Flow
Device enumeration on startup → state update per duty cycle → action binding evaluation → application receives input state queries. Hot-plug handling during runtime adds/removes devices from the tracked list. State is cleared between frames unless persisted. The service processes input service queries from application code, translating them into concrete device reads and action binding evaluations.

## Integration
Source/Runtime/Application/Public/Input/ (public API), Source/Runtime/Application/Private/Input/Device/ (concrete devices), Source/Extensions/ApplicationUI/Public/UI/ (ImGui input integration consumes private input service state)