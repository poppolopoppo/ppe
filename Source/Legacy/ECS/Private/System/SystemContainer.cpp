// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Core.Logic/System/SystemContainer.h"

#include "Core.Logic/System/ISystem.h"
#include "Core.Logic/System/SystemLayer.h"

#include "Core.Logic/EntityManager.h"

#include "Thread/ThreadPool.h"

#include <thread>

namespace PPE {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool SortSystemLayers_(
    const TPair<int, PSystemLayer>& lhs,
    const TPair<int, PSystemLayer>& rhs ) {
    return lhs.first < rhs.first;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FSystemContainer::FSystemContainer(FEntityManager& manager) 
:   _manager(&manager) {
    Assert(_manager);
}
//----------------------------------------------------------------------------
FSystemContainer::~FSystemContainer() {}
//----------------------------------------------------------------------------
void FSystemContainer::Initialize() {
    Assert(_manager);
}
//----------------------------------------------------------------------------
void FSystemContainer::Destroy() {
    Assert(_manager);

    for (const auto& it : _layers)
        it.second->Destroy(*_manager);

    _layers.clear();
}
//----------------------------------------------------------------------------
void FSystemContainer::Add(ISystem *system, int priority, ESystemExecution executionType) {
    Assert(system);

    PSystemLayer layer;
    if (!_layers.TryGet(priority, &layer)) {
        layer = new FSystemLayer();
        _layers.Insert_AssertUnique(priority, layer);
        std::sort(_layers.Vector().begin(), _layers.Vector().end(), SortSystemLayers_);
    }
    Assert(layer);

    system->Initialize(*_manager);
    layer->Add(executionType, system);
}
//----------------------------------------------------------------------------
void FSystemContainer::Remove(const PSystem& system) {
    Assert(system);

    for (const TPair<int, PSystemLayer>& it : _layers)
        if (it.second->TryRemove(system)) {
            system->Destroy(*_manager);
            return;
        }

    AssertNotReached();
}
//----------------------------------------------------------------------------
void FSystemContainer::Process(const FTimeline& timeline) {

    for (const auto& it : _layers)
        it.second->Process(*this, timeline);
}
//----------------------------------------------------------------------------
void FSystemContainer::RefreshEntity(const FEntity& entity, ComponentFlag components) {

    for (const auto& it : _layers)
        it.second->RefreshEntity(entity, components);
}
//----------------------------------------------------------------------------
void FSystemContainer::RemoveEntity(const FEntity& entity) {

    for (const auto& it : _layers)
        it.second->RemoveEntity(entity);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace PPE
