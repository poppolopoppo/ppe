#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace Graphics {
class IDeviceAPIContext;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class RenderState : public RefCountable {
public:
    enum class Blending {
        Opaque = 0,
        Additive,
        AlphaBlend,
        NonPremultiplied,
    };

    enum class Culling {
        None = 0,
        Clockwise,
        CounterClockwise,
    };

    enum class DepthTest {
        None = 0,
        Default,
        Read,
    };

    enum class FillMode {
        Automatic = 0,
        Solid,
        Wireframe,
    };

    RenderState();
    ~RenderState();

    RenderState(Blending blend, Culling cull, DepthTest depth,
                FillMode fill = FillMode::Automatic);

    Blending Blend() const { return _blend; }
    Culling Cull() const { return _cull; }
    DepthTest Depth() const { return _depth; }
    FillMode Fill() const { return _fill; }

    SINGLETON_POOL_ALLOCATED_DECL(RenderState);

private:
    Blending _blend;
    Culling _cull;
    DepthTest _depth;
    FillMode _fill;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
