# PPE — Codemap Atlas

## Responsibility
PPE is a C++ game engine built on a modular architecture. This file is the **top-level index** for the generated codemap. The full hierarchical map lives at [`Source/codemap.md`](Source/codemap.md); this root atlas orients you and points to the right subtree.

The mapped source tree is `Source/`, organized into five top-level module groups. Three trees are **intentionally excluded** from the codemap (per project scope): `Source/External/` (third-party vendored code), `Source/Legacy/` (deprecated code), and `Build/` (build-system tooling). Do not expect codemaps under those paths.

## Design
The engine uses a **module system** where each subsystem declares its interface in a `*-module.json` file consumed by a Go-based build system. Modules follow a strict lifecycle — `Start → PostStart → DutyCycle → PreShutdown → Shutdown` — and register services through a type-safe registry, `FModularServices` (parent-child scoped for dependency injection). Cross-DLL discovery uses `extern "C"` module anchors.

Hot paths avoid the STL and use custom allocators (36 types) and containers. Reflection is provided by the RTTI module, which drives serialization, UI binding, and automatic OpenAPI/Swagger generation for the Remoting module.

The five source groups under `Source/`:

| Group | Role | Entry codemap |
|-------|------|---------------|
| **Runtime** | Core engine libraries (8 modules) | [Source/Runtime/codemap.md](Source/Runtime/codemap.md) |
| **Extensions** | Pluggable backends (RHI Vulkan, ImGui UI) | [Source/Extensions/codemap.md](Source/Extensions/codemap.md) |
| **ContentPipeline** | Asset processing & build orchestration | [Source/ContentPipeline/codemap.md](Source/ContentPipeline/codemap.md) |
| **Tools** | Developer utilities (IO detouring, distributed FS) | [Source/Tools/codemap.md](Source/Tools/codemap.md) |
| **Programs** | Standalone executables & test harnesses | [Source/Programs/codemap.md](Source/Programs/codemap.md) |

## Flow
**Build time:** The Go build system reads `*-module.json` across all subfolders, generates `BuildModules.generated.h` (static/dynamic registration, dependency lists, DLL import/export symbols), and performs unity builds per module with HAL-aware file exclusion.

**Runtime:** Modules initialize in dependency order (controlled by `LoadOrder` in module JSON): Core first (foundation), then Application, RHI, RTTI, Serialize, VFS, Network, Remoting. Extensions (RHIVulkan, ApplicationUI) load dynamically and register with modular services. ContentPipeline processes assets through the BuildGraph DAG. Programs (BuildRobot, ShaderToy, VoxelCube, WindowTest, UnitTest, IOWrapperTest) exercise the runtime subsystems.

I/O is fiber-friendly: blocking stream operations yield the fiber, allowing the work-stealing task scheduler (`FTaskManager` / `FTaskScheduler`) to run other tasks.

## Integration
- **Core** underpins everything: containers, math (68 types), allocators, threading, IO, HAL, meta-programming, diagnostics.
- **RHI** is the API-agnostic rendering abstraction; **Extensions/RHIVulkan** implements it for Vulkan.
- **RTTI** drives **Serialize** (format generation) and **Remoting** (automatic OpenAPI/Swagger); it also feeds **Extensions/ApplicationUI** (ImGui binding).
- **VFS** provides trie-based mount-point resolution used by ContentPipeline, Tools/IOWrapper, and Programs.
- **Tools/IODetouring** hooks Win32 file APIs for debugging; **Tools/IOWrapper** wraps the VFS for distributed builds.
- **ContentPipeline** consumes Runtime (Serialize, VFS, RTTI) and feeds Programs (BuildRobot).

**Navigation tip:** Start at [`Source/codemap.md`](Source/codemap.md), then drill into a group, then a module, then `Public/`/`Private/` subfolders. Every folder containing source has a `codemap.md` with four sections: `## Responsibility`, `## Design`, `## Flow`, `## Integration`.
