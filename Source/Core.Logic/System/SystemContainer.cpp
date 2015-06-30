#include "stdafx.h"

#include "Core.Logic/System/SystemContainer.h"

#include "Core.Logic/System/ISystem.h"
#include "Core.Logic/System/SystemLayer.h"

#include "Core.Logic/EntityManager.h"

#include "Core/Thread/ThreadPool.h"

#include <thread>

namespace Core {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool SortSystemLayers_(
    const Pair<int, PSystemLayer>& lhs,
    const Pair<int, PSystemLayer>& rhs ) {
    return lhs.first < rhs.first;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SystemContainer::SystemContainer(EntityManager& manager) 
:   _manager(&manager) {
    Assert(_manager);
}
//----------------------------------------------------------------------------
SystemContainer::~SystemContainer() {}
//----------------------------------------------------------------------------
void SystemContainer::Initialize() {
    Assert(_manager);
}
//----------------------------------------------------------------------------
void SystemContainer::Destroy() {
    Assert(_manager);

    for (const auto& it : _layers)
        it.second->Destroy(*_manager);

    _layers.clear();
}
//----------------------------------------------------------------------------
void SystemContainer::Add(ISystem *system, int priority, SystemExecution executionType) {
    Assert(system);

    PSystemLayer layer;
    if (!_layers.TryGet(priority, &layer)) {
        layer = new SystemLayer();
        _layers.Insert_AssertUnique(priority, layer);
        std::sort(_layers.Vector().begin(), _layers.Vector().end(), SortSystemLayers_);
    }
    Assert(layer);

    system->Initialize(*_manager);
    layer->Add(executionType, system);
}
//----------------------------------------------------------------------------
void SystemContainer::Remove(const PSystem& system) {
    Assert(system);

    for (const Pair<int, PSystemLayer>& it : _layers)
        if (it.second->TryRemove(system)) {
            system->Destroy(*_manager);
            return;
        }

    AssertNotReached();
}
//----------------------------------------------------------------------------
void SystemContainer::Process(const Timeline& timeline) {

    for (const auto& it : _layers)
        it.second->Process(*this, timeline);
}
//----------------------------------------------------------------------------
void SystemContainer::RefreshEntity(const Entity& entity, ComponentFlag components) {

    for (const auto& it : _layers)
        it.second->RefreshEntity(entity, components);
}
//----------------------------------------------------------------------------
void SystemContainer::RemoveEntity(const Entity& entity) {

    for (const auto& it : _layers)
        it.second->RemoveEntity(entity);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
