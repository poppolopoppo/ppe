#include "stdafx.h"

#include "RenderState.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FRenderState::FRenderState()
:   FRenderState(EBlending::Opaque, ECulling::Clockwise, EDepthTest::Default) {}
//----------------------------------------------------------------------------
FRenderState::~FRenderState() {}
//----------------------------------------------------------------------------
FRenderState::FRenderState(EBlending blend, ECulling cull, EDepthTest depth, EFillMode fill)
:   _blend(blend), _cull(cull), _depth(depth), _fill(fill) {}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, FRenderState, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
