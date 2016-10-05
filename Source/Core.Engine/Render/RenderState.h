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
class FRenderState : public FRefCountable {
public:
    enum class EBlending {
        Opaque = 0,
        Additive,
        AlphaBlend,
        NonPremultiplied,
    };

    enum class ECulling {
        None = 0,
        Clockwise,
        CounterClockwise,
    };

    enum class EDepthTest {
        None = 0,
        Default,
        Read,
    };

    enum class EFillMode {
        Automatic = 0,
        Solid,
        Wireframe,
    };

    FRenderState();
    ~FRenderState();

    FRenderState(EBlending blend, ECulling cull, EDepthTest depth,
                EFillMode fill = EFillMode::Automatic);

    EBlending EBlend() const { return _blend; }
    ECulling Cull() const { return _cull; }
    EDepthTest Depth() const { return _depth; }
    EFillMode Fill() const { return _fill; }
    SINGLETON_POOL_ALLOCATED_DECL();

private:
    EBlending _blend;
    ECulling _cull;
    EDepthTest _depth;
    EFillMode _fill;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
