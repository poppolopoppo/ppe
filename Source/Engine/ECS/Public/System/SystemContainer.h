#pragma once

#include "Core.Logic/Logic.h"

#include "Core.Logic/Component/Component_fwd.h"
#include "Core.Logic/Entity/Entity_fwd.h"
#include "Core.Logic/System/System_fwd.h"

#include "Container/AssociativeVector.h"

namespace PPE {
namespace Logic {
class FEntityManager;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FSystemContainer {
public:
    explicit FSystemContainer(FEntityManager& manager);
    ~FSystemContainer();

    void Initialize();
    void Destroy();

    void Add(ISystem *system, int priority, ESystemExecution executionType);
    void Remove(const PSystem& system);

    void Process(const FTimeline& timeline);

    void RefreshEntity(const FEntity& entity, ComponentFlag components);
    void RemoveEntity(const FEntity& entity);

private:
    friend class FSystemLayer;

    ASSOCIATIVE_VECTOR(System, int, PSystemLayer) _layers;
    FEntityManager *_manager;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace PPE
