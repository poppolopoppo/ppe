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
// Reserved memory
//------------------------------------------------------------------------------
MEMORYDOMAIN_IMPL(Fibers,                               ReservedMemory)
MEMORYDOMAIN_IMPL(LeakDetector,                         ReservedMemory)
MEMORYDOMAIN_IMPL(LinearHeap,                           ReservedMemory)
MEMORYDOMAIN_GROUP_IMPL(Malloc,                         ReservedMemory)
MEMORYDOMAIN_COLLAPSABLE_IMPL(SmallTables,              Malloc)
MEMORYDOMAIN_COLLAPSABLE_IMPL(MediumMipMaps,            Malloc)
MEMORYDOMAIN_COLLAPSABLE_IMPL(LargeBlocks,              Malloc)
MEMORYDOMAIN_IMPL(MemoryPool,                           ReservedMemory)
MEMORYDOMAIN_IMPL(SizePtrie,                            ReservedMemory)
//------------------------------------------------------------------------------
// Used memory
//------------------------------------------------------------------------------
MEMORYDOMAIN_GROUP_IMPL(Core,                           UsedMemory)
//------------------------------------------------------------------------------
MEMORYDOMAIN_IMPL(Alloca,                               Core)
MEMORYDOMAIN_IMPL(Benchmark,                            Core)
MEMORYDOMAIN_IMPL(Compress,                             Core)
MEMORYDOMAIN_IMPL(Container,                            Core)
MEMORYDOMAIN_IMPL(Diagnostic,                           Core)
MEMORYDOMAIN_IMPL(Event,                                Core)
MEMORYDOMAIN_IMPL(Function,                             Core)
MEMORYDOMAIN_IMPL(Internal,                             Core)
MEMORYDOMAIN_IMPL(FileSystem,                           Core)
MEMORYDOMAIN_IMPL(Logger,                               Core)
MEMORYDOMAIN_IMPL(Maths,                                Core)
MEMORYDOMAIN_IMPL(Stream,                               Core)
MEMORYDOMAIN_IMPL(String,                               Core)
MEMORYDOMAIN_IMPL(Task,                                 Core)
MEMORYDOMAIN_IMPL(Token,                                Core)
MEMORYDOMAIN_IMPL(UnitTest,                             Core)
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
MEMORYDOMAIN_GROUP_IMPL(Graphics,                       UsedMemory)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Device,                   Graphics)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Index,                    Graphics)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Resource,                 Graphics)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Shader,                   Graphics)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Texture,                  Graphics)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Value,                    Graphics)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Vertex,                   Graphics)
//------------------------------------------------------------------------------
MEMORYDOMAIN_GROUP_IMPL(Lattice,                        UsedMemory)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Collada,                  Lattice)
MEMORYDOMAIN_COLLAPSABLE_IMPL(GenericMesh,              Lattice)
MEMORYDOMAIN_COLLAPSABLE_IMPL(GenericMaterial,          Lattice)
MEMORYDOMAIN_COLLAPSABLE_IMPL(WaveFrontObj,             Lattice)
//------------------------------------------------------------------------------
MEMORYDOMAIN_GROUP_IMPL(Pixmap,                         UsedMemory)
MEMORYDOMAIN_COLLAPSABLE_IMPL(CompressedImage,          Pixmap)
MEMORYDOMAIN_COLLAPSABLE_IMPL(FloatImage,               Pixmap)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Image,                    Pixmap)
//------------------------------------------------------------------------------
MEMORYDOMAIN_GROUP_IMPL(Network,                        UsedMemory)
MEMORYDOMAIN_COLLAPSABLE_IMPL(DNS,                      Network)
MEMORYDOMAIN_COLLAPSABLE_IMPL(HTTP,                     Network)
MEMORYDOMAIN_COLLAPSABLE_IMPL(URI,                      Network)
//------------------------------------------------------------------------------
MEMORYDOMAIN_GROUP_IMPL(ContentPipeline,                UsedMemory)
MEMORYDOMAIN_COLLAPSABLE_IMPL(BuildGraph,               ContentPipeline)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Generation,               ContentPipeline)
//------------------------------------------------------------------------------
MEMORYDOMAIN_GROUP_IMPL(Logic,                          UsedMemory)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Component,                Logic)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Entity,                   Logic)
MEMORYDOMAIN_COLLAPSABLE_IMPL(System,                   Logic)
//------------------------------------------------------------------------------
MEMORYDOMAIN_GROUP_IMPL(Application,                    UsedMemory)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Input,                    Application)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Message,                  Application)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Survey,                   Application)
MEMORYDOMAIN_COLLAPSABLE_IMPL(Window,                   Application)
//------------------------------------------------------------------------------
// External memory
//------------------------------------------------------------------------------
MEMORYDOMAIN_GROUP_IMPL(External,                       UsedMemory)
MEMORYDOMAIN_COLLAPSABLE_IMPL(LZ4,                      External)
MEMORYDOMAIN_COLLAPSABLE_IMPL(STBImage,                 External)
//------------------------------------------------------------------------------
