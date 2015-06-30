#include "stdafx.h"

#include "Core.Logic/System/SystemLayer.h"

#include "Core.Logic/System/ISystem.h"
#include "Core.Logic/System/SystemContainer.h"

#include "Core/Allocator/PoolAllocator-impl.h"

#include "Core/Memory/BitSet.h"
#include "Core/Thread/ThreadPool.h"

#include <algorithm>

namespace Core {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
struct AsyncSystemUpdater_ {
    Timeline Timeline;
    ISystem *System;
    void Update() const {
        System->Update(Timeline);
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Logic, SystemLayer, );
//----------------------------------------------------------------------------
SystemLayer::SystemLayer() {}
//----------------------------------------------------------------------------
SystemLayer::~SystemLayer() {}
//----------------------------------------------------------------------------
SystemLayer::SystemLayer(SystemLayer&& rvalue)
:   _asynchronous(std::move(rvalue._asynchronous))
,   _synchronous(std::move(rvalue._asynchronous)) {}
//----------------------------------------------------------------------------
SystemLayer& SystemLayer::operator =(SystemLayer&& rvalue) {
    _asynchronous = std::move(rvalue._asynchronous);
    _synchronous = std::move(rvalue._synchronous);
    return *this;
}
//----------------------------------------------------------------------------
void SystemLayer::Add(SystemExecution executionType, ISystem *system) {
    Assert(system);

    switch (executionType)
    {
    case Core::Logic::SystemExecution::Asynchronous:
        _asynchronous.emplace_back(system);
        break;
    case Core::Logic::SystemExecution::Synchronous:
        _synchronous.emplace_back(system);
        break;
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
bool SystemLayer::TryRemove(const PSystem& system) {
    Assert(system);

    VECTOR(System, PSystem)::iterator it;
    if (_asynchronous.end() != (it = std::find(_asynchronous.begin(), _asynchronous.end(), system)))
        _asynchronous.erase(it);
    else if (_synchronous.end() != (it = std::find(_synchronous.begin(), _synchronous.end(), system)))
        _synchronous.erase(it);
    else
        return false;

    return true;
}
//----------------------------------------------------------------------------
bool SystemLayer::Contains(const PSystem& system) const {
    Assert(system);

    return  Core::Contains(_asynchronous, system) ||
            Core::Contains(_synchronous, system);
}
//----------------------------------------------------------------------------
void SystemLayer::Process(SystemContainer& container, const Timeline& timeline) {

    // asynchronous part :
    TaskCounter *pcounter = nullptr;
    if (_asynchronous.size())
    {
        const size_t taskCount = _asynchronous.size();

        size_t queuedTask = 0;
        STACKLOCAL_POD_ARRAY(Task, tasks, taskCount);
        STACKLOCAL_POD_ARRAY(AsyncSystemUpdater_, updaters, taskCount);

        for (const PSystem& system : _asynchronous)
            if (system->Enabled()) {
                updaters[queuedTask].System = system.get();
                updaters[queuedTask].Timeline = timeline;
                tasks[queuedTask] = Delegate(&AsyncSystemUpdater_::Update, &updaters[queuedTask]);
                ++queuedTask;
            }
        
        GlobalThreadPool::Instance().Run(tasks.SubRange(0, queuedTask), &pcounter);
    }
    // synchronous part :
    if (_synchronous.size())
    {
        for (const PSystem& system : _synchronous)
            if (system->Enabled())
                system->Update(timeline);
    }
    // wait for asynchronous tasks :
    if (pcounter)
    {
        GlobalThreadPool::Instance().WaitFor(&pcounter);
    }
    Assert(nullptr == pcounter);
}
//----------------------------------------------------------------------------
void SystemLayer::Destroy(EntityManager& manager) {

    for (const PSystem& system : _asynchronous)
        system->Destroy(manager);

    for (const PSystem& system : _synchronous)
        system->Destroy(manager);
}
//----------------------------------------------------------------------------
void SystemLayer::RefreshEntity(const Entity& entity, ComponentFlag components) {

    for (const PSystem& system : _asynchronous)
        system->OnEntityRefresh(entity, components);

    for (const PSystem& system : _synchronous)
        system->OnEntityRefresh(entity, components);
}
//----------------------------------------------------------------------------
void SystemLayer::RemoveEntity(const Entity& entity) {
    
    for (const PSystem& system : _asynchronous)
        system->OnEntityDeleted(entity);

    for (const PSystem& system : _synchronous)
        system->OnEntityDeleted(entity);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
