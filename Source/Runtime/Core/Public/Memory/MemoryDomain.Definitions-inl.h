#ifndef MEMORYDOMAIN_IMPL
#   define MEMORYDOMAIN_IMPL(_Name, _Parent)
#endif
#ifndef MEMORYDOMAIN_GROUP_IMPL
#   define MEMORYDOMAIN_GROUP_IMPL(_Name, _Parent) MEMORYDOMAIN_IMPL(_Name, _Parent)
#endif
#ifndef MEMORYDOMAIN_COLLAPSABLE_IMPL
#   define MEMORYDOMAIN_COLLAPSABLE_IMPL(_Name, _Parent) MEMORYDOMAIN_IMPL(_Name, _Parent)
#endif

//------------------------------------------------------------------------------
// Gpu memory
//------------------------------------------------------------------------------
MEMORYDOMAIN_IMPL(DeviceLocal,                          GpuMemory)
MEMORYDOMAIN_IMPL(DeviceHostCached,                     GpuMemory)
MEMORYDOMAIN_IMPL(DeviceHostCoherent,                   GpuMemory)
MEMORYDOMAIN_IMPL(DeviceHostVisible,                    GpuMemory)
MEMORYDOMAIN_IMPL(DeviceProtected,                      GpuMemory)
MEMORYDOMAIN_IMPL(DeviceUnknown,                        GpuMemory)
//------------------------------------------------------------------------------
// Reserved memory
//------------------------------------------------------------------------------FSys
MEMORYDOMAIN_GROUP_IMPL(Bookkeeping,                    ReservedMemory)
MEMORYDOMAIN_COLLAPSABLE_IMPL(MipmapCache,              Bookkeeping)
MEMORYDOMAIN_COLLAPSABLE_IMPL(SmallPoolInfo,            Bookkeeping)
MEMORYDOMAIN_IMPL(Fibers,                               ReservedMemory)
MEMORYDOMAIN_IMPL(LeakDetector,                         ReservedMemory)
MEMORYDOMAIN_GROUP_IMPL(Malloc,                         ReservedMemory)
MEMORYDOMAIN_COLLAPSABLE_IMPL(SmallTables,              Malloc)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Heaps,                    Malloc)
MEMORYDOMAIN_COLLAPSABLE_IMPL(MediumHeap,               Heaps)
MEMORYDOMAIN_COLLAPSABLE_IMPL(LargeHeap,                Heaps)
MEMORYDOMAIN_COLLAPSABLE_IMPL(VeryLargeBlocks,          Malloc)
MEMORYDOMAIN_IMPL(PageAllocator,                        ReservedMemory)
MEMORYDOMAIN_IMPL(SystemMalloc,                         ReservedMemory)
//------------------------------------------------------------------------------
// Used memory
//------------------------------------------------------------------------------
MEMORYDOMAIN_GROUP_IMPL(Core,                           UsedMemory)
//------------------------------------------------------------------------------
MEMORYDOMAIN_IMPL(Alloca,                               Core)
MEMORYDOMAIN_IMPL(Benchmark,                            Core)
MEMORYDOMAIN_IMPL(BulkData,                             Core)
MEMORYDOMAIN_IMPL(Compress,                             Core)
MEMORYDOMAIN_IMPL(Config,                               Core)
MEMORYDOMAIN_IMPL(Container,                            Core)
MEMORYDOMAIN_IMPL(Diagnostic,                           Core)
MEMORYDOMAIN_IMPL(Event,                                Core)
MEMORYDOMAIN_IMPL(Function,                             Core)
MEMORYDOMAIN_IMPL(HAL,                                  Core)
MEMORYDOMAIN_GROUP_IMPL(Internal,                       Core)
MEMORYDOMAIN_COLLAPSABLE_IMPL(BitmapHeap,               Internal)
MEMORYDOMAIN_COLLAPSABLE_IMPL(BitmapPage,               Internal)
MEMORYDOMAIN_COLLAPSABLE_IMPL(SizePTrie,                Internal)
MEMORYDOMAIN_COLLAPSABLE_IMPL(SlabHeap,                 Internal)
MEMORYDOMAIN_IMPL(FileSystem,                           Core)
MEMORYDOMAIN_IMPL(Logger,                               Core)
MEMORYDOMAIN_IMPL(Maths,                                Core)
MEMORYDOMAIN_IMPL(Modular,                              Core)
MEMORYDOMAIN_IMPL(Opaq,                                 Core)
MEMORYDOMAIN_IMPL(Process,                              Core)
MEMORYDOMAIN_IMPL(Regexp,                               Core)
MEMORYDOMAIN_IMPL(SharedBuffer,                         Core)
MEMORYDOMAIN_IMPL(Stream,                               Core)
MEMORYDOMAIN_IMPL(String,                               Core)
MEMORYDOMAIN_IMPL(Task,                                 Core)
MEMORYDOMAIN_IMPL(Text,                                 Core)
MEMORYDOMAIN_IMPL(Token,                                Core)
MEMORYDOMAIN_IMPL(UnitTest,                             Core)
MEMORYDOMAIN_IMPL(Unique,                               Core)
MEMORYDOMAIN_IMPL(Unknown,                              Core)
MEMORYDOMAIN_IMPL(WeakRef,                              Core)
//------------------------------------------------------------------------------
MEMORYDOMAIN_GROUP_IMPL(RTTI,                           UsedMemory)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Any,                      RTTI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Atom,                     RTTI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(BinaryData,               RTTI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(MetaEnum,                 RTTI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(MetaClass,                RTTI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(MetaDatabase,             RTTI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(MetaFunction,             RTTI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(MetaModule,               RTTI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(MetaObject,               RTTI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(MetaTransaction,          RTTI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(NativeTypes,              RTTI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(OpaqueData,               RTTI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Script,                   RTTI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(TypeNames,                RTTI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(UserFacet,                RTTI)
//------------------------------------------------------------------------------
MEMORYDOMAIN_GROUP_IMPL(Serialize,                      UsedMemory)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Binary,                   Serialize)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Grammar,                  Serialize)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Json,                     Serialize)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Lexer,                    Serialize)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Markup,                   Serialize)
MEMORYDOMAIN_COLLAPSABLE_IMPL(MetaSerialize,            Serialize)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Parser,                   Serialize)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Transient,                Serialize)
//------------------------------------------------------------------------------
MEMORYDOMAIN_GROUP_IMPL(RHI,                            UsedMemory)
MEMORYDOMAIN_COLLAPSABLE_IMPL(RHIBatch,                 RHI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(RHIBuffer,                RHI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(RHICache,                 RHI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(RHICommand,               RHI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(RHIDescriptor,            RHI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(RHIDevice,                RHI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(RHIDebug,                 RHI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(RHIDynamicData,           RHI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(RHIFrameGraph,            RHI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(RHIImage,                 RHI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(RHIInstance,              RHI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(RHIInternal,              RHI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(RHIMisc,                  RHI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(RHIObject,                RHI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(RHIPipeline,              RHI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(RHIRawData,               RHI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(RHIRayTracing,            RHI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(RHIRenderPass,            RHI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(RHIResource,              RHI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(RHIShader,                RHI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(RHIState,                 RHI)
MEMORYDOMAIN_COLLAPSABLE_IMPL(RHIVulkan,                RHI)
//------------------------------------------------------------------------------
MEMORYDOMAIN_GROUP_IMPL(Network,                        UsedMemory)
MEMORYDOMAIN_COLLAPSABLE_IMPL(DNS,                      Network)
MEMORYDOMAIN_COLLAPSABLE_IMPL(HTTP,                     Network)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Socket,                   Network)
MEMORYDOMAIN_COLLAPSABLE_IMPL(URI,                      Network)
//------------------------------------------------------------------------------
MEMORYDOMAIN_GROUP_IMPL(ContentPipeline,                UsedMemory)
MEMORYDOMAIN_COLLAPSABLE_IMPL(BuildGraph,               ContentPipeline)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Generation,               ContentPipeline)
MEMORYDOMAIN_COLLAPSABLE_IMPL(MeshBuilder,              ContentPipeline)
MEMORYDOMAIN_COLLAPSABLE_IMPL(PipelineCompiler,         ContentPipeline)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Texture,                  ContentPipeline)
//------------------------------------------------------------------------------
MEMORYDOMAIN_GROUP_IMPL(Logic,                          UsedMemory)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Component,                Logic)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Entity,                   Logic)
MEMORYDOMAIN_COLLAPSABLE_IMPL(System,                   Logic)
//------------------------------------------------------------------------------
MEMORYDOMAIN_GROUP_IMPL(Application,                    UsedMemory)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Input,                    Application)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Launch,                   Application)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Message,                  Application)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Remoting,                 Application)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Survey,                   Application)
MEMORYDOMAIN_COLLAPSABLE_IMPL(UI,                       Application)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Viewport,                 Application)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Window,                   Application)
//------------------------------------------------------------------------------
// External memory
//------------------------------------------------------------------------------
MEMORYDOMAIN_GROUP_IMPL(External,                       UsedMemory)
MEMORYDOMAIN_IMPL(ImGui,                                External)
MEMORYDOMAIN_IMPL(LZ4,                                  External)
MEMORYDOMAIN_IMPL(VMA,                                  External)
MEMORYDOMAIN_GROUP_IMPL(STB,                            External)
MEMORYDOMAIN_COLLAPSABLE_IMPL(STBImage,                 STB)
MEMORYDOMAIN_COLLAPSABLE_IMPL(STBImageResize,           STB)
MEMORYDOMAIN_COLLAPSABLE_IMPL(STBImageWrite,            STB)
MEMORYDOMAIN_GROUP_IMPL(Vulkan,                         External)
MEMORYDOMAIN_COLLAPSABLE_IMPL(VkCommand,                Vulkan)
MEMORYDOMAIN_COLLAPSABLE_IMPL(VkObject,                 Vulkan)
MEMORYDOMAIN_COLLAPSABLE_IMPL(VkCache,                  Vulkan)
MEMORYDOMAIN_COLLAPSABLE_IMPL(VkDevice,                 Vulkan)
MEMORYDOMAIN_COLLAPSABLE_IMPL(VkInstance,               Vulkan)
//------------------------------------------------------------------------------
// User Domain
//------------------------------------------------------------------------------
MEMORYDOMAIN_IMPL(UserDomain,                           UsedMemory)
//------------------------------------------------------------------------------
// Pooled memory
//------------------------------------------------------------------------------
MEMORYDOMAIN_COLLAPSABLE_IMPL(AtomicPool,               PooledMemory)
MEMORYDOMAIN_COLLAPSABLE_IMPL(MemoryPool,               PooledMemory)
//------------------------------------------------------------------------------
// Virtual memory
//------------------------------------------------------------------------------
MEMORYDOMAIN_IMPL(VirtualAlloc,                         VirtualMemory)
//------------------------------------------------------------------------------
// Unaccounted memory
//------------------------------------------------------------------------------
MEMORYDOMAIN_IMPL(UnaccountedMalloc,                    UnaccountedMemory)
//------------------------------------------------------------------------------
