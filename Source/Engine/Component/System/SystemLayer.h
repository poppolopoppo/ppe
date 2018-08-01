#pragma once

#include "Core.Logic/Logic.h"

#include "Core.Logic/Component/Component_fwd.h"
#include "Core.Logic/Entity/Entity_fwd.h"
#include "Core.Logic/System/System_fwd.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/Vector.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
class FTimeline;

namespace Logic {
class FEntityManager;
class ISystem;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ESystemExecution {
    Asynchronous = 0,
    Synchronous
};
//----------------------------------------------------------------------------
class FSystemLayer : public FRefCountable {
public:
    FSystemLayer();
    ~FSystemLayer();

    FSystemLayer(const FSystemLayer& ) = delete;
    FSystemLayer& operator =(const FSystemLayer& ) = delete;

    FSystemLayer(FSystemLayer&& rvalue);
    FSystemLayer& operator =(FSystemLayer&& rvalue);

    const VECTOR(System, PSystem)& Asynchronous() const { return _asynchronous; }
    const VECTOR(System, PSystem)& Synchronous() const { return _synchronous; }

    void Add(ESystemExecution executionType, ISystem *system);
    bool TryRemove(const PSystem& system);

    bool Contains(const PSystem& system) const;

    void Process(FSystemContainer& container, const FTimeline& timeline);
    void Destroy(FEntityManager& container);

    void RefreshEntity(const FEntity& entity, ComponentFlag components);
    void RemoveEntity(const FEntity& entity);

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    VECTOR(System, PSystem) _asynchronous;
    VECTOR(System, PSystem) _synchronous;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
