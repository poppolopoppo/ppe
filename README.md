# PPE - Custom C++ Game Engine

> A from-scratch, general-purpose game engine and rendering framework written in modern C++. Over a decade of iterative development - a full rendering pipeline, a custom build system, a reflection-driven serialization framework, a fiber-based concurrency model, and a modular plugin architecture. Everything built from first principles.

[![Windows build](https://github.com/poppolopoppo/ppe/actions/workflows/build_windows.yml/badge.svg)](https://github.com/poppolopoppo/ppe/actions/workflows/build_windows.yml)
[![Linux tests](https://github.com/poppolopoppo/ppe/actions/workflows/build_linux.yml/badge.svg)](https://github.com/poppolopoppo/ppe/actions/workflows/build_linux.yml)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)]()
[![Go](https://img.shields.io/badge/Go-build_system-00ADD8.svg)]()
[![Vulkan](https://img.shields.io/badge/API-Vulkan-000000.svg)]()
[![Commits](https://img.shields.io/badge/commits-4155+-brightgreen.svg)]()
[![Since](https://img.shields.io/badge/since-2014-lightgrey.svg)]()

---

## Why This Exists

I started PPE in 2014 out of a genuine desire to understand how game engines work at their deepest level - not at the "call the engine API" level, but at the "how does memory get from disk to GPU" level. What started as a hobby project became a decade-long exercise in systems thinking.

Every time I hit a wall - a subtle memory bug, a fiber deadlock that only appeared under high contention, a hash table prime that was just too close to a power of two - I had to actually understand it, not paper over it with a library. That's what this codebase represents: the accumulated answers to hard questions, written in C++.

The result is ~286,000 lines of hand-rolled C++17 across 1,505 source files, with no STL in hot paths, custom allocators for every access pattern, a Vulkan renderer built on a frame graph, a runtime reflection system that drives serialization and REST API generation, and a fiber-based task scheduler. The build system is written in Go because the engine needed things no off-the-shelf build tool offered - HAL-aware file exclusion, module anchor patterns, unity builds per module, and deterministic compilation.

This project taught me more about software architecture than anything else I've worked on. It's also where I learned that the best abstractions are the ones that make the wrong thing hard to do - a lesson I try to carry into every codebase I touch.

---

## At a Glance

| Metric | Count |
|---|---|
| **C++ source files** | 1,505 (945 headers / 560 sources) |
| **Lines of engine code** | ~286,000 (git-tracked, excluding third-party) |
| **Git commits** | 4,155 since 2014 |
| **Runtime modules** | 8 (Core, Application, RHI, RTTI, Serialize, VFS, Network, Remoting) |
| **Extensions** | 2 (RHIVulkan, ApplicationUI) |
| **Content pipeline modules** | 7 (BuildGraph, MeshBuilder, PipelineCompiler, PipelineReflection, Texture, Asset.Geometry, Asset.Texture) |
| **Programs** | 6 (BuildRobot, ShaderToy, VoxelCube, WindowTest, UnitTest, IOWrapperTest) |
| **External dependencies** | 19 (vulkan, glslang, spirv-*, imgui, vma, stb, lz4, xxHash, .) |
| **Custom containers** | 50 types (62 headers including templates) |
| **Custom allocators** | 36 |
| **Math types** | 68 |
| **HAL platform interfaces** | 18 |
| **Template declarations** | 5,705 |
| **Scoped enums** | 326 |

---

## Documentation & Navigation

This repository ships with a **hierarchical codemap** - a `codemap.md` in (almost) every source folder. Each codemap has four sections:

- **Responsibility** - what the folder owns
- **Design** - key types, patterns, and decisions
- **Flow** - how data/control moves through it
- **Integration** - how it connects to other folders (with real `Source/...` links)

Start here:

- [`codemap.md`](codemap.md) - root atlas: the whole tree at a glance, with links to every module.
- [`AGENTS.md`](AGENTS.md) - navigation guide for AI coding agents (how to read the codemaps, key architecture concepts).
- [`Source/codemap.md`](Source/codemap.md) - `Source/` aggregator and inter-module dependencies.

The rest of this README is a high-level tour. For per-module type inventories, API details, and design rationale, follow the codemap links in each section below.

---

## Architecture Overview

PPE is organized as a **hierarchical modular system** with strict dependency ordering. Each module declares its interface in a `*-module.json` file consumed by the custom Go-based build system. Modules follow a lifecycle pattern (`Start -> PostStart -> DutyCycle -> PreShutdown -> Shutdown`) and communicate through a **type-safe service registry** (`FModularServices`) that supports parent-child scoping for dependency injection.

- **[`Source/`](Source/)**
  - **[`Runtime/`](Source/Runtime/)** - Core engine libraries
    - **[`Core/`](Source/Runtime/Core/)** - Foundation: containers, math, memory, thread, IO, HAL, meta, diagnostics
    - **[`Application/`](Source/Runtime/Application/)** - Windowing, input, UI, platform application layer
    - **[`RHI/`](Source/Runtime/RHI/)** - Rendering Hardware Interface (API-agnostic)
    - **[`RTTI/`](Source/Runtime/RTTI/)** - Runtime type information and reflection
    - **[`Serialize/`](Source/Runtime/Serialize/)** - Serialization: binary, JSON, markup, FAT, transactional
    - **[`VFS/`](Source/Runtime/VFS/)** - Virtual File System with mount points and trie resolution
    - **[`Network/`](Source/Runtime/Network/)** - Socket and HTTP client/server
    - **[`Remoting/`](Source/Runtime/Remoting/)** - RTTI-driven HTTP API with automatic OpenAPI/Swagger generation
  - **[`Extensions/`](Source/Extensions/)** - Pluggable backends
    - **[`RHIVulkan/`](Source/Extensions/RHIVulkan/)** - Vulkan implementation of the RHI
    - **[`ApplicationUI/`](Source/Extensions/ApplicationUI/)** - ImGui-based UI service
  - **[`ContentPipeline/`](Source/ContentPipeline/)** - Asset processing tools (BuildGraph, MeshBuilder, PipelineCompiler, PipelineReflection, Texture, Asset.*)
  - **[`Tools/`](Source/Tools/)** - Developer utilities (IODetouring, IOWrapper)
  - **[`Programs/`](Source/Programs/)** - Standalone executables (BuildRobot, ShaderToy, VoxelCube, WindowTest, UnitTest, IOWrapperTest)
  - **[`External/`](Source/External/)** - Third-party dependencies (integrated as modules)
  - **[`Legacy/`](Source/Legacy/)** - Superseded systems kept for reference: ECS (replaced by `TSparseArray`), old RHI (replaced by frame graph), old engine structure

**Design philosophy**: No STL in hot paths. Everything is custom-built with explicit control over memory layout, cache behavior, and compilation units. Heavy use of C++ templates, CRTP, expression templates, constexpr, and macro-based code generation.

---

## Core Foundation

The foundation layer - containers, math, memory/allocators, threading/fibers, meta-programming, platform HAL, I/O, and diagnostics - all hand-rolled. Highlights: a Naughty-Dog-inspired **fiber scheduler** (thousands of logical tasks on a fixed thread pool), `TCompressedPair` empty-base optimization, `TScalarVector` expression templates, and a 64-bit-packed `FLeakDetector`.

- Full inventory (62 container types, 68 math types, 36 allocators, ...): [Source/Runtime/Core/codemap.md](Source/Runtime/Core/codemap.md)
- Per-subsystem codemaps live under `Core/Public/*` and `Core/Private/*` (e.g. [Memory](Source/Runtime/Core/Public/Memory/codemap.md), [Maths](Source/Runtime/Core/Public/Maths/codemap.md), [Thread/Task](Source/Runtime/Core/Private/Thread/Task/codemap.md), [Meta](Source/Runtime/Core/Public/Meta/codemap.md), [Diagnostic](Source/Runtime/Core/Public/Diagnostic/codemap.md)).

---

## Modular System

Every subsystem is a self-contained module declared in `*-module.json`; the build system generates `BuildModules.generated.h` and wires dependencies automatically via a cross-DLL **module anchor pattern** (`extern "C"` function pointers). Lifecycle: `Start -> PostStart -> DutyCycle -> PreShutdown -> Shutdown`.

- [Source/Runtime/Core/Public/Modular/codemap.md](Source/Runtime/Core/Public/Modular/codemap.md)

---

## RTTI & Reflection

A complete runtime type system (`FMetaClass`, `FMetaProperty`, `FMetaObject`, ...) that drives serialization, UI binding, and REST API generation. Metadata is auto-generated from C++ annotations (`PPE_DEFINE_AUTOPOD` / `PPE_DEFINE_AUTOSTRUCT`) into `*.generated.h` - annotate once, use everywhere.

- [Source/Runtime/RTTI/codemap.md](Source/Runtime/RTTI/codemap.md)

---

## Serialization Framework

Multi-format serialization (Binary, JSON, Markup, FAT, Text) built on a **runtime-defined DSL grammar** (Lexer -> Parser -> AST -> Serializer) with a transactional object-graph system for undo/redo and asset loading.

- [Source/Runtime/Serialize/codemap.md](Source/Runtime/Serialize/codemap.md)

---

## Virtual File System

A mount-based VFS with trie-structured path resolution (`FVirtualFileSystemTrie`), virtual-to-native aliasing, glob/regex matching, and compressed read/write. Exposed via a C-like `VFS_*()` API.

- [Source/Runtime/VFS/codemap.md](Source/Runtime/VFS/codemap.md)

---

## Rendering Hardware Interface (RHI)

An API-agnostic rendering abstraction with a **frame graph** for automatic resource lifetime management and render-pass merging, a three-tier ID system, and a custom shader-pipeline descriptor with reflection/attributes.

- RHI core: [Source/Runtime/RHI/codemap.md](Source/Runtime/RHI/codemap.md)
- Vulkan backend: [Source/Extensions/RHIVulkan/codemap.md](Source/Extensions/RHIVulkan/codemap.md) - including the build-time-generated `vk` namespace bindings.

---

## Application Framework

A modular, service-based application layer: `FApplicationBase` / `FApplicationWindow` lifecycle, a three-layer input system (Devices -> Actions -> Service), windowing, and an abstract UI service implemented by the `ApplicationUI` extension.

- Application: [Source/Runtime/Application/codemap.md](Source/Runtime/Application/codemap.md)
- UI service: [Source/Extensions/ApplicationUI/codemap.md](Source/Extensions/ApplicationUI/codemap.md)

---

## Network & Remoting

A from-scratch HTTP server/client (no third-party HTTP lib) plus an **RTTI-driven remoting layer** that exposes any RTTI-enabled class as a REST endpoint with automatic OpenAPI/Swagger generation.

- Network: [Source/Runtime/Network/codemap.md](Source/Runtime/Network/codemap.md)
- Remoting: [Source/Runtime/Remoting/codemap.md](Source/Runtime/Remoting/codemap.md)

---

## Content Pipeline

A DAG-based asset build system (`FBuildGraph`) with incremental, multi-threaded builds, plus mesh/texture import, shader compilation with SPIRV reflection, and pipeline introspection.

- [Source/ContentPipeline/codemap.md](Source/ContentPipeline/codemap.md)

---

## Build System

A custom **Go-based build system** (`PPE.go`) with archetypes, code generation (`BuildModules.generated.h`, `BuildVersion.generated.h`), HAL-aware file exclusion, per-module PCH/unity builds, and distributed builds via I/O detouring.

**Build**: run [`configure.bat`](configure.bat) / [`configure.sh`](configure.sh) to build the Go binary and generate project files (`PPE configure -and vscode -and vcxproj -Summary`).

- See `PPE.go` and the `Build/` tooling for details.

---

## Tools & Utilities

| Tool | Purpose |
|---|---|
| **BuildRobot** ([`Programs/BuildRobot/`](Source/Programs/BuildRobot/)) | CI/asset build orchestrator |
| **ShaderToy** ([`Programs/ShaderToy/`](Source/Programs/ShaderToy/)) | Shader playground |
| **VoxelCube** ([`Programs/VoxelCube/`](Source/Programs/VoxelCube/)) | Voxel rendering demo |
| **WindowTest** ([`Programs/WindowTest/`](Source/Programs/WindowTest/)) | Windowing/input test harness |
| **UnitTest** ([`Programs/UnitTest/`](Source/Programs/UnitTest/)) | Engine unit-test runner |
| **IOWrapperTest** ([`Programs/IOWrapperTest/`](Source/Programs/IOWrapperTest/)) | I/O wrapper test |
| **IODetouring** ([`Tools/IODetouring/`](Source/Tools/IODetouring/)) | Win32 I/O interception (MinHook/Detours) for distributed builds |
| **IOWrapper** ([`Tools/IOWrapper/`](Source/Tools/IOWrapper/)) | Filesystem wrapper redirecting I/O to remote WebDAV |

External dependencies (20, integrated as modules): vulkan, glslang, spirv-cross, spirv-reflect, spirv-tools, imgui, vma, renderdoc, stb, lz4, xxHash, detours, minhook, and others.

- Programs: [Source/Programs/codemap.md](Source/Programs/codemap.md) - Tools: [Source/Tools/codemap.md](Source/Tools/codemap.md)

---

## Additional Resources

- **Doxygen documentation**: Run [`Doc/doxygen.bat`](Doc/doxygen.bat) to generate full API documentation
- **Shell completions**: Available in [`Extras/`](Extras/) for bash, fish, nushell, and PowerShell
- **Benchmarks**: [`Extras/Benchmarks/`](Extras/Benchmarks/) contains performance comparison data
- **Debugging**: [`Extras/Debug/PPE.natvis`](Extras/Debug/PPE.natvis) provides Visual Studio debugger visualizations
- **Profiling**: [`cpu.pprof`](cpu.pprof), [`mem.pprof`](mem.pprof) - pprof-compatible CPU and memory profiles
- **Future directions** (see [`TODO.md`](TODO.md)): Gaussian splatting, Nanite-like clustering, path tracing, volumetric clouds, screen-space horizon GI, REStir/DDGI, physics engine (PBD), HyperLogLog, Bloom filter, concurrent Hopscotch hashing

---

## License

Proprietary. All rights reserved.

---

*4,155 commits and ~286,000 lines of custom engine C++ across 1,505 source files. Every subsystem built from first principles - not because it was necessary, but because understanding how things work at the lowest level is the only way I know how to build things at the highest level.*
