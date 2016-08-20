#ifndef MEMORY_DOMAIN_IMPL
#   define MEMORY_DOMAIN_IMPL(_Name, _Parent)
#endif
#ifndef MEMORY_DOMAIN_COLLAPSABLE_IMPL
#   define MEMORY_DOMAIN_COLLAPSABLE_IMPL(_Name, _Parent) \
        MEMORY_DOMAIN_IMPL(_Name, _Parent)
#endif

// Here goes the list of all the domains

//------------------------------------------------------------------------------
MEMORY_DOMAIN_IMPL(Core,                                Global)
//------------------------------------------------------------------------------
MEMORY_DOMAIN_IMPL(Alloca,                              Core)
MEMORY_DOMAIN_IMPL(Container,                           Core)
MEMORY_DOMAIN_IMPL(Diagnostic,                          Core)
MEMORY_DOMAIN_IMPL(Event,                               Core)
MEMORY_DOMAIN_IMPL(FileSystem,                          Core)
MEMORY_DOMAIN_IMPL(Internal,                            Core)
MEMORY_DOMAIN_IMPL(Maths,                               Core)
MEMORY_DOMAIN_IMPL(Pool,                                Core)
MEMORY_DOMAIN_IMPL(RTTI,                                Core)
MEMORY_DOMAIN_IMPL(Singleton,                           Core)
MEMORY_DOMAIN_IMPL(Stream,                              Core)
MEMORY_DOMAIN_IMPL(String,                              Core)
MEMORY_DOMAIN_IMPL(Task,                                Core)
MEMORY_DOMAIN_IMPL(ThreadLocal,                         Core)
MEMORY_DOMAIN_IMPL(Token,                               Core)
//------------------------------------------------------------------------------
MEMORY_DOMAIN_IMPL(Serialize,                           Core)
MEMORY_DOMAIN_COLLAPSABLE_IMPL(Grammar,                 Serialize)
MEMORY_DOMAIN_COLLAPSABLE_IMPL(Lexer,                   Serialize)
MEMORY_DOMAIN_COLLAPSABLE_IMPL(Parser,                  Serialize)
MEMORY_DOMAIN_COLLAPSABLE_IMPL(Serializer,              Serialize)
MEMORY_DOMAIN_COLLAPSABLE_IMPL(Transaction,             Serialize)
MEMORY_DOMAIN_COLLAPSABLE_IMPL(XML,                     Serialize)
//------------------------------------------------------------------------------
MEMORY_DOMAIN_IMPL(Graphics,                            Core)
MEMORY_DOMAIN_COLLAPSABLE_IMPL(Device,                  Graphics)
MEMORY_DOMAIN_COLLAPSABLE_IMPL(Index,                   Graphics)
MEMORY_DOMAIN_COLLAPSABLE_IMPL(Resource,                Graphics)
MEMORY_DOMAIN_COLLAPSABLE_IMPL(Shader,                  Graphics)
MEMORY_DOMAIN_COLLAPSABLE_IMPL(Texture,                 Graphics)
MEMORY_DOMAIN_COLLAPSABLE_IMPL(Value,                   Graphics)
MEMORY_DOMAIN_COLLAPSABLE_IMPL(Vertex,                  Graphics)
//------------------------------------------------------------------------------
MEMORY_DOMAIN_IMPL(Lattice,                             Core)
MEMORY_DOMAIN_COLLAPSABLE_IMPL(Collada,                 Lattice)
MEMORY_DOMAIN_COLLAPSABLE_IMPL(GenericMesh,             Lattice)
MEMORY_DOMAIN_COLLAPSABLE_IMPL(GenericMaterial,         Lattice)
//------------------------------------------------------------------------------
MEMORY_DOMAIN_IMPL(Pixmap,                              Core)
MEMORY_DOMAIN_COLLAPSABLE_IMPL(DXTImage,                Pixmap)
MEMORY_DOMAIN_COLLAPSABLE_IMPL(FloatImage,              Pixmap)
MEMORY_DOMAIN_COLLAPSABLE_IMPL(Image,                   Pixmap)
//------------------------------------------------------------------------------
MEMORY_DOMAIN_IMPL(ContentPipeline,                     Core)
MEMORY_DOMAIN_COLLAPSABLE_IMPL(Generation,              ContentPipeline)
//------------------------------------------------------------------------------
MEMORY_DOMAIN_IMPL(Logic,                               Core)
MEMORY_DOMAIN_COLLAPSABLE_IMPL(Component,               Logic)
MEMORY_DOMAIN_COLLAPSABLE_IMPL(Entity,                  Logic)
MEMORY_DOMAIN_COLLAPSABLE_IMPL(System,                  Logic)
//------------------------------------------------------------------------------
MEMORY_DOMAIN_IMPL(Application,                         Core)
MEMORY_DOMAIN_COLLAPSABLE_IMPL(Window,                  Application)
//------------------------------------------------------------------------------
