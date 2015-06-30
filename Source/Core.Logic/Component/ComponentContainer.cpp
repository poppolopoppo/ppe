#include "stdafx.h"

#include "ComponentContainer.h"

#include "ComponentTag.h"
#include "IComponent.h"
#include "ITypedComponent.h"

#include "Entity/Entity.h"

namespace Core {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ComponentContainer::ComponentContainer() : _reservedFlags(0) {
    _components.resize(ComponentCapacity);
    _tagToID.reserve(ComponentCapacity);
}
//----------------------------------------------------------------------------
ComponentContainer::~ComponentContainer() {}
//----------------------------------------------------------------------------
void ComponentContainer::Register(IComponent *component) {
    Assert(nullptr != component);

    const ComponentTag tag = component->Tag();
    Assert(IComponent::InvalidTag != tag);

    ComponentID freeID = u32(-1);
    forrange(i, 0, u32(ComponentCapacity))
        if (0 == (_reservedFlags & (1<<i))) {
            _reservedFlags = _reservedFlags | (1<<i);
            freeID = i;
            break;
        }
    AssertRelease(u32(freeID) < ComponentCapacity);
    Assert(nullptr == _components[freeID]);

    _components[freeID] = component;
    Insert_AssertUnique(_tagToID, tag, freeID);
}
//----------------------------------------------------------------------------
void ComponentContainer::Unregister(IComponent *component) {
    Assert(nullptr != component);

    const ComponentTag tag = component->Tag();
    Assert(IComponent::InvalidTag != tag);

    const auto it = _tagToID.find(tag);
    AssertRelease(it != _tagToID.end());
    Assert(component == _components[it->second]);

    RemoveRef_AssertReachZero(_components[it->second]);
    _tagToID.erase(it);

    _reservedFlags = _reservedFlags & ~(1<<it->second);
}
//----------------------------------------------------------------------------
ComponentID ComponentContainer::ID(ComponentTag tag) const {
    Assert(IComponent::InvalidTag != tag);

    const ComponentID id = _tagToID.at(tag);
    Assert(0 != (_reservedFlags & (1<<id)) );

    return id;
}
//----------------------------------------------------------------------------
IComponent *ComponentContainer::GetByTag(ComponentTag tag) {
    Assert(IComponent::InvalidTag != tag);

    ComponentID id = u32(-1);
    if (!TryGetValue(_tagToID, tag, &id)) {
        AssertNotReached();
        return nullptr;
    }
    Assert(u32(id) < ComponentCapacity);
    Assert(0 != (_reservedFlags & (1<<id)));
    Assert(nullptr != _components[id]);

    return _components[id];
}
//----------------------------------------------------------------------------
IComponent *ComponentContainer::GetByID(ComponentID id) {
    Assert(id);
    Assert(u32(id) < ComponentCapacity);
    Assert(0 != (_reservedFlags & (1<<id)));
    Assert(nullptr != _components[id]);

    return _components[id];
}
//----------------------------------------------------------------------------
void ComponentContainer::RemoveEntity(const Entity& entity) {
    Assert(entity.Deleting());

    forrange(i, 0, u32(ComponentContainer::ComponentCapacity))
        if (0 != (entity.ComponentFlags() & (1<<i))) {
            const PComponent& component = _components[i];
            Assert(component);

            component->RemoveComponent(entity.ID());
        }
}
//----------------------------------------------------------------------------
void ComponentContainer::Clear() {
    for (const PComponent& component : _components)
        if (component)
            component->Clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
