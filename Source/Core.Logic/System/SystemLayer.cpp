#include "stdafx.h"

#include "Core.Logic/System/SystemLayer.h"

#include "Core.Logic/System/ISystem.h"
#include "Core.Logic/System/SystemContainer.h"

#include "Core/Allocator/PoolAllocator-impl.h"

#include "Core/Container/BitSet.h"
#include "Core/Thread/ThreadPool.h"

#include <algorithm>

namespace Core {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
struct FAsyncSystemUpdater_ {
    FTimeline FTimeline;
    ISystem *System;
    void Update() const {
        System->Update(FTimeline);
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Logic, FSystemLayer, );
//----------------------------------------------------------------------------
FSystemLayer::FSystemLayer() {}
//----------------------------------------------------------------------------
FSystemLayer::~FSystemLayer() {}
//----------------------------------------------------------------------------
FSystemLayer::FSystemLayer(FSystemLayer&& rvalue)
:   _asynchronous(std::move(rvalue._asynchronous))
,   _synchronous(std::move(rvalue._asynchronous)) {}
//----------------------------------------------------------------------------
FSystemLayer& FSystemLayer::operator =(FSystemLayer&& rvalue) {
    _asynchronous = std::move(rvalue._asynchronous);
    _synchronous = std::move(rvalue._synchronous);
    return *this;
}
//----------------------------------------------------------------------------
void FSystemLayer::Add(ESystemExecution executionType, ISystem *system) {
    Assert(system);

    switch (executionType)
    {
    case Core::Logic::ESystemExecution::Asynchronous:
        _asynchronous.emplace_back(system);
        break;
    case Core::Logic::ESystemExecution::Synchronous:
        _synchronous.emplace_back(system);
        break;
    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
bool FSystemLayer::TryRemove(const PSystem& system) {
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
bool FSystemLayer::Contains(const PSystem& system) const {
    Assert(system);

    return  Core::Contains(_asynchronous, system) ||
            Core::Contains(_synchronous, system);
}
//----------------------------------------------------------------------------
void FSystemLayer::Process(FSystemContainer& container, const FTimeline& timeline) {

    // asynchronous part :
    FTaskCounter *pcounter = nullptr;
    if (_asynchronous.size())
    {
        const size_t taskCount = _asynchronous.size();

        size_t queuedTask = 0;
        STACKLOCAL_POD_ARRAY(FTask, tasks, taskCount);
        STACKLOCAL_POD_ARRAY(FAsyncSystemUpdater_, updaters, taskCount);

        for (const PSystem& system : _asynchronous)
            if (system->Enabled()) {
                updaters[queuedTask].System = system.get();
                updaters[queuedTask].Timeline = timeline;
                tasks[queuedTask] = TDelegate(&FAsyncSystemUpdater_::Update, &updaters[queuedTask]);
                ++queuedTask;
            }

        FGlobalThreadPool::Instance().Run(tasks.SubRange(0, queuedTask), &pcounter);
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
        FGlobalThreadPool::Instance().WaitFor(&pcounter);
    }
    Assert(nullptr == pcounter);
}
//----------------------------------------------------------------------------
void FSystemLayer::Destroy(FEntityManager& manager) {

    for (const PSystem& system : _asynchronous)
        system->Destroy(manager);

    for (const PSystem& system : _synchronous)
        system->Destroy(manager);
}
//----------------------------------------------------------------------------
void FSystemLayer::RefreshEntity(const FEntity& entity, ComponentFlag components) {

    for (const PSystem& system : _asynchronous)
        system->OnEntityRefresh(entity, components);

    for (const PSystem& system : _synchronous)
        system->OnEntityRefresh(entity, components);
}
//----------------------------------------------------------------------------
void FSystemLayer::RemoveEntity(const FEntity& entity) {

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
