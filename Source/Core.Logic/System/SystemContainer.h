#pragma once

#include "Core.Logic/Logic.h"

#include "Core.Logic/Component/Component_fwd.h"
#include "Core.Logic/Entity/Entity_fwd.h"
#include "Core.Logic/System/System_fwd.h"

#include "Core/Container/AssociativeVector.h"

namespace Core {
namespace Logic {
class EntityManager;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class SystemContainer {
public:
    explicit SystemContainer(EntityManager& manager);
    ~SystemContainer();

    void Initialize();
    void Destroy();

    void Add(ISystem *system, int priority, SystemExecution executionType);
    void Remove(const PSystem& system);

    void Process(const Timeline& timeline);

    void RefreshEntity(const Entity& entity, ComponentFlag components);
    void RemoveEntity(const Entity& entity);

private:
    friend class SystemLayer;

    ASSOCIATIVE_VECTOR(System, int, PSystemLayer) _layers;
    EntityManager *_manager;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
