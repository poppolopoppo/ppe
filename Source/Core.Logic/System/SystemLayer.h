#pragma once

#include "Core.Logic/Logic.h"

#include "Core.Logic/Component/Component_fwd.h"
#include "Core.Logic/Entity/Entity_fwd.h"
#include "Core.Logic/System/System_fwd.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/Vector.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
class Timeline;

namespace Logic {
class EntityManager;
class ISystem;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class SystemExecution {
    Asynchronous = 0,
    Synchronous
};
//----------------------------------------------------------------------------
class SystemLayer : public RefCountable {
public:
    SystemLayer();
    ~SystemLayer();

    SystemLayer(const SystemLayer& ) = delete;
    SystemLayer& operator =(const SystemLayer& ) = delete;

    SystemLayer(SystemLayer&& rvalue);
    SystemLayer& operator =(SystemLayer&& rvalue);

    const VECTOR(System, PSystem)& Asynchronous() const { return _asynchronous; }
    const VECTOR(System, PSystem)& Synchronous() const { return _synchronous; }

    void Add(SystemExecution executionType, ISystem *system);
    bool TryRemove(const PSystem& system);

    bool Contains(const PSystem& system) const;

    void Process(SystemContainer& container, const Timeline& timeline);
    void Destroy(EntityManager& container);

    void RefreshEntity(const Entity& entity, ComponentFlag components);
    void RemoveEntity(const Entity& entity);

    SINGLETON_POOL_ALLOCATED_DECL(SystemLayer);

private:
    VECTOR(System, PSystem) _asynchronous;
    VECTOR(System, PSystem) _synchronous;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
