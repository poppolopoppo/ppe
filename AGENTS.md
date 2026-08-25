# AGENTS.md — Navigating PPE with Codemaps

This file helps AI coding agents orient quickly in the PPE codebase using the
generated codemaps. **Read this before exploring unfamiliar code.**

## What a codemap is

Every source folder containing C++ has a `codemap.md` with four sections:

- `## Responsibility` — what the folder's job is
- `## Design` — key patterns, abstractions, types, decisions
- `## Flow` — how data/control moves through it
- `## Integration` — how it connects to other folders (real `Source/` paths)

Codemaps are **navigation aids**, not documentation of every function. When you
need exact signatures, read the actual header — do not trust a codemap as
authoritative for implementation details.

## How to navigate

1. Start at the **root atlas**: [`codemap.md`](codemap.md) (this repo's top index).
2. Drill into the group aggregator: [`Source/codemap.md`](Source/codemap.md).
3. Pick a group, then a module, then `Public/` / `Private/` subfolders.

Example path for "how does Vulkan rendering work":
`codemap.md` → `Source/codemap.md` → `Source/Extensions/codemap.md` →
`Source/Extensions/RHIVulkan/codemap.md` → `Source/Extensions/RHIVulkan/Public/Vulkan/RenderPass/codemap.md`

## Source tree (mapped)

| Group | Modules | Purpose |
|-------|---------|---------|
| `Source/Runtime/` | Core, Application, RHI, RTTI, Serialize, VFS, Network, Remoting | Core engine libraries |
| `Source/Extensions/` | RHIVulkan, ApplicationUI | Pluggable backends |
| `Source/ContentPipeline/` | BuildGraph, MeshBuilder, PipelineCompiler, PipelineReflection, Texture, Asset.* | Asset processing & build orchestration |
| `Source/Tools/` | IODetouring, IOWrapper | Developer utilities |
| `Source/Programs/` | BuildRobot, ShaderToy, VoxelCube, WindowTest, UnitTest, IOWrapperTest | Executables & tests |

## Intentionally NOT mapped

Do **not** expect codemaps under these paths (excluded by project scope):
- `Source/External/` — vendored third-party code
- `Source/Legacy/` — deprecated code
- `Build/` — build-system tooling

## Key architectural concepts (read before coding)

- **Module system**: each subsystem declares a `*-module.json`; the Go build
  system generates `BuildModules.generated.h`. Lifecycle:
  `Start → PostStart → DutyCycle → PreShutdown → Shutdown`.
- **FModularServices**: type-safe service registry with parent-child scoping
  for dependency injection. Register/resolve services here, not via globals.
- **RTTI-driven everything**: `RTTI` module generates reflection that drives
  `Serialize` (binary/JSON/markup/FAT/Text), `Remoting` (auto OpenAPI/Swagger),
  and `ApplicationUI` (ImGui binding). Use `META_DYNAMIC_CASTABLE_IMPL` /
  `RTTI_MODULE_DECL` macros rather than hand-rolling reflection.
- **Custom allocators & containers**: hot paths avoid STL. Use `TVector`,
  `TArray`, `TSparseArray`, smart pointers (`TRefPtr`/`TWeakPtr`), and the
  allocator types from `Core/Public/Allocator/` and `Core/Private/Memory/`.
- **Fiber-friendly I/O**: blocking stream ops yield the fiber; the
  work-stealing scheduler (`FTaskManager`/`FTaskScheduler`) runs other tasks.
- **HAL pattern**: platform code lives under `HAL/Windows`, `HAL/Linux`,
  `HAL/Generic` with target-alias macros selecting the active implementation.

## When you need deeper detail

- Use **codegraph** (`codegraph_explore`) for symbol-level source and call paths.
- Use the **explorer** agent for broad recon, **librarian** for external/library
  research, **oracle** for architecture/risk decisions, **fixer** for bounded
  implementation, **designer** for UI/UX.
