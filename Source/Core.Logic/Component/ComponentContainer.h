#pragma once

#include "Core.Logic/Logic.h"

#include "Core.Logic/Component/Component_fwd.h"
#include "Core.Logic/Entity/Entity_fwd.h"

#include "Core/Container/HashMap.h"
#include "Core/Container/Vector.h"

namespace Core {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ComponentContainer {
public:
    STATIC_CONST_INTEGRAL(size_t, ComponentCapacity, (sizeof(ComponentID)<<3));

    ComponentContainer();
    ~ComponentContainer();

    void Register(IComponent *component);
    void Unregister(IComponent *component);

    ComponentID ID(ComponentTag tag) const;

    template <typename T>
    ComponentID ID() const { return ID(ComponentTag<T>::Tag()); }
    template <typename T>
    ComponentFlag Flag() const { return ComponentFlag(1) << ID<T>(); }

    IComponent *GetByTag(ComponentTag tag);
    IComponent *GetByID(ComponentID id);

    template <typename T>
    ITypedComponent<T> *TypedGet(ComponentID id) {
        return checked_cast<ITypedComponent<T> *>(GetByID(id));
    }

    void RemoveEntity(const Entity& entity);

    void Clear();

private:
    VECTOR(Entity, PComponent) _components;
    HASHMAP(Entity, ComponentTag, ComponentID) _tagToID;

    ComponentFlag _reservedFlags;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
