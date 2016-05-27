#ifndef MEMORY_DOMAIN_IMPL
#   define MEMORY_DOMAIN_IMPL(_Name, _Parent)
#endif

// Here goes the list of all the domains

//------------------------------------------------------------------------------
MEMORY_DOMAIN_IMPL(Core,                    Global)
//------------------------------------------------------------------------------
MEMORY_DOMAIN_IMPL(Alloca,                  Core)
MEMORY_DOMAIN_IMPL(Container,               Core)
MEMORY_DOMAIN_IMPL(Diagnostic,              Core)
MEMORY_DOMAIN_IMPL(Event,                   Core)
MEMORY_DOMAIN_IMPL(FileSystem,              Core)
MEMORY_DOMAIN_IMPL(Internal,                Core)
MEMORY_DOMAIN_IMPL(Maths,                   Core)
MEMORY_DOMAIN_IMPL(Pool,                    Core)
MEMORY_DOMAIN_IMPL(RTTI,                    Core)
MEMORY_DOMAIN_IMPL(Singleton,               Core)
MEMORY_DOMAIN_IMPL(Stream,                  Core)
MEMORY_DOMAIN_IMPL(String,                  Core)
MEMORY_DOMAIN_IMPL(Task,                    Core)
MEMORY_DOMAIN_IMPL(ThreadLocal,             Core)
MEMORY_DOMAIN_IMPL(Token,                   Core)
//------------------------------------------------------------------------------
MEMORY_DOMAIN_IMPL(Serialize,               Core)
//------------------------------------------------------------------------------
MEMORY_DOMAIN_IMPL(Grammar,                 Serialize)
MEMORY_DOMAIN_IMPL(Lexer,                   Serialize)
MEMORY_DOMAIN_IMPL(Parser,                  Serialize)
MEMORY_DOMAIN_IMPL(Serializer,              Serialize)
MEMORY_DOMAIN_IMPL(Transaction,             Serialize)
//------------------------------------------------------------------------------
MEMORY_DOMAIN_IMPL(Graphics,                Core)
//------------------------------------------------------------------------------
MEMORY_DOMAIN_IMPL(Device,                  Graphics)
MEMORY_DOMAIN_IMPL(Geometry,                Graphics)
MEMORY_DOMAIN_IMPL(Index,                   Graphics)
MEMORY_DOMAIN_IMPL(Resource,                Graphics)
MEMORY_DOMAIN_IMPL(Shader,                  Graphics)
MEMORY_DOMAIN_IMPL(Texture,                 Graphics)
MEMORY_DOMAIN_IMPL(Vertex,                  Graphics)
//------------------------------------------------------------------------------
MEMORY_DOMAIN_IMPL(Pixmap,                  Core)
MEMORY_DOMAIN_IMPL(Image,                   Pixmap)
//------------------------------------------------------------------------------
MEMORY_DOMAIN_IMPL(ContentPipeline,         Core)
MEMORY_DOMAIN_IMPL(Generation,              ContentPipeline)
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
MEMORY_DOMAIN_IMPL(Engine,                  Core)
//------------------------------------------------------------------------------
MEMORY_DOMAIN_IMPL(Effect,                  Engine)
MEMORY_DOMAIN_IMPL(Lighting,                Engine)
MEMORY_DOMAIN_IMPL(Material,                Engine)
MEMORY_DOMAIN_IMPL(Mesh,                    Engine)
MEMORY_DOMAIN_IMPL(MeshGeneration,          Engine)
MEMORY_DOMAIN_IMPL(Render,                  Engine)
MEMORY_DOMAIN_IMPL(Scene,                   Engine)
MEMORY_DOMAIN_IMPL(Service,                 Engine)
MEMORY_DOMAIN_IMPL(World,                   Engine)
//------------------------------------------------------------------------------
MEMORY_DOMAIN_IMPL(Logic,                   Core)
//------------------------------------------------------------------------------
MEMORY_DOMAIN_IMPL(Component,               Logic)
MEMORY_DOMAIN_IMPL(Entity,                  Logic)
MEMORY_DOMAIN_IMPL(System,                  Logic)
//------------------------------------------------------------------------------
MEMORY_DOMAIN_IMPL(Application,             Core)
//------------------------------------------------------------------------------
MEMORY_DOMAIN_IMPL(Window,                  Application)
//------------------------------------------------------------------------------
