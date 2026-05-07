# PPE — Custom C++ Game Engine

> A from-scratch, general-purpose game engine and rendering framework written in modern C++. Over a decade of iterative development — a full rendering pipeline, a custom build system, a reflection-driven serialization framework, a fiber-based concurrency model, and a modular plugin architecture. Everything built from first principles.

[![Windows build](https://github.com/poppolopoppo/ppe/actions/workflows/build_windows.yml/badge.svg)](https://github.com/poppolopoppo/ppe/actions/workflows/build_windows.yml)
[![Linux tests](https://github.com/poppolopoppo/ppe/actions/workflows/build_linux.yml/badge.svg)](https://github.com/poppolopoppo/ppe/actions/workflows/build_linux.yml)
[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)]()
[![Go](https://img.shields.io/badge/Go-build_system-00ADD8.svg)]()
[![Vulkan](https://img.shields.io/badge/API-Vulkan-000000.svg)]()
[![Commits](https://img.shields.io/badge/commits-4155+-brightgreen.svg)]()
[![Since](https://img.shields.io/badge/since-2014-lightgrey.svg)]()

---

## Why This Exists

I started PPE in 2014 out of a genuine desire to understand how game engines work at their deepest level — not at the "call the engine API" level, but at the "how does memory get from disk to GPU" level. What started as a hobby project became a decade-long exercise in systems thinking.

Every time I hit a wall — a subtle memory bug, a fiber deadlock that only appeared under high contention, a hash table prime that was just too close to a power of two — I had to actually understand it, not paper over it with a library. That's what this codebase represents: the accumulated answers to hard questions, written in C++.

The result is ~286,000 lines of hand-rolled C++17 across 1,505 source files, with no STL in hot paths, custom allocators for every access pattern, a Vulkan renderer built on a frame graph, a runtime reflection system that drives serialization and REST API generation, and a fiber-based task scheduler. The build system is written in Go because the engine needed things no off-the-shelf build tool offered — HAL-aware file exclusion, module anchor patterns, unity builds per module, and deterministic compilation.

This project taught me more about software architecture than anything else I've worked on. It's also where I learned that the best abstractions are the ones that make the wrong thing hard to do — a lesson I try to carry into every codebase I touch.

---

## At a Glance

| Metric | Count |
|---|---|
| **C++ source files** | 1,505 (945 headers / 560 sources) |
| **Lines of engine code** | ~286,000 (git-tracked, excluding third-party) |
| **Git commits** | 4,155 since 2014 |
| **Runtime modules** | 8 (Core, Application, RHI, RTTI, Serialize, VFS, Network, Remoting) |
| **Extensions** | 2 (RHIVulkan, ApplicationUI) |
| **Content pipeline modules** | 5 (BuildGraph, MeshBuilder, PipelineCompiler, PipelineReflection, Texture) |
| **Programs** | 6 (BuildRobot, ShaderToy, VoxelCube, WindowTest, UnitTest, IOWrapperTest) |
| **External dependencies** | 19 (vulkan, glslang, spirv-*, imgui, vma, stb, lz4, xxHash, …) |
| **Custom containers** | 50 types (62 headers including templates) |
| **Custom allocators** | 36 |
| **Math types** | 68 |
| **HAL platform interfaces** | 18 |
| **Template declarations** | 5,705 |
| **Scoped enums** | 326 |

---

## Table of Contents

- [At a Glance](#at-a-glance)
- [Architecture Overview](#architecture-overview)
- [Core Foundation](#core-foundation)
  - [Custom Containers](#custom-containers)
  - [Concurrency & Fiber Scheduler](#concurrency--fiber-scheduler)
  - [Meta-Programming Utilities](#meta-programming-utilities)
  - [Platform HAL Abstraction](#platform-hal-abstraction)
  - [Memory Management](#memory-management)
  - [String & Text](#string--text)
  - [Time](#time)
  - [Math](#math)
  - [Diagnostics](#diagnostics)
  - [Opaque Data DSL](#opaque-data-dsl)
- [Modular System](#modular-system)
- [Rendering Hardware Interface (RHI)](#rendering-hardware-interface-rhi)
  - [Vulkan Backend](#vulkan-backend)
- [Runtime Type Information (RTTI)](#runtime-type-information-rtti)
- [Serialization](#serialization)
- [Application Framework](#application-framework)
- [Network & Remoting](#network--remoting)
- [Content Pipeline](#content-pipeline)
- [Build System](#build-system)
- [Tools & Utilities](#tools--utilities)
- [Additional Resources](#additional-resources)

---

## Architecture Overview

PPE is organized as a **hierarchical modular system** with strict dependency ordering. Each module declares its interface in a `*-module.json` file consumed by the custom Go-based build system. Modules follow a lifecycle pattern (`Start → PostStart → DutyCycle → PreShutdown → Shutdown`) and communicate through a **type-safe service registry** (`FModularServices`) that supports parent-child scoping for dependency injection.

- **[`Source/`](Source/)**
  - **[`Runtime/`](Source/Runtime/)** — Core engine libraries
    - **[`Core/`](Source/Runtime/Core/)** — Foundation: containers, math, memory, thread, IO, HAL, meta, diagnostics
    - **[`Application/`](Source/Runtime/Application/)** — Windowing, input, UI, platform application layer
    - **[`RHI/`](Source/Runtime/RHI/)** — Rendering Hardware Interface (API-agnostic)
    - **[`RTTI/`](Source/Runtime/RTTI/)** — Runtime type information and reflection
    - **[`Serialize/`](Source/Runtime/Serialize/)** — Serialization: binary, JSON, markup, FAT, transactional
    - **[`VFS/`](Source/Runtime/VFS/)** — Virtual File System with mount points and trie resolution
    - **[`Network/`](Source/Runtime/Network/)** — Socket and HTTP client/server
    - **[`Remoting/`](Source/Runtime/Remoting/)** — RTTI-driven HTTP API with automatic OpenAPI/Swagger generation
  - **[`Extensions/`](Source/Extensions/)** — Pluggable backends
    - **[`RHIVulkan/`](Source/Extensions/RHIVulkan/)** — Vulkan implementation of the RHI
    - **[`ApplicationUI/`](Source/Extensions/ApplicationUI/)** — ImGui-based UI service
  - **[`ContentPipeline/`](Source/ContentPipeline/)** — Asset processing tools
    - **[`BuildGraph/`](Source/ContentPipeline/BuildGraph/)** — DAG-based asset build system
    - **[`MeshBuilder/`](Source/ContentPipeline/MeshBuilder/)** — Mesh import and procedural generation
    - **[`PipelineCompiler/`](Source/ContentPipeline/PipelineCompiler/)** — Shader and pipeline compilation
    - **[`PipelineReflection/`](Source/ContentPipeline/PipelineReflection/)** — Pipeline introspection
    - **[`Texture/`](Source/ContentPipeline/Texture/)** — Texture processing and compression
  - **[`Tools/`](Source/Tools/)** — Developer utilities
    - **[`IODetouring/`](Source/Tools/IODetouring/)** — Win32 IO interception via MinHook/Detours
    - **[`IOWrapper/`](Source/Tools/IOWrapper/)** — File system wrapping for distributed builds
  - **[`Programs/`](Source/Programs/)** — Standalone executables
    - **[`BuildRobot/`](Source/Programs/BuildRobot/)** — CI/asset build orchestrator
    - **[`ShaderToy/`](Source/Programs/ShaderToy/)** — Shader playground
    - **[`VoxelCube/`](Source/Programs/VoxelCube/)** — Voxel rendering demo
    - **[`WindowTest/`](Source/Programs/WindowTest/)** — Windowing test harness
    - **[`UnitTest/`](Source/Programs/UnitTest/)** — Test runner
    - **[`IOWrapperTest/`](Source/Programs/IOWrapperTest/)** — IO wrapper test
  - **[`External/`](Source/External/)** — Third-party dependencies (20 integrated modules)
  - **[`Legacy/`](Source/Legacy/)** — Superseded systems kept for reference: ECS (replaced by `TSparseArray`), old RHI (replaced by frame graph), old engine structure

**Design philosophy**: No STL in hot paths. Everything is custom-built with explicit control over memory layout, cache behavior, and compilation units. Heavy use of C++ templates, CRTP, expression templates, constexpr, and macro-based code generation.

---

## Core Foundation

### Memory & Allocators

**36 custom allocator types** organized in a hierarchy from low-level system allocators to specialized high-level strategies:

| Allocator | Purpose |
|---|---|
| `FMallocBinned` | General-purpose binned allocator reducing fragmentation for small objects |
| `FMallocMipMap` | Virtual memory-based mipmap allocator for streaming large assets |
| `FBitmapAllocator` | Fixed-size block allocation via bitmap tracking |
| `FCascadedAllocator` | Chains multiple allocators with fallback policies |
| `FSlabHeap` / `FSlabAllocator` | Slab allocation for fixed-size objects |
| `FBucketAllocator` | Size-class bucketed allocation |
| `FInSituAllocator` | Small-object-in-pointer optimization |
| `FPageAllocator` / `FVirtualAllocator` | OS-level page and virtual memory management |
| `FStackLocalAllocator` | Thread-local stack allocator |
| `FTrackingAllocator` / `FTrackingMalloc` | Instrumented allocation for profiling |
| `FStlAllocator` | STL-compatible adapter |
| `FSystemAllocator` | Raw system allocation |
| `FStaticAllocator` | Compile-time fixed-capacity allocator |
| `FFreeListAllocator` | Free-list based allocation |

**Smart pointer and view system** ([`Source/Runtime/Core/Public/Memory/`](Source/Runtime/Core/Public/Memory/)):

- **`TInSituPtr<T, N>`** — Small-buffer-optimized polymorphic smart pointer. Stores derived objects inline when `sizeof(Derived) <= N`, avoiding heap allocation entirely. Uses `Meta::IClonable` for virtual copy/move into the raw buffer, with separate specializations for clonable vs. trivially-copyable types. Placement new + explicit destructor management. `TPureInterfacePtr<T>` variant for optional semantics.
- **`TRelativeView<T>`** — Position-independent memory view storing an offset relative to its own address. Half the size of a raw pointer+size pair, trivially serializable (no pointer fixup needed), survives reallocation without updating internal pointer. Copy and move are explicitly deleted — the type cannot be misused accidentally.
- **`TPointerWFlags<T>`** — Pointer tagging: stores 2 boolean flags in the low bits of an aligned pointer, eliminating padding. Used in octree node children.
- **`TRefPtr<T>` / `TWeakPtr<T>`** — Custom reference-counted and weak pointers with allocator support.
- **`TUniquePtr<T>` / `TUniqueView<T>`** — Ownership semantics with custom deleter policies.
- **`TSharedBuffer`** — Shared ownership of raw memory blocks.
- **`RValuePtr<T>`** — R-value reference wrapper with move semantics.
- **`InSituPtr`** — In-situ construction and polymorphic storage.

**Memory infrastructure**:

- **`FMemoryDomain`** — Domain-scoped memory tracking with per-domain statistics and release-on-shutdown.
- **`FMemoryPool<T>` / `FCachedMemoryPool<T>`** — Object pooling with optional LRU caching. `TMemoryPool<>` was optimized specifically to reduce thread contention in multi-threaded scenarios — the naive implementation became a bottleneck under the fiber scheduler.
- **`FMemoryStream`** — In-memory byte stream for serialization.
- **`TMemoryView<T>`** — Non-owning memory slice (contiguous data + count).
- **`FVirtualMemory` / `FVirtualMemoryCache`** — OS virtual memory allocation with caching layer.
- **`FMemoryTracking`** — Allocation tracking for profiling and leak detection.
- **`TPtrRef<T>`** — Pointer reference wrapper.

### Custom Container Library

**62 container types** — every data structure is hand-rolled to avoid STL overhead, with aggressive small-object optimization.

**Sequences**:

| Container | Description |
|---|---|
| `TVector<T>` | Contiguous dynamic array with inline capacity support. A latent use-after-free was caught only by ASAN during a stress test — the bug had been dormant for months. ASAN is now part of CI. |
| `TArray<T>` | Fixed-size or growable array with allocator support |
| `TDeque<T>` | Double-ended queue with block allocation |
| `TStack<T>` | LIFO stack |
| `TList<T>` | Doubly-linked list |
| `TIntrusiveList<T>` | Intrusive doubly-linked list, zero extra allocation |
| `TIntrusiveAVL<T>` | Intrusive balanced BST (AVL) |
| `TIntrusiveWAVL<T>` | Intrusive weighted AVL tree |
| `TRingBuffer<T>` | Circular buffer; constructible from `std::initializer_list<>` |
| `TSparseArray<T>` | Generational-index sparse array — see detailed note below |
| `TTupleVector<...>` | Structure-of-Arrays layout via tuple transposition |

**`TSparseArray<T>` — Generational Indices** ([`SparseArray.h`](Source/Runtime/Core/Public/Container/SparseArray.h), 537 lines): Inspired by [DataArray](http://greysphere.tumblr.com/post/31601463396/data-arrays), `FSparseDataId` packs an `Index` (array lookup) + `Key` (generation counter). When items are removed the slot goes to an intrusive free list (stored in the unused item's `FDataChunkRef` via `TPointerWFlags`), but existing references never invalidate because storage is chunked with growth pattern `s1 = s0 * 3` (not exponential ×2). Originally designed for an ECS (see [`Legacy/ECS/`](Source/Legacy/ECS/)), but generational indices proved so useful that `TSparseArray` became a first-class container across the engine. As noted in `TODO.md`: *"Note that TSparseArray<> is already a generational index array :)"*

**Associative**:

| Container | Description |
|---|---|
| `TAssociativeVector<K,V>` | Sorted vector with O(log n) lookup, superior cache density for small datasets |
| `TFlatMap<K,V>` / `TFlatSet<K>` | Linear-probing open-addressed hash tables |
| `THashMap<K,V>` / `THashSet<K>` | Chained hash tables with allocator support |
| `TDenseHashMap<K,V>` / `TDenseHashSet<K>` | Dense-packed hash tables — see detailed note below |
| `TFixedSizeHashTable<K,V>` | Fixed-capacity hash table, zero allocation. `Meta::FEmptyKey` trait allows user types to override tombstone values. |
| `TStringHashMap<V>` | String-keyed hash map with custom string hashing |
| `TMultiMap<K,V>` | Multi-value map |
| `TMap<K,V>` | General-purpose map abstraction |
| `TConcurrentHashMap<K,V>` | Thread-safe concurrent hash map |

**`TDenseHashMap<>` / `TDenseHashSet<>` — Prime-Capacity Buckets** ([`DenseHashMap.h`](Source/Runtime/Core/Public/Container/DenseHashMap.h)): Use a sparse bucket list + dense node list algorithm optimized for cache-friendly iteration. Sparse capacity uses **29 precomputed "good" primes** chosen to be maximally far from powers of two (sourced from [planetmath.org/goodhashtableprimes](https://planetmath.org/goodhashtableprimes)) to maximize hash entropy. Trade-off: slightly slower insert/lookup vs. `THashMap<>`, but dramatically faster iteration since data is stored contiguously. The prime selection wasn't intuitive — primes close to powers of two caused measurably worse distribution in practice.

**Tries & String Structures**:

| Container | Description |
|---|---|
| `TBurstTrie` | Space-efficient compressed trie for string storage |
| `TCompressedRadixTrie` | Radix trie with path compression for filesystem/VFS paths |
| `TTernarySearchTree<K,V>` | Ternary search tree for string-keyed maps |
| `TPatriciaTrie<K,V>` | Patricia trie implementation |

**Bit Structures**:

| Container | Description |
|---|---|
| `TBitSet` | Dynamic bitset with word-parallel operations |
| `TBitTree` | Bit-indexed tree (Fenwick tree) |
| `TBitMask` / `TByteMask` | Bit and byte mask utilities |

**Specialized**:

| Container | Description |
|---|---|
| `TPerfectHashMap<K,V>` | Compile-time or load-time perfect hash generation for collision-free O(1) lookup |
| `TMinMaxHeap<T>` | Double-ended priority queue |
| `TPolymorphicTuple<...>` | Type-erased tuple storing heterogeneous types with interface dispatch |
| `TTuple<...>` / `TPair<K,V>` | Custom tuple and pair implementations |
| `TRawStorage<T>` | Type-erased inline storage with placement semantics |
| `TCompressedPair<A,B>` | Empty-base optimization — see note below |
| `TEnumerable<T>` | Enumerator pattern container; supports default construction and empty states |
| `TResizable<T>` | Resizable wrapper with custom growth policy |
| `TToken<T>` | Token-based access control container |
| `TMRUCache<K,V>` | Most-recently-used bounded cache |

**`TCompressedPair<A,B>` — Empty Base Class Optimization** ([`CompressedPair.h`](Source/Runtime/Core/Public/Container/CompressedPair.h)): `TCompressedPair<void*, std::tuple<>>` is the **same size as a single pointer**, while `TPair<>` would be 2×. The C++ standard guarantees every member has a distinct address, but `TCompressedPair<>` stores one element as a base class instead of a member, sidestepping that rule. Exposes `first()` and `second()` as functions rather than direct members. Critical for containers storing empty type tags alongside pointers.

**Fixed-size inline containers everywhere**: `ASSOCIATIVE_VECTORINSITU(K, V, N)`, `FLATMAP_INSITU(..., N)`, `TFixedSizeStack<T, N>`, `TFixedSizeFlatMap<K, V, N>` — avoid heap allocation for small datasets by storing inline capacity directly in the container. Used in module registration (8 entries), VFS trie nodes (8 entries), ImGui resource caches (5 entries), and Vulkan descriptor sets.

### Mathematics Library

**68 math types** covering vectors, matrices, transforms, spatial structures, spherical harmonics, and advanced numerical methods.

**Core types** ([`Source/Runtime/Core/Public/Maths/`](Source/Runtime/Core/Public/Maths/)):

- **`TScalarVector<T, N>`** — N-dimensional vector (N=1..4) with full **expression template** support for zero-overhead deferred computation. Key implementation details:
  - Union-based exhaustive swizzling: every possible 2D, 3D, 4D swizzle exposed as named member variables (`xy`, `xz`, `yz`, `xyz`, `xyzw`, `wzyx`, `rgba`, etc.). Compile-time swizzling with no function call overhead.
  - Comma operator overloading for construction: `(v.xy, 0, 1)` builds a 4D vector at compile time through expression template chaining. Near-scripting-language ergonomics at zero runtime cost.
  - Fold expressions (`Meta::static_for`) for compile-time loop unrolling
  - Extern template declarations for all bool/int/uint/float/double x1–4 combos to reduce binary bloat
  - Godbolt verification links directly in the source header to confirm codegen → [`ScalarVector.h`](Source/Runtime/Core/Public/Maths/ScalarVector.h)

- **`TScalarMatrix<T, R, C>`** — R×C matrix with expression templates:
  - `TScalarMatrixTranspose` — Transposed view without copying (swaps row/col access)
  - `TScalarMatrixCrop` — Sub-matrix view with offset
  - `TScalarMatrixHomogeneous` — Auto-extend with identity padding for homogeneous coords
  - Row-major storage with multi-view union: `data[]`, `m[][]`, `rows[]`, `transposed`

- **`TQuaternion<T>`** — Quaternion as `float4` union with exponential/logarithm for spherical interpolation.
- **`FTransform`** — Compact 40-byte transform: `FQuaternion` (16) + `float3` translation (12) + `float3` scale (12). Supports `Accumulate`/`Multiply` for animation blending.

**Geometric primitives**: `FPlane`, `FRay`, `FSphere`, `FFrustum`, `FScalarRectangle`, `FScalarBoundingBox`, `FCollision` — full intersection testing suite.

**Spatial structures**:

- **`FBIH` (Bounding Interval Hierarchy)** — Uses **64-bit exact bitfield packing**: Axis(2) + Child0(20) + Clip0(10) + Clip1(10) + Split(22) = 64 bits. Each node is exactly one cache-line-friendly 64-bit word — perfect density for spatial traversal. → [`BoundingIntervalHierarchy.h`](Source/Runtime/Core/Public/Maths/BoundingIntervalHierarchy.h)
- **`FOctree<T>`** — Octree using `TPointerWFlags` for children (flags encoded in low pointer bits).

**Spherical Harmonics** (5 files — references: Sloan's "Stupid SH36", SCEA GDC 2003, Habel's SSH):

- `TSHVector<N>` — Dynamically allocated SH coefficients per RGB channel
- `FSHRotation` — Precomputed rotation matrices per SH band
- `FSHSampleCollection` — Jittered stratification sampling with precomputed SH basis values
- `FSHFunctionProjector` — Projects polar/3D functions into SH basis
- `SHVector` — Core SH operations and evaluation

**Advanced numerical methods**:

| Type | Description |
|---|---|
| `FEigenMatrixSolver` | Full symmetric QR algorithm (Golub & Van Loan) for eigendecomposition |
| `FGaussianMixtureModels3f` | GMM with EM algorithm; `FGaussianRandom3f` uses Box-Muller + Cholesky |
| `FVarianceEstimator` | Welford's online algorithm + reservoir sampling + approximate histogram |
| `FLeastSquaresFitting` | Quadratic, sphere, circle, orthogonal line/plane fitting |
| `FConvexHull` | Monotone chain algorithm |
| `FPNTriangle` | PN Triangle tessellation (Vlachos et al.) for curved surfaces |
| `FThreefy` | Custom PRNG |
| `FRandomGenerator` | General-purpose random number generation |
| `FBinPacking` | 2D/3D bin packing algorithms |
| `FRange` | Range utilities |

**Packing system** ([`PackingHelpers.h`](Source/Runtime/Core/Public/Maths/PackingHelpers.h), [`PackedVectors.h`](Source/Runtime/Core/Public/Maths/PackedVectors.h)):

- **`TBasicNorm<T>`** — Transparent normalized type wrapper (SNorm/UNorm) with automatic float→integer conversion
- **`UX10Y10Z10W2N`** — 32-bit packed: 10-10-10-2 bit normals
- **`UX11Y11Z10`** — 32-bit packed RGB with shared exponent
- **`FHalfFloat`** — FP16↔FP32 conversion via union bit manipulation
- Quantization: uniform, signed-log, unsigned-log with floor/ceil/round variants

### Concurrency & Fiber Scheduler

A custom **fiber-based task scheduler** enabling thousands of logical tasks on a fixed thread pool — directly inspired by the Naughty Dog fiber architecture.

| Component | Description |
|---|---|
| `FFiber` | User-mode context switching wrapping platform fibers (Windows `CreateFiber`/`SwitchToFiber`). `ThreadFiber()` for main thread, `RunningFiber()` for current fiber |
| `FTaskManager` | Task dependency graph execution with scheduling, stealing, and fiber handoff |
| **`FTaskScheduler`** | **Priority-based work-stealing scheduler** — originally based on *"Priority Work-Stealing Scheduler"* (decentralized, non-preemptive, fixed-priority), but diverged to keep threads blocking when no tasks are in-flight. Two backends: **(1) Work-stealing** (per-worker min-max heap, cache-line aligned queues, atomic priority groups with revision packing for order preservation, exponential backoff on contention, optional shuffling via `FRandomGenerator`), or **(2) Concurrent priority queue** (`CONCURRENT_PRIORITY_QUEUE`) for simpler configurations. 4 priority levels: `High > Normal > Low > Internal`. → [`TaskScheduler.h`](Source/Runtime/Core/Private/Thread/Task/TaskScheduler.h) |
| **`FCompletionPort`** | **Task synchronization primitive** — not to be confused with Windows I/O Completion Ports. Acts as a countdown barrier for task completion: `Start(n)` arms it with N expected completions, each task calls `OnJobComplete()` to decrement. When count reaches zero, queues waiting fibers for resume via `FInterruptedTask` (stores fiber ref + `ITaskContext*` + priority). Supports child port aggregation via `FAggregationPort` (attach multiple ports, `Join()`/`JoinAndReset()` to wait). Weak-ref counted for thread-safe attachment. → [`CompletionPort.h`](Source/Runtime/Core/Public/Thread/Task/CompletionPort.h) |
| `FGlobalThreadPool` | Thread pool backing the task scheduler |
| `TConcurrentQueue<T>` | Lock-free producer-consumer queue |
| `FMPMCBoundedQueue<T>` | Multi-producer multi-consumer bounded queue |
| `FWaitGroup` | Blocks until N concurrent operations complete (directly inspired by Go's `sync.WaitGroup`) |
| `FReadWriteLock` | Reader-writer lock for shared-exclusive access |
| `FSynchronizationBarrier` | Thread barrier for synchronized phase transitions |
| `FAtomicPool<T>` | Lock-free object pool. Requires explicit atomic fencing for correctness — the lock-free design made fencing requirements non-obvious. |
| `FAtomicRefPtrPool<T>` | Lock-free pool of reference-counted pointers |
| `FAtomicSet<T>` | Lock-free set operations |
| `FAtomicSpinLock` | Spinlock for short critical sections |
| `FCriticalSection` | Mutex-protected critical section. Replaced `FAtomicOrderedLock` after profiling found threads throttling CPU with busy-waiting under high contention — locks now own contiguous chunks of data instead of cycling through shared state. |
| `TThreadSafe<T>` | Thread-safe wrapper with configurable barrier type (incl. `DataRaceCheck`) |
| `FThreadContext` | Thread-local execution context |
| `FThreadPool` | Configurable thread pool with work distribution |
| `FDeferredStream` | Deferred execution stream for async I/O |
| `FDataRaceCheck` | Thread sanitizer-like data race detection in debug builds. Rewritten with atomic-based validation because `std::recursive_mutex` assumes thread identity — which doesn't hold for fibers. |

**Hard-won design details**: `FWaitGroup` is directly inspired by Go's `sync.WaitGroup`. `FWaitForTask_` had a spurious wakeup guard removed after it caused deadlocks — lesson: understand every invariant before touching it. `TaskFiberPool` includes canary debugging to verify fiber lifecycle state machines. Fibers use a guaranteed stack size for stack-overflow recovery support.

**Async I/O integration**: The fiber scheduler cooperates with the I/O system — buffered streams (`FBufferedStreamReader`/`FBufferedStreamWriter` with 64KB default buffers) yield the fiber during blocking I/O operations, allowing other tasks to execute on the same OS thread. Observable streams (`FObservableStreamReader`) wrap any reader with progress callbacks.

### Meta-Programming Utilities

A comprehensive type-level programming toolkit ([`Source/Runtime/Core/Public/Meta/`](Source/Runtime/Core/Public/Meta/), 32 files):

| Utility | Description |
|---|---|
| `TAlignedStorage<T>` | Type-aligned storage without constructor/destructor overhead |
| `TAlignment` | Compile-time alignment queries |
| `TAutoEnum<T>` | Auto-generates enum reflection metadata with `TConstExpr<>` helpers |
| `TAutoSingleton<T>` | CRTP singleton with lazy initialization. `TSingleton<T>` mixin used by VFS |
| **`TAutoStruct<T>`** | **Auto-generates POD structs** with `operator==`, `operator!=`, `hash_value`, `ASSUME_TYPE_AS_POD`. |
| `TBaseClass<>` | Empty base class optimization — allows empty type tags at zero memory cost |
| `TBitField<T, Index, Count>` | Compile-time bit field composition with chaining (`TAfter<Bit>::TField<Count>`). `TBit<T>` for single-bit with `True`/`False` helpers. Used in BIH nodes, block headers, and enum flags → [`BitField.h`](Source/Runtime/Core/Public/Meta/BitField.h) |
| `TCast<T>` | Safe downcast with RTTI verification |
| `TClonable<T>` | Interface for deep-copy polymorphic types (used by `TInSituPtr`) |
| `TEnum<T>` | Enum reflection with name→value and value→name mapping |
| **`TForRange`** | Range-based for loop with custom iteration policies. **`TForRange::static_for<>`** 2D variant constructs pairs with explicit integral constant type — replaced `std::initializer_list<>` in many places to avoid lifetime issues |
| `TFunctor<T>` | Type-erased callable wrapper. **`TFunction<>::Bind<>()`** — member function binding, significantly leaner than before → [`Function.h`](Source/Runtime/Core/Public/Misc/Function.h) |
| **`TFunctionRef<>`** | **Non-owning callable reference** — type-erased function view with zero allocation. Accepts lambdas, function pointers, and `TFunction` without copying. Uses a simple forward dispatcher + storage union (pointer or function pointer). No vtable, no heap. → [`Function.h`](Source/Runtime/Core/Public/Misc/Function.h) |
| **`TFunction<>`** | **Owning type-erased callable** with in-situ storage optimization (`TInSituPtr`). Captures lambdas, static functions (`Meta::StaticFunction<>`), and bound member functions. Payload stored inline when it fits; otherwise ref-counted allocation. Smart pointer wrapping for raw pointers to `FRefCountable` objects (`TSafePtr<T>`). `FireAndForget()` transfers ownership on invocation. → [`Function.h`](Source/Runtime/Core/Public/Misc/Function.h) |
| `FMd5sum` | Full constexpr MD5 — `round1()` through `round4()` each recurse 16 times; padding handled entirely at compile time via `leftover()` template. Better collision resistance than 32/64-bit hashes, zero runtime cost. Based on elbeno/constexpr → [`MD5.h`](Source/Runtime/Core/Public/Meta/MD5.h) |
| `TNumericLimits<T>` | Numeric limits with platform-aware bounds |
| `FOneTimeInitialize` | Thread-safe one-time initialization primitive |
| `TOptional<T>` | Optional value with in-place construction |
| **`TNumeric<T, Tag>`** | Type-safe numeric wrapper preventing unit mixing at compile time. User literals: `1_KiB`, `1_MiB`, `1_kB`, `1_MB`. `TStronglyTyped` tagged type wrappers → [`StronglyTyped.h`](Source/Runtime/Core/Public/Meta/StronglyTyped.h) |
| `TThreadResource<T>` | Thread-local resource management |
| `TTypeInfo<T>` | Runtime type information for generic types |
| `TTypeTraits<T>` | Extensive type trait utilities. `TEnableIf<>` for SFINAE |
| `TSafeBool` | Explicit bool conversion without implicit promotion |
| `TOneOf` | Type constraint: must be one of a set of types |
| **`TInPlace`** | In-place construction tag for controlled object construction/destruction |
| **`TAtScopeExit`** | RAII scope exit — cleanup at scope end |
| `TIterator` | Iterator category and trait utilities. `TIndexed<>` and `TEnumerated<>` helpers |
| `TDelete` | Safe deletion with null checks |
| `TConfig` | Compile-time configuration flags |
| **`FStaticString`** | Compile-time string utilities. `MakeStaticArray()` helper for `TStaticArray<>` construction |
| **`TEvent<>::Bind<>()`** | Wrap `TFunction<>::Bind<>` for event handlers — member function binding significantly leaner than before |
| `TConstExpr<>` | Constexpr helpers for C++20 compatibility |

**Design notes**:
- `TCompressedPair<>` uses Empty Base Class Optimization — `TCompressedPair<void*, std::tuple<>>` is the same size as a single pointer, while `TPair<>` would be 2×. Member functions instead of direct members avoid C++ address uniqueness guarantees.
- `TDenseHashMap<>` / `TDenseHashSet<>` use a sparse bucket list + dense node list algorithm with 29 precomputed good primes (chosen far from powers of 2) for sparse capacity to maximize hash entropy. Optimized for dense iteration at the cost of slightly slower insert/lookup vs. `THashMap<>`.
- `FModularServices` can be implicitly cast as an interface for clean service access.
- `TAutoStruct<>` / `PPE_DEFINE_AUTOSTRUCT` replaced `PPE_DEFINE_AUTOPOD` with better constexpr support and `ASSUME_TYPE_AS_POD` integration.
- `TForRange::static_for<>` 2D variant constructs pairs with explicit integral constant type — replaced `std::initializer_list<>` to avoid lifetime issues.
- `Meta::FEmptyKey` trait allows user types to override tombstone values in `TFixedSizeHashTable<>`.
- `TFunction<>::Bind<>()` / `TEvent<>::Bind<>()` — member function binding is significantly leaner than before.

### Platform HAL Abstraction

**Hardware Abstraction Layer** provides compile-time platform dispatch through a macro-based indirection pattern — clean, zero-cost, and easy to extend:

```cpp
// PlatformMacros.h resolves to HAL/Windows/WindowsPlatformMacros.h
// or HAL/Linux/LinuxPlatformMacros.h based on TARGET_PLATFORM
#include PPE_HAL_MAKEINCLUDE(PlatformMacros)

// Creates type aliases: FPlatformMemory = FWindowsPlatformMemory
PPE_HAL_MAKEALIAS(PlatformMemory)
```

**Fallback chain**: Platform-specific → `GenericPlatform` (included last for missing macros).

**Platform interfaces** (each with Windows, Linux, and Generic implementations):

| Interface | Purpose |
|---|---|
| `FPlatformMemory` | Memcpy, Memset, aligned allocation |
| `FPlatformAtomics` | CAS, memory barriers, fetch-add |
| `FPlatformTime` | RDTSC, CpuTime, Cycles, ChronoMicroseconds |
| `FPlatformThread` | Thread creation, TLS |
| `FPlatformFile` / `FPlatformLowLevelIO` | File operations, low-level read/write |
| `FPlatformCallstack` | Stack walking with symbol resolution |
| `FPlatformMaths` | SIMD intrinsics |
| `FPlatformHash` | Hardware-accelerated hashing (CRC32, AES-NI) |
| `FPlatformCrash` | Minidump generation |
| `FPlatformConsole` | Console I/O with color support |
| `FPlatformDialog` | OS dialog boxes |
| `FPlatformProfiler` | ETW/trace integration |
| `FPlatformEndian` | Byte swap operations |
| `FPlatformDebug` | Debugger detection and attachment |
| `FPlatformProcess` | Process management |
| `FPlatformString` | String functions |
| `FPlatformMisc` | Miscellaneous platform utilities |

**Target platform system**: `TargetPlatform.h` defines platform capabilities through compile-time feature flags. `winnt_version.h` controls Windows SDK version targeting.

### I/O & String System

A comprehensive I/O framework with **47 header files** covering strings, streams, file systems, and formatting.

**String system** ([`Source/Runtime/Core/Public/IO/`](Source/Runtime/Core/Public/IO/)):

| Type | Description |
|---|---|
| `FString` | Custom string with Small String Optimization (SSO), allocator support, inline capacity |
| `FStringView` | Non-owning string view (pre-C++17 implementation) |
| `FStaticString<N>` | Stack-allocated fixed-capacity string |
| `FStringBuilder` | Efficient concatenation with reserved capacity |
| `FText` | Localized text with key-based lookup |
| `FTextMemoization` | String interning for deduplication |
| `FConstChar` / `FConstNames` | Compile-time string constants |
| `FRegexp` | Regular expression engine |

**Stream system**:

| Type | Description |
|---|---|
| `IArchive` | Base serialization archive with byte-order and version support |
| `FFileStream` | Memory-mapped or buffered file stream |
| `FBufferedStream` | Buffered I/O with configurable buffer size (64KB default) |
| `FCompressedStream` | Compressed stream wrapper (LZ4 integration) |
| `FObservableStream` | Stream with progress callback |
| `FMemoryStream` | In-memory byte stream |
| `FStreamProvider` | Platform-specific stream creation |

**File system**:

| Type | Description |
|---|---|
| `FFileSystem` | Platform file system abstraction with properties and token-based access |
| `FFileSystemTrie` | Trie-based directory enumeration |
| `FMountingPoint` | VFS mount point configuration |
| `FBulkData` | Large data chunk management with lazy loading and streaming |

**Formatting**:

| Type | Description |
|---|---|
| `Format<T>` | Type-safe string formatting with custom formatters per type |
| `FTextWriter` / `FTextReader` | Text I/O with encoding support |
| `FLZJB` | LZJB compression algorithm |

**Path utilities**: `FBasename`, `FDirname`, `FDirpath`, `FExtname`, `FFilename` — path component extraction.

**Design details**: `FTextReader` parses hex/octal/binary integer literals. `TBasicTextWriter::operator<<` forbids implicit type promotion from `T*` to `bool` or `const void*` — a silent bug source caught repeatedly during development; resolved by using meta-programming to disable all implicit conversions except explicitly handled types. `FLexer`/`FLookAheadReader` was refactored away from `Peek(n)` (offset-based lookahead) to single-char `Peek()` only, since arbitrary-offset peek is extremely difficult to implement correctly and efficiently with the generic stream interface.

### Diagnostics & Debugging

A sophisticated debugging toolkit ([`Source/Runtime/Core/Public/Diagnostic/`](Source/Runtime/Core/Public/Diagnostic/), 14 files):

- **`FStackMarker`** — Return address hijacking for automatic call-stack annotation. RAII objects on the stack track call depth with begin/end pairs. Zero-cost when disabled (`USE_PPE_STACKMARKER = 0`). Two modes: lightweight (address only) vs. full (function, file, line).

- **`FLeakDetector`** (~670 lines) — Extremely sophisticated memory leak detector:
  - `FBlockHeader` — **64-bit packed**: Reserved(1) + Enabled(1) + SizeInBytes(30) + CallstackUID
  - `FCallstackData` — 32-frame backtraces with 128-bit fingerprint
  - `FCallstackTracker` — Custom open-addressed hash table with 16MB capacity (524,288 entries), writes callstacks to temp file via low-level I/O
  - `FHashedBlockTracker` — 251-bucket sharding to reduce lock contention
  - `FBlockCompressedRadixTrie` — Per-bucket compressed radix trie for O(1) pointer lookup
  - Reservoir sampling for representative allocation samples per callstack
  - Detects non-deleters (free without delete) and trimmers (realloc shrink)
  - TLS whitelist scope to ignore internal allocations

- **`Benchmark`** — Google Benchmark-style API with a clever iterator trick: `operator!=` controls loop continuation so timing happens inside the iteration protocol itself, not around it:
  - `DoNotOptimize` uses inline asm clobber to prevent compiler elimination
  - `TCounter` template with 4 timing backends: RawTicks (RDTSC), PerfCounter, ChronoTime
  - `FApproximateHistogram` with reservoir sampling
  - `TTable<Benchmarks...>` — Variadic template benchmark comparison with **multi-threaded execution**
  - Outputs: TXT, CSV, stacked bar charts

- **`FDebugFunction`** — **Natvis trick**: registers functions in a self-registering linked list so the Visual Studio debugger can call them at breakpoints. `DEBUG_FUNCTION` macro disables optimization + forces NO_INLINE. Enables interactive debugging of complex engine state without recompilation. → [`DebugFunction.h`](Source/Runtime/Core/Public/Diagnostic/DebugFunction.h)

- **`FDecodedCallstack`** — Uses `ALIGNED_STORAGE` to avoid constructor/destructor overhead for unused frames. Max depth 46, fingerprinted for deduplication.

- **`FException`** — Custom exception hierarchy with source location capture.
- **`FLogger`** — Full-featured structured logging system with **zero-allocation structured output** (see [Godbolt example](https://godbolt.org/z/xh3P8GvP4)):
  - **`ELoggerVerbosity`** bitfield enum: `Debug`, `Verbose`, `Info`, `Profiling`, `Emphasis`, `Warning`, `Error`, `Fatal` — combinable via bit flags (`NoDebug`, `NoDebugInfo`, `All`)
  - **`FLoggerCategory`** — Named categories with per-category verbosity and flags. Uses `hash_arr_constexpr()` for O(1) hash-based lookup. Flags: `Immediate` (flush on write), `BreakOnError`, `BreakOnWarning` (debug only)
  - **`FLoggerMessage`** — Structured message with `Opaq::object_view` for **key-value structured data** (type-erased, no allocation). `FLoggerSiteInfo` packs: `ThreadId` + `SourceLine`(23 bits) + `PackedLevel`(8 bits) + `IsTextAllocated`(1 bit) into 8 bytes
  - **Multiple output sinks**: stdout (with color support), `OutputDebugString`, append file, roll-file (with automatic rotation), **JSON logger** (`RegisterAppendJsonLogger()`, `RegisterRollJsonLogger()`), system trace logger
  - **Macros**: `PPE_LOG()` (formatted), `PPE_SLOG()` (**structured** with `Opaq::object_init` key-value pairs), `PPE_LOG_DIRECT()` (callback-based), `PPE_LOG_PRINTF()` (va_list), `PPE_LOG_RECORD()` (argument recording)
  - **Compile-time format validation**: `EValidateFormat` validates format strings statically via `ValidateFormatString()`
  - **Compile-time filtering**: `ShouldCompileMessage()` elides entire log sites when verbosity doesn't match — zero overhead in release
  - **Registration API**: `RegisterLogger()`, `RegisterStdoutLogger()`, `RegisterOutputDebugLogger()`, `RegisterAppendFileLogger()`, `RegisterRollFileLogger()`, `RegisterSystemTraceLogger()`

- **`FFeedbackContext`** — User feedback and progress reporting system.
- **`FCurrentProcess`** — Process information and diagnostics.
- **`FBuildVersion`** — Auto-generated build metadata: git branch, revision, compiler, timestamp. Note: this header was initially transitively included in every translation unit, severely impacting compilation performance — the build system was refactored to isolate it, which produced a measurable compile-time drop.

### Opaque Data DSL

A type-erased, allocation-free data representation system for structured key-value data — used by the logger, serializer, and RTTI systems ([`Source/Runtime/Core/Public/Misc/Opaque.h`](Source/Runtime/Core/Public/Misc/Opaque.h), [`OpaqueBuilder.h`](Source/Runtime/Core/Public/Misc/OpaqueBuilder.h), 560+ lines):

| Component | Description |
|---|---|
| **`Opaq::value_init`** | DSL for inline value declaration — variant of `nil`, `boolean`, `integer`, `uinteger`, `floating_point`, `string_init`, `wstring_init`, `array_init`, `object_init`, `string_literal`, `wstring_literal`, and `string_format`/`wstring_format` for dynamic formatting via `TFunctionRef`. Trivial type promotion: `i8`/`i16`/`i32` → `integer`, `u8`/`u16`/`u32` → `uinteger`, `float` → `floating_point`. All types assumed POD via `PPE_ASSUME_TYPE_AS_POD` |
| **`Opaq::value_view`** | Packed contiguous memory block — same variant types as `value_init` but with `string_view`, `wstring_view`, `array_view`, `object_view`. `TRelativeView<>`-based: strings, arrays, and objects are offsets into a single allocation, not independent pointers. `key_value_view` for named properties |
| **`Opaq::value_block`** | Single allocated block containing a fully packed `value_view`. `NewBlock()` computes exact size via `BlockSize()` then lays out all data contiguously. `TValueBlock<_Allocator>` ties allocation to a specific allocator with RAII cleanup |
| **`Opaq::IBuilder`** | Interactive builder interface — `BeginArray()`/`EndArray()`, `BeginObject()`/`EndObject()`, `BeginKeyValue()`/`EndKeyValue()`. Closure-based helpers: `Array(fn)`, `Object(fn)`, `KeyValue(key, fn)`. Full visitor pattern over all value types |
| **`Opaq::TBuilder<_Allocator>`** | Concrete builder with `TFixedSizeStack<TPtrRef<value_type>, 16>` for nested structure editing. Optional text memoization (`TBasicTextMemoization`) to deduplicate strings. `Peek()` for inspecting current edit position, `ToValueBlock()` for finalization |
| **`Opaq::XPath()`** | Path lookup into `object_view` / `value_view` — returns `Meta::TOptional<TPtrRef<const value_view>>`. `XPathAs<T>()` for typed extraction |
| **`RTTI::FOpaqueData`** | RTTI-layer opaque data container — `TAssociativeVector<FName, FAny>` with `TRawInlineAllocator` (3 slots per entry). `FOpaqueArray` for typed arrays. Native type helpers via `PPE_RTTI_OPAQUEDATA_NATIVETYPE_DECL` macro expansion over all RTTI native types → [`OpaqueData.h`](Source/Runtime/RTTI/Public/RTTI/OpaqueData.h) |

**Design details**: `Opaq::string_format` wraps `TFunctionRef<void(FTextWriter&)>` — a bound `Meta::StaticFunction<&FTextWriter::WriteValue<T>>` with the value, enabling type-erased formatting without vtable or allocation. `value_view` uses `TRelativeView<>` (pointer + offset) so the entire structure is relocatable as a single block — critical for passing structured log data across thread boundaries without copying. The `IBuilder` visitor pattern means a single `operator()` dispatch handles `value_init`, `value_view`, and `value<_Allocator>` uniformly.

---

## Modular System

The engine uses a **fully modular architecture** where every subsystem is a self-contained module with a well-defined interface, declared in a `*-module.json` and managed by the build system.

**Module lifecycle**: `Start(domain) → PostStart(domain) → DutyCycle(domain) → PreShutdown(domain) → Shutdown(domain) → ReleaseMemory(domain)` (deferred cleanup).

**Key components** ([`Source/Runtime/Core/Public/Modular/`](Source/Runtime/Core/Public/Modular/), 7 files):
- `FModuleStaticRegistration` uses `FLATMAP_INSITU(..., 8)` — inline capacity of 8 avoids heap for typical module counts.
- `FModuleDynamicRegistration` — runtime DLL loading with ref counting, anchor symbol lookup, dynamic library management via `LoadLibrary()`/`GetProcAddress()`.
- `IModuleInterface` — non-copyable, non-movable. `DutyCycle()` for frame-based async processing, `ReleaseMemory()` for deferred memory cleanup.
- `FModularDomain` owns a `FReadWriteLock` for thread-safe service access.
- `FModuleInfo` — Module metadata: Name, Phase, Usage, Source, LoadOrder, Dependencies, BuildVersion, Initializer factory. `LoadOrder` controls initialization order, `Usage` sets Runtime/Shipping/Tools/Developer.

**Module anchor pattern** ([`ModuleRegistration.h`](Source/Runtime/Core/Public/Modular/ModuleRegistration.h)): Cross-DLL module discovery via `extern "C"` function pointers. Each module exports a function returning `FModuleInfo*`. The build system generates registration code that calls these anchors for static linking or `LoadLibrary`/`GetProcAddress` for dynamic linking. **Zero manual dependency wiring needed** — declare in JSON, it wires itself.

**Module declaration** (`*-module.json`):
```json
{
    "IsolatedFiles": ["Private/Allocator/InitSegAllocator.cpp"],
    "PrivateDependencies": ["External/double-conversion", "External/xxHash"],
    "HAL": {
        "Linux": { "Libraries": ["ncurses", "tinfo", "dl", "rt"] }
    },
    "TAG": {
        "DEBUG|DEVEL": {
            "HAL": {
                "Windows": {
                    "Defines": ["USE_PPE_VIRTUALLOC_DETOUR=1"],
                    "PrivateDependencies": ["External/minhook"],
                    "RuntimeDependencies": ["External/vstools"]
                }
            }
        }
    }
}
```

**Dependency inversion pattern**: Modules declare dependencies at compile time via JSON, and the build system generates `BuildModules.generated.h` with static/dynamic module registration code, dependency lists, and DLL import/export symbols.

---

## RTTI & Reflection

A **complete runtime type information system** enabling serialization, UI binding, and scripting integration ([`Source/Runtime/RTTI/Public/`](Source/Runtime/RTTI/Public/), 42 public headers / 71 total):

| Component | Description |
|---|---|
| `FMetaClass` | Describes C++ classes: base classes, properties, functions, flags |
| `FMetaProperty` | Member variable metadata: type, offset, attributes, serialization info |
| `FMetaEnum` | Enum reflection: name→value, value→name, flag semantics |
| `FMetaFunction` | Function wrapper for dynamic invocation via RTTI |
| `FMetaObject` | RTTI-backed object with property access and modification |
| `FMetaTransaction` | Container for serialized object data or modification sets — facilitates undo/redo and asset loading |
| `FMetaDatabase` | Central registry of all RTTI metadata |
| `RTTI::FAtom` | Type-erased value container used by the serialization DSL grammar |

**Helper utilities**: [`MetaClassHelpers.h`](Source/Runtime/RTTI/Public/MetaClassHelpers.h), [`MetaEnumHelpers.h`](Source/Runtime/RTTI/Public/MetaEnumHelpers.h), [`MetaFunctionHelpers.h`](Source/Runtime/RTTI/Public/MetaFunctionHelpers.h), [`MetaObjectHelpers.h`](Source/Runtime/RTTI/Public/MetaObjectHelpers.h), [`MetaProperty.h`](Source/Runtime/RTTI/Public/MetaProperty.h) — utility functions for traversing and querying the RTTI graph.

**Code generation**: RTTI metadata is auto-generated by the build system from C++ source annotations. Modules use `PPE_DEFINE_AUTOPOD` and similar macros to declare reflection metadata, which is processed into `*.generated.h` files at build time. Annotate once; the same metadata drives serialization, REST endpoints, and editor tooling.

---

## Serialization Framework

A **multi-format serialization system** with a runtime-defined DSL grammar ([`Source/Runtime/Serialize/Public/`](Source/Runtime/Serialize/Public/)):

**Architecture**: Lexer → Parser → AST → Serializer

**Lexer** ([`Lexer/`](Source/Runtime/Serialize/Public/Lexer/)):
- `FLexer` — Tokenizer with look-ahead (`FLookAheadReader`)
- `FMatch` — Matched token with source span
- `FSymbol` — Token definitions
- `TextHeap` — String interning

**Parser / DSL Grammar** ([`Parser/`](Source/Runtime/Serialize/Public/Parser/)):
The grammar is **runtime-defined**, not a fixed grammar file. Symbols and productions are registered at startup via `FGrammarStartup` and `FLexerStartup`.

- `FParseExpression` — Abstract base for all AST nodes:
  - `TLiteral<T>` — Typed literal values
  - `FVariableExport` / `FVariableReference` — Variables with scope (Public/Private/Global)
  - `TUnaryFunction<_Functor>` / `TBinaryFunction<_Functor>` — Callable expressions with user functors
  - `TTernary<_Test>` — Conditional expressions
  - `FObjectDefinition` — Object construction
  - `FPropertyReference` — Member access
  - `FTupleExpr`, `FArrayExpr`, `FDictionaryExpr` — Collection literals
  - `FCastExpr` — Type casting via RTTI
  - `FFunctionCall` — Method invocation
  - `FSubscriptOperator` — Indexing with wrap-around support
- `FParseStatement` — Statement nodes
- `FParseContext` — Evaluation context with `RTTI::FAtom` values
- All expressions evaluate to `RTTI::FAtom` (type-erased value)
- Uses `VECTORINSITU` and `ASSOCIATIVE_VECTORINSITU` for small-object optimization in AST nodes

**Serializers** (5 format backends):

| Format | Module | Description |
|---|---|---|
| **Binary** | [`Binary/`](Source/Runtime/Serialize/Public/Binary/) | High-performance binary format |
| **JSON** | [`Json/`](Source/Runtime/Serialize/Public/Json/) | JSON support for config files and debugging, with RTTI integration |
| **Markup** | [`Markup/`](Source/Runtime/Serialize/Public/Markup/) | Human-readable markup format |
| **FAT** | [`FAT/`](Source/Runtime/Serialize/Public/FAT/) | "File Allocation Table" style format for packaging many small files into a single archive |
| **Text** | [`Text/`](Source/Runtime/Serialize/Public/Text/) | Plain text serialization |

**Transaction system**:
- **`ISerializer`** — Abstract interface with `Serialize`/`Deserialize`
- **`FTransactionSaver` / `FTransactionLinker`** — Object graph serialization with cross-references
- **`FTransactionSerializer`** — RTTI-driven serialization with object graph traversal

---

## Virtual File System

A **mount-based virtual file system** with trie-structured path resolution ([`Source/Runtime/VFS/Public/`](Source/Runtime/VFS/Public/)):

```cpp
// VFS is a singleton that inherits from and exposes FVirtualFileSystemTrie's interface
class FVirtualFileSystem : Meta::TSingleton<FVirtualFileSystemTrie, FVirtualFileSystem>
```

This combines the CRTP singleton pattern with full interface exposure — `FVirtualFileSystem` *is* a `FVirtualFileSystemTrie`, with static convenience methods (`VFS_ReadAll()`, `VFS_WriteAll()`, `VFS_Copy()`, `VFS_Compress()`, `VFS_Decompress()`) delegating to the singleton. C-like free-function API for ergonomics: `VFS()`, `VFS_OpenReadable()`, `VFS_GlobFiles()`, `VFS_MatchFiles()` (regex support), `VFS_RollFile()` (log rotation).

| Component | Description |
|---|---|
| `FVirtualFileSystemTrie` | Trie-based mount point resolution. Uses `ASSOCIATIVE_VECTORINSITU(..., 8)` for inline node storage — avoids heap for small mount counts. Tries beat hash maps here, but getting the radix compression right took several iterations. `FReadWriteLock` for concurrent access. |
| `IVirtualFileSystemComponent` | Abstract component interface with Readable, Writable, and ReadWritable variants (multiple inheritance) |
| `FVirtualFileSystemNativeComponent` | Native filesystem component — bridges VFS to the real OS filesystem |
| `VFS_*()` free functions | C-like API: `VFS()`, `VFS_OpenReadable()`, `VFS_GlobFiles()`, `VFS_MatchFiles()` (regex support), `VFS_RollFile()` (log rotation) |

**Features**: Mount/unmount operations, virtual-to-native path aliasing, temporary file generation, compressed read/write, glob and regex file matching, log rotation.

---

## Rendering Hardware Interface (RHI)

An **API-agnostic rendering abstraction** with modern frame graph architecture ([`Source/Runtime/RHI/Public/`](Source/Runtime/RHI/Public/)).

### Frame Graph Architecture

The RHI implements a **frame graph** pattern for automatic resource lifetime management and render pass merging:

- **`IFrameGraph`** — Central rendering interface combining resource manager and frame execution. Resource creation returns strongly-typed IDs (`FImageID`, `FBufferID`, `FGPipelineID`, etc.). Frame pipeline: `PrepareNewFrame() → Begin() → Task() → Execute() → Flush() → Wait()`.
- **`TAutoResource<T>`** — RAII wrapper that auto-releases GPU resources on destruction. Prevents resource leaks in exception-heavy frame graph code. Works with all RHI resource types.
- **`IFrameTask`** — Frame-level task with dependency tracking, debug naming, and color coding.

### Resource Management

**Three-tier ID system** ([`ResourceId.h`](Source/Runtime/RHI/Public/RHI/ResourceId.h)):

| Tier | Type | Description |
|---|---|---|
| Polymorphic | `FResourceHandle` | 64-bit packed: `{u32 Uid, u16 Index, u16 InstanceID}`. InstanceID prevents ABA problems in concurrent resource access. Uses `Visit()` pattern for type-safe dispatch. User literal syntax: `RHI::FUniformID"my_uniform"_uniform` |
| Typed | `TResourceId<_Uid>` | Compile-time typed IDs with 32-bit packing |
| Wrapped | `TResourceWrappedId<T>` | Move-only wrapper for strong ownership semantics (deleted copy, move-only) — makes accidental handle copying a compile error |

**Resource proxy** ([`ResourceProxy.h`](Source/Runtime/RHI/Public/RHI/ResourceProxy.h)):
- `TResourceProxy<T>` — `CACHELINE_ALIGNED` with atomic state machine (`EState::Initial/Failed/Created`), instance ID for ABA detection, and reference counting. Optional tracing via `USE_PPE_RHITRACE`.

**Command buffer** ([`CommandBuffer.h`](Source/Runtime/RHI/Public/RHI/CommandBuffer.h)):
- `ICommandBuffer` — Task-based command recording: `Task(FSubmitRenderPass)`, `Task(FDispatchCompute)`, `Task(FTraceRays)`, etc.
- Render pass sub-tasks: `Task(FLogicalPassID, FDrawVertices/FDrawIndexed/FDrawMeshes...)`
- Staging buffer allocator via `StagingAlloc()`.

### Pipeline System — Custom Shader Pipeline with Reflection & Attributes

`FPipelineDesc` uses a **custom attribute system** via `PPE_DEFINE_AUTOSTRUCT()` macros (generates `operator==`, `hash_value`, `ASSUME_TYPE_AS_POD`). ([`PipelineDesc.h`](Source/Runtime/RHI/Public/RHI/PipelineDesc.h), 589 lines)

| Feature | Description |
|---|---|
| **`FPipelineDesc`** | Base pipeline descriptor with `FDescriptorSets` (fixed-size stack), `FPushConstants` (fixed-size flat map), `FShaders` (associative vector by shader type), `FVertexAttributes` (fixed stack) |
| **Uniform system** | `std::variant<>` with `FPipelineDescUniform<T>`: `FTextureUniform`, `FSamplerUniform`, `FImageUniform`, `FUniformBufferUniform`, `FStorageBufferUniform`, `FRayTracingSceneUniform`. Each has `FUniformID`, `FBindingIndex`, `ArraySize`, `EShaderStages` |
| **Four pipeline types** | `FGraphicsPipelineDesc`, `FComputePipelineDesc`, `FMeshPipelineDesc`, `FRayTracingPipelineDesc` — all inherit from `FPipelineDesc` |
| **Custom attributes** | `EShaderLangFormat` encodes **API** (Vulkan/OpenGL/DirectX), **version** (GLSL 450, VKSL 120, SPIRV 100), **storage** (Source/Binary/Executable), **format** (HighLevel/SPIRV), and **flags** (`EnableDebugTrace`, `EnableProfiling`, `EnableTimeMap`) in a single `u32` bitfield |
| **Shader sources** | `FShaderData` with `FShaderSource` (load GLSL/HLSL), `IShaderData` (compiled: fingerprint-based caching via `FShaderDataFingerprint`). RTTI-reflected for automatic serialization |
| **`FPipelineResources`** | `TPipelineElementArray<T>` with fixed-capacity inline storage. `FDescriptorSet` maps `FDescriptorSetID` → `FVariantResource` (variant of all resource types) |
| **Vertex input** | `FVertexAttributes` with `FVertexAttribute` (location, format, offset). Auto-generated from `FGenericMesh` vertex layout |
| **Push constants** | `FPushConstantID` + `FPushConstant` (offset + size). `FPushConstants` as `TFixedSizeFlatMap` |
| **Fragment output** | `FPipelineFragmentOutput` with `FAttachmentIndex` and `EFragmentOutput` type. `FFragmentOutputs` as fixed stack |
| **Specialization** | `FSpecializationConstant` with `FSpecializationID` + `u32 Index`. `FSpecializationConstants` as fixed flat map |
| **`PPE_DEFINE_AUTOSTRUCT`** | Generates POD structs with `operator==`, `hash_value`, `ASSUME_TYPE_AS_POD` for all pipeline-related types. Eliminates boilerplate for 15+ POD structs |

**Design details**:
- `GlslangToSpv()` isn't deterministic (differs by 1 int on recompile) — handled via **fingerprint cache** using xxHash. This took a while to diagnose.
- Vulkan shader modules **set debug names** for RenderDoc/Vulkan validation layers via `SetVulkanObjectName()`
- Pipeline compiler exposed through **`IRHIService`**: `ExposeIPipelineCompiler()` allows the RHI to lazily compile pipelines
- **Custom attributes on shader parameters** enable automatic `FUniformBuffer` layout generation from RTTI metadata
- `EShaderLangFormat` handles `#include` resolution and preprocessor defines in GLSL via the PipelineCompiler
- `VulkanDebuggableShaderData` extracts debug symbols from SPIRV for **RenderDoc integration** (`VK_LAYER_KHRONOS_shader_object`)
- Pipeline **state objects** use `ERenderState` enums (blend, depth, stencil, rasterizer) with `EResourceState` for resource access masks
- `PPE_DEFINE_AUTOSTRUCT()` macros replaced old `PPE_DEFINE_AUTOPOD()` with better constexpr support

**Task descriptors** — CRTP-based fluent builder pattern:
- `TFrameTaskDesc<_Self>` — Base with Dependencies, Name, DebugColor
- `TDrawTaskDesc<_Self>` — Draw-specific with SetName/SetDebugColor
- `TDrawCallDesc<_Self>` — Full draw state: Resources, PushConstants, Scissors, etc.
- `PPE_RHI_EACH_DYNAMICSTATE(DEF_DYNAMICSTATE_SET)` macro for DRY dynamic state setters

**Ray tracing** — Dedicated descriptor types: `FRayTracingDesc`, `FRayTracingTask`, `FRayTracingEnums`

**Config** — Mobile vs. desktop constants: MaxColorBuffers (4 vs. 8), GPUPageSizeInMb (64 vs. 256)

### Vulkan Backend

The **RHIVulkan** extension provides a complete Vulkan implementation ([`Source/Extensions/RHIVulkan/`](Source/Extensions/RHIVulkan/)):

| Component | Description |
|---|---|
| `FVulkanTargetRHI` | Implements `ITargetRHI`, returns `ETargetRHI::Vulkan`. Feature query system: `RecommendedFeatures()`, `RequiresFeature()`, `SupportsFeature()` |
| `FVulkanRHIService` | Implements `IRHIService`. Composition: `FVulkanInstance`, `FVulkanDeviceInfo`, `PVulkanFrameGraph`. Swapchain management |
| Vulkan subsystems | `Buffer/`, `Command/`, `Common/`, `Debugger/`, `Descriptors/`, `Image/`, `Instance/`, `Memory/`, `Pipeline/`, `RayTracing/`, `RenderPass/` |
| `FVulkanExternalObject` | Unified handle wrapper converting between Vulkan handles and engine external object types |
| `FVulkanDescriptorSets` | Uses `TFixedSizeStack<VkDescriptorSet, MaxDescriptorSets>` to avoid allocations |
| Custom Vulkan C++ wrapper | `vk::global_api`, `vk::instance_api`, `vk::device_api` for versioned API access |

**Custom Vulkan header generation** ([`Build/internal/hal/generic/VulkanSDK.go`](Build/internal/hal/generic/VulkanSDK.go), 1,164 lines) — The build system parses [`vulkan-exports.inl`](Source/External/vulkan/Public/vulkan-exports.inl) (a declarative bindings file with ~640 lines of `VK_*_FUNCTION` / `VK_*_EXTENSION` macros) and the official Vulkan headers to generate two files at build time:

| Generated file | Purpose |
|---|---|
| `vulkan-exports.generated.h` | C++ header in the `vk` namespace: `api_version` enum, `instance_extension` / `device_extension` enums with `_set` bitmasks (`TFixedSizeBitMask`), `exported_api` / `global_api` / `instance_api` / `device_api` structs (function pointer members + inline wrappers), `instance_extensions_require(device_extension_set)` for transitive dependency resolution |
| `vulkan-exports.generated.cpp` | Implementation: `*_name()` / `*_from()` string↔enum conversion via `hash_strI_constexpr()` switch tables, `*_available()` to probe supported extensions, `*_require()` to expand transitive dependencies (reverse-order iteration over sorted extensions), `attach_return_error()` for function pointer resolution via `vkGetInstanceProcAddr` / `vkGetDeviceProcAddr` with backward compatibility aliasing |

The bindings file (`vulkan-exports.inl`) declares every Vulkan function at its correct scope level:
- `VK_EXPORTED_FUNCTION` — dynamically loaded from the Vulkan loader DLL
- `VK_GLOBAL_LEVEL_FUNCTION` — instance-independent (`vkCreateInstance`, `vkEnumeratePhysicalDevices`)
- `VK_INSTANCE_LEVEL_FUNCTION(fn, version)` — core Vulkan version functions
- `VK_INSTANCE_LEVEL_EXTENSION(ext_name)` / `VK_INSTANCE_LEVEL_FUNCTION_FROM_EXTENSION(fn, ext)` — extension-specific functions with dependency tracking
- `VK_INSTANCE_LEVEL_BACKWARD_COMPATIBILITY` — version aliasing (e.g., `vkGetPhysicalDeviceFeatures2` → `vkGetPhysicalDeviceFeatures2KHR` when core version >= 1.1)
- Same pattern repeated for `VK_DEVICE_LEVEL_*`

The generated `vk` namespace provides **type-safe, version-aware API access**: `vk::device_fn` wraps `device_api` which wraps `instance_api` which wraps `global_api`, so every call chain has the right function pointers available. Extension availability is tracked as `FVulkanInstanceExtensionSet` / `FVulkanDeviceExtensionSet` bitmasks, with `instance_extensions_require()` computing transitive instance extension dependencies from a set of device extensions. **No manual function pointer loading or extension string management** — the build system generates it all from the declarative bindings file.

**Design details**: Extension dependency resolution iterates in reverse order (extensions are sorted by dependencies), so when `EXT_descriptor_indexing` is enabled, its required extensions are automatically added. The `attach_return_error()` functions return the name of the first missing function pointer, making initialization failures self-diagnosing. Backward compatibility aliases are resolved at runtime based on the negotiated Vulkan version.

---

## Application Framework

A **modular service-based application framework** with platform abstraction ([`Source/Runtime/Application/Public/`](Source/Runtime/Application/Public/)).

**Core entry points** ([`Application.h`](Source/Runtime/Application/Public/Application.h)):
- `RunningApp()` — Global singleton access
- `LaunchApplication()` — Bootstrap function

**Application hierarchy** ([`Application/`](Source/Runtime/Application/Public/Application/)):
- **`FApplicationBase`** — Inherits from `FPlatformApplication` (platform-specific alias via HAL macro dispatch). Lifecycle: `Start() → Run() → Shutdown()` with `FTimeline` for time tracking.
- **`FApplicationWindow`** — Extends base with RHI, Input, and Window services. Composition via smart pointers: `PMainWindow`, `UInputService`, `URHIService`, `UWindowService`. **Rendering hook**: `virtual void Render(RHI::IFrameGraph& fg, FTimespan dt)`.
- **`FApplicationConsole`** — Lightweight console variant with `Daemonize()` support.

**HAL pattern** — Same macro-based platform dispatch as Core:
```cpp
#include PPE_HAL_MAKEINCLUDE(PlatformApplication)
using FPlatformApplication = CONCAT3(F, TARGET_PLATFORM, Application);
```
Resolves to `FWindowsApplication`, `FGLFWApplication`, `FLinuxApplication`, or `FGenericApplication` at compile time.

**Input system** ([`Input/`](Source/Runtime/Application/Public/Input/)) — Three-layer hierarchy:

| Layer | Components | Description |
|---|---|---|
| **Devices** | [`InputDevice.h`](Source/Runtime/Application/Public/Input/InputDevice.h), `Device/` | `IInputDevice` base interface with `PostInputMessages()` for polling. Device types: `FKeyboardDevice`, `FMouseDevice`, `FGamepadDevice` — each with state structs (`FKeyboardState`, `FMouseState`, `FGamepadState`). Key types: `FInputKey` (discriminated union: `FKeyboardKey`, `FMouseButton`, `FGamepadButton`), `FInputValue` (digital, 1D axis, 2D axis, 3D axis). `FInputMessage` carries `EInputMessageEvent` (Pressed, Released, Repeat, DoubleClick, Axis) with delta time and device ID. `FilteredAnalog` provides debounced analog input |
| **Actions** | [`Action/InputAction.h`](Source/Runtime/Application/Public/Input/Action/InputAction.h), [`Action/InputMapping.h`](Source/Runtime/Application/Public/Input/Action/InputMapping.h), [`Action/InputListener.h`](Source/Runtime/Application/Public/Input/Action/InputListener.h) | `FInputAction` defines logical game actions with `EInputTriggerEvent` lifecycle (Inactive → Started → Triggered → Completed). `FInputMapping` binds physical keys to actions. `FInputListener` holds a priority-sorted list of mappings (`FInputMappingWPriority`) and dispatches triggers to callbacks. Events: `OnStarted`, `OnTriggered`, `OnCompleted`, `Modifiers` (pre-dispatch value modification) |
| **Service** | [`InputService.h`](Source/Runtime/Application/Public/Input/InputService.h) | `IInputService` orchestrates devices, listeners, and mappings. Thread-safe events via `THREADSAFE_EVENT` macro: `OnUpdateInput`, `OnDeviceConnected`, `OnDeviceDisconnected`. Listener stack with `PushInputListener`/`PopInputListener` for input focus management. `ToggleFocus()` for modal input handling |

`FInputListener` uses `TThreadSafe<FInternalData_, EThreadBarrier::DataRaceCheck>` for lock-protected internal state with priority-sorted mappings (`FInputMappingWPriority`).

**Application loop** — `ApplicationLoop()` uses `FTimeline::Tick_Every()` for fixed-rate ticking with bisected sleep for remaining time. Background tick rate reduction when window loses focus.

**UI service** ([`UI/UIService.h`](Source/Runtime/Application/Public/UI/UIService.h)) — Abstract UI service interface, implemented by `ApplicationUI` extension (ImGui-based).

**Window system** ([`Window/`](Source/Runtime/Application/Public/Window/)) — `IWindowService`, `IMainWindow`, `IWindowListener` for window lifecycle and events.

---

## Network & Remoting

### Network Module

The engine includes a **fully custom HTTP server** (no third-party HTTP library) built from scratch to avoid dependencies. `CompletionPort` I/O on Windows for scalable async handling. Any engine subsystem exposed through RTTI can be controlled remotely.

**Socket layer** ([`Source/Runtime/Network/Public/Socket/`](Source/Runtime/Network/Public/Socket/), hand-rolled from scratch):

| Component | Description |
|---|---|
| `FSocket` | RAII socket with `FConnectionScope` helper for automatic connect/disconnect. Nagle algorithm control. Type-safe binary I/O via `ReadPOD<T>()`/`WritePOD<T>()` |
| `FSocketBuffered` | Wraps socket with I/O buffers (`_bufferI`, `_bufferO`). Template methods for type-safe binary serialization over sockets |
| `FListener` | Server-side accept with `FConnectionScope` and configurable retry logic (3 retries) |

**HTTP layer** ([`Source/Runtime/Network/Public/Http/`](Source/Runtime/Network/Public/Http/), fully hand-rolled):

| Component | Description |
|---|---|
| `FHttpClient` | HTTP client with cookie management (`FCookieMap`). `Get`/`Head`/`Post` methods. `DefaultMaxContentLength = 10MB`. Static `Read/Write` methods for serialization over `FSocketBuffered` |
| `FHttpServer` | **Custom HTTP server** with virtual callbacks: `OnConnect`, `OnRequest`, `OnDisconnect`. Worker thread pool via `Start(workerCount)` for concurrent request handling. `FCookieMap` for session management |
| `FHttpRequest` / `FHttpResponse` | Both inherit from `FHttpHeader`. Serialization via static `Read/Write` methods over buffered sockets |

`FHttpServer` supports `Access-Control-Allow-Origin: *` for CORS when serving the Swagger UI from `file://` protocol.

### Remoting Module

**RTTI-driven HTTP API** with automatic OpenAPI/Swagger generation ([`Source/Runtime/Remoting/Public/`](Source/Runtime/Remoting/Public/)). **The engine can be fully controlled remotely** by exposing any RTTI-enabled class as a REST endpoint:

| Component | Description |
|---|---|
| `FBaseEndpoint` | Dual inheritance: `RTTI::FMetaObject` + `IRemotingEndpoint`. Uses `FOperationMap` keyed by `(FName, EHttpMethod)` for O(1) dispatch |
| **Facet system** | Annotate RTTI functions with HTTP metadata: |
| `FOperationFacet` | Binds RTTI function to HTTP method + URL path. Factory: `Get(prefix, {"path"})`, `Post()`, `Put()`, `Delete()` |
| `FParameterLocationFacet` | Specifies parameter source: query string, path, body |
| `FParameterSchemaFacet` | Overrides JSON schema for parameters |
| **Automatic binding** | `RTTI_EndpointAutomaticBinding()` scans all RTTI metadata for `FOperationFacet` annotations and automatically registers operations. No manual route registration needed. |
| `EEndpointFlags` | `AutomaticBinding` (scan RTTI for facets) + `AutomaticRegister` (register in server). Combined as `Automated` |
| `FOpenAPI` | Builds JSON OpenAPI 3.0 spec in-place using `Serialize::FJson`. Schema builders: `AllOf()`, `OneOf()`, `Array()`, `Enum()`, `Object()`, `Scalar()`, `Ref()`. `DefineRTTISchemas()` auto-generates schemas from RTTI metadata |
| `FProcessEndpoint` | Built-in endpoint exposing: `About()` (engine info), `MemoryStats()` (allocator stats), `MemoryDomains()` (domain listing) — all as `RTTI::FMetaObject` subclasses for automatic serialization |
| `FSwaggerEndpoint` | Serves **Swagger UI** from the engine itself. Generates interactive API documentation from `FOpenAPI` builder. `Ctrl+click` link in systray opens browser |
| **Listing endpoint** | When no arguments provided, lists all RTTI classes, enums, namespaces, and methods — searchable via autocomplete |

**Zero boilerplate**: Annotate any RTTI function with `FOperationFacet::Get("/api/v1/path")` and it becomes a live REST endpoint with full OpenAPI schema — no manual routing, no registration code.

---

## Content Pipeline

A **DAG-based asset processing pipeline** with multi-threaded execution, featuring file dependency tracking, incremental builds via revision control, and parallel shader compilation.

### BuildGraph

The DAG-based build system where each `FBuildNode` represents an asset processing task with three dependency types:

| Component | Description |
|---|---|
| `FBuildGraph` | DAG of build nodes. `HASHMAP(FFilename, SBuildNode)` for O(1) reverse dependency lookup. `FBuildRevision` for incremental builds (only rebuild changed assets) |
| `FBuildNode` (`SBuildNode`) | Base node with **three dependency lists**: `StaticDeps` (compile-time), `DynamicDeps` (module-level), `RuntimeDeps` (DLL-loaded). Uses `Rtti::FOpaqueData` for per-node typed state. `FAtomicPhaseLock` for thread-safe state transitions |
| `FBuildEnvironment` | Build context carrying configuration, source control status, and file fingerprints |
| `FBuildExecutor` / `IBuildExecutor` | Execution strategy interface. `MakeParallelExecutor(FTaskManager&)` creates a **parallel executor** that dispatches nodes via the fiber-based task scheduler |
| `FBuildCache` / `IBuildCache` | Persistent cache with `FileSystemBuildCache` implementation. Stores fingerprinted build results, detects `SourceControl` modified files via git status |
| `FBuildLog` | Structured logging for build operations with `EBuildResult` (Unbuilt/Building/Built/Failed/Cleaned) |
| Node types | `FFileNode` (single file), `FDirectoryListNode` (directory glob), `FCommandNode` (child process execution with `CompletionPort` I/O on Windows) |
| Operations | `ScanAll → BuildAll / CleanAll`. `Scan()` checks file modifications via source control, `Build()` processes dirty nodes, `Clean()` removes outputs |
| Incremental builds | `FBuildRevision` tracks per-node revision. `SourceControlFolderStatus` provides git branch/revision/timestamp. Only rebuilds when inputs change |

**Design details**:
- Source control integration uses `git` directly to detect modified files — not just timestamps, which are unreliable across distributed builds.
- Cache uses file fingerprints (not just names) for cache invalidation.
- Runtime dependencies allow dynamically loaded modules to participate in the build graph.

### PipelineCompiler

Full shader compilation pipeline with **SPIRV reflection** and **pipeline state caching**:

| Component | Description |
|---|---|
| `FPipelineCompilerModule` | Implements `IModuleInterface` with lifecycle hooks |
| `FVulkanPipelineCompiler` | GLSL/HLSL → SPIRV via **glslang** (integrated as external module). Handles `#include` resolution, preprocessor defines |
| `FVulkanSpirvCompiler` | SPIRV → `FVulkanShaderModule` with debug info extraction. Uses **spirv-tools** for validation and optimization |
| `FVulkanDebuggableShaderData` | Extracts debug symbols from SPIRV for **RenderDoc** and **VK_LAYER_KHRONOS_shader_object** integration |
| Pipeline reflection | **Custom attribute system** via `Rtti::FMetaClass` — shader parameters are reflected at runtime for automatic binding. `IPipelineCompiler` interface exposed through `IRHIService` |
| Shader cache | Content-addressable cache using **xxHash** fingerprints. Avoids recompilation when source/defines haven't changed. Handles `EShaderCompilationFlags::Quiet` for silent builds |
| Multi-threaded | Shader compilation parallelized via `FTaskManager`. Each shader compiles independently, with cache shared across threads |
| Pipeline state objects | `FPipelineDesc` variants: `FGraphicsPipelineDesc`, `FComputePipelineDesc`, `FMeshPipelineDesc`, `FRayTracingPipelineDesc`. Uniforms use `std::variant<>` with automatic reflection |

### MeshBuilder

| Component | Description |
|---|---|
| `FMeshBuilderService` | Mesh processing pipeline with `FGenericMesh` representation |
| Formats | **WaveFront OBJ** (full material support), **Polygon File Format (PLY)** |
| `FGenericMesh` | Vertex/attribute system with interleaved or separate buffers. Supports `FVertexInputState` generation for RHI |
| `FGeometricPrimitives` | Procedural mesh generation (cubes, spheres, etc.) |
| `FAsset_Geometry` | Asset representation for geometry data, integrates with `FTransform` and `FMaterial` |

### Texture Pipeline

| Component | Description |
|---|---|
| `FTextureService` | Texture processing with lazy loading and **TextureCache** (clearable at runtime via Ctrl+F7) |
| Compression | **BC1/BC2/BC4/BC5** via STB DXT. **PassthroughCompression** for uncompressed. Supports DDS (DirectDraw Surface) format with **mip map generation** for non-square textures |
| Image loading | **STBImageFormat** for PNG/JPG/BMP/TGA. Handles `DX10` extended format headers in DDS |
| Types | Texture2D, 2DArray, 3D, Cube, CubeArray. **TextureReloader** supports hot-reloading without restart |
| `FMaterial` / `FMaterialParameter` | Material system with texture parameters, camera frustum rays, eye-up vectors. Supports `FMaterialParameterCamera_FrustumRays` and `FMaterialParameterCamera_EyeUp` |

**Design details**:
- Texture cache uses `TSparseArray<>` for generational-safe references.
- Mip map support clamps to 1×1 minimum for non-square textures.
- DDS loader handles DX10 format extensions for modern texture formats.
- `FMaterial` parameters use `TRelativeView<>` for serialization-safe texture references.

### PipelineReflection

SPIRV introspection and pipeline state validation. Uses **spirv-reflect** and **spirv-cross** (external modules) for:
- Cross-compilation (SPIRV → GLSL/HLSL/MSL) for debugging
- Resource binding reflection for automatic descriptor set layout
- Pipeline state object validation

**Multi-threading**: All content pipeline modules expose `DutyCycle()` for frame-based async processing and `ReleaseMemory()` for deferred memory cleanup. The build graph leverages the task system ([`FTaskManager`](Source/Runtime/Core/Public/Thread/Task.h), [`FGlobalThreadPool`](Source/Runtime/Core/Public/Thread/ThreadPool.h)) for parallel node execution. `FBuildExecutor::MakeParallelExecutor()` creates a work-stealing executor that dispatches `FBuildNode` instances across the fiber pool.

---

## Build System

A **custom Go-based build system** ([`PPE.go`](PPE.go), 346 lines) with advanced features:

**Core concepts** ([`PPE.go`](PPE.go)):

| Concept | Description |
|---|---|
| **Archetypes** | Reusable module templates: `PPE/Headers`, `PPE/External`, `PPE/Module`, `PPE/Program/{Runtime,Shipping,Tools,Developer}` |
| **Code generation** | Generates `BuildModules.generated.h` and `BuildVersion.generated.h` from module metadata |
| **HAL handling** | Automatically excludes non-target platform HAL files from compilation |
| **PCH support** | Monolithic precompiled header per module (`stdafx.h`/`stdafx.cpp`) |
| **Unity builds** | Configurable per module (disabled for programs) |
| **Tag-based configuration** | Build flags (DEBUG\|DEVEL) control platform-specific defines and dependencies |
| **RTTI control** | `PPE_HAS_CXXRTTI` define based on unit compilation settings |
| **Build usage types** | Runtime, Shipping, Tools, Developer — each with different archetype settings |

**Generated headers**:

| Header | Contents |
|---|---|
| `BuildVersion.generated.h` | Git branch, revision, build timestamp as `constexpr`. Was initially transitively included in every TU — a refactor to isolate it produced a measurable compile-time improvement. |
| `BuildModules.generated.h` | Module registration functions (`RegisterStaticModules()`, `UnregisterStaticModules()`, `RegisterDynamicModules()`, `UnregisterDynamicModules()`), dependency lists as `constexpr std::array`, DLL import/export symbols |

**Build features** (documented in TODO.md):
- Deterministic builds (MSVC `/d1nodatetime`, `/Brepro`, `/experimental:deterministic`, `/pdbaltpath:%_PDB%`)
- Build object caching with fingerprint-based cache keys
- Distributed build with worker discovery, WebDAV file sharing, and I/O detouring via MinHook/Detours

**Entry point**: [`configure.bat`](configure.bat) / [`configure.sh`](configure.sh) builds the Go binary and runs `PPE configure -and vscode -and vcxproj -Summary`.

---

## Tools & Utilities

| Tool | Purpose |
|---|---|
| **BuildRobot** ([`Programs/BuildRobot/`](Source/Programs/BuildRobot/)) | CI/asset build orchestrator — automates asset builds, project generation, and distributed compilation |
| **ShaderToy** ([`Programs/ShaderToy/`](Source/Programs/ShaderToy/)) | Shader playground for testing and iteration |
| **VoxelCube** ([`Programs/VoxelCube/`](Source/Programs/VoxelCube/)) | Voxel rendering demo application |
| **WindowTest** ([`Programs/WindowTest/`](Source/Programs/WindowTest/)) | Windowing and input test harness |
| **UnitTest** ([`Programs/UnitTest/`](Source/Programs/UnitTest/)) | Test runner for the engine's unit test suite |
| **IOWrapperTest** ([`Programs/IOWrapperTest/`](Source/Programs/IOWrapperTest/)) | I/O wrapper test application |
| **IODetouring** ([`Tools/IODetouring/`](Source/Tools/IODetouring/)) | Win32 I/O interception via MinHook/Microsoft Detours — used for distributed build file system wrapping |
| **IOWrapper** ([`Tools/IOWrapper/`](Source/Tools/IOWrapper/)) | File system wrapper that redirects I/O to remote WebDAV servers for distributed builds |

**Shell completions** ([`Extras/`](Extras/)): Available for bash, fish, nushell, and PowerShell.

**External dependencies** (20 integrated as modules):

| Library | Purpose |
|---|---|
| `vulkan` | Vulkan SDK headers and loader |
| `glslang` | GLSL/HLSL → SPIRV compiler |
| `spirv-cross` | SPIRV → GLSL/HLSL/MSL cross-compiler |
| `spirv-reflect` | SPIRV reflection/introspection |
| `spirv-tools` | SPIRV optimization and validation |
| `spirv-headers` | SPIRV specification headers |
| `glsl_trace` | GLSL tracing/debugging |
| `imgui` | Immediate mode GUI |
| `vma` | Vulkan Memory Allocator (GPUOpen) |
| `renderdoc` | RenderDoc API integration |
| `stb` | stb_image, stb_image_write, stb_truetype |
| `lz4` | LZ4 compression |
| `xxHash` | Fast non-cryptographic hashing |
| `farmhash` | Google's farm hash functions |
| `double-conversion` | IEEE double-precision to string conversion |
| `detours` | Microsoft Detours API hooking |
| `minhook` | Minimalistic API hooking library |
| `iaca` | Intel Architecture Code Analyzer |
| `vstools` | Visual Studio tooling integration |

---

## Additional Resources

- **Doxygen documentation**: Run [`Doc/doxygen.bat`](Doc/doxygen.bat) to generate full API documentation
- **Shell completions**: Available in [`Extras/`](Extras/) for bash, fish, nushell, and PowerShell
- **Benchmarks**: [`Extras/Benchmarks/`](Extras/Benchmarks/) contains performance comparison data
- **Debugging**: [`Extras/Debug/PPE.natvis`](Extras/Debug/PPE.natvis) provides Visual Studio debugger visualizations
- **Profiling**: [`cpu.pprof`](cpu.pprof), [`mem.pprof`](mem.pprof) — pprof-compatible CPU and memory profiles
- **Future directions** (see [`TODO.md`](TODO.md)): Gaussian splatting, Nanite-like clustering, path tracing, volumetric clouds, screen-space horizon GI, REStir/DDGI, physics engine (PBD), HyperLogLog, Bloom filter, concurrent Hopscotch hashing

---

## License

Proprietary. All rights reserved.

---

*4,155 commits and ~286,000 lines of custom engine C++ across 1,505 source files. Every subsystem built from first principles — not because it was necessary, but because understanding how things work at the lowest level is the only way I know how to build things at the highest level.*