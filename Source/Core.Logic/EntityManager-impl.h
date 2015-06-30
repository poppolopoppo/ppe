#pragma once

#include "Core.Logic/EntityManager.h"

namespace Core {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
void EntityManager::AddComponent(EntityID id, T&& data) {
    const ComponentTag componentTag = Component<T>::Tag();
    const ComponentID componentID = _components.TagToID(name);

    AddComponent<T>(componentID, id, std::move(data));
}
//----------------------------------------------------------------------------
template <typename T>
void EntityManager::AddComponent(ComponentID componentID, EntityID entityID, T&& data) {
    Assert(componentID == _components.TagToID(Component<T>::Tag()) );

    Entity& entity = _entities.Get(entityID);
    Assert(0 == (entity._componentFlags & (1<<componentID)) );

    const ComponentFlag previousComponents = entity._componentFlags;
    entity._componentFlags |= (1<<componentID);

    IComponent *const component = _components.Get(componentID);
    Assert(component);

    ITypedComponent<T> *const typedComponent = checked_cast<ITypedComponent<T> *>(component);
    typedComponent->AddComponent(entityID, std::move(data));

    Refresh(entityID, previousComponents);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
