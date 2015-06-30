#include "stdafx.h"

#include "RenderState.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
RenderState::RenderState()
:   RenderState(Blending::Opaque, Culling::Clockwise, DepthTest::Default) {}
//----------------------------------------------------------------------------
RenderState::~RenderState() {}
//----------------------------------------------------------------------------
RenderState::RenderState(Blending blend, Culling cull, DepthTest depth, FillMode fill)
:   _blend(blend), _cull(cull), _depth(depth), _fill(fill) {}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, RenderState, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
