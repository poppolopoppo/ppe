#pragma once

#include "Core.Logic/Logic.h"

#include "Core.Logic/Component/Component_fwd.h"
#include "Core.Logic/Entity/Entity_fwd.h"

#include "Memory/RefPtr.h"

#include <type_traits>

namespace PPE {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class ITypedComponent;
//----------------------------------------------------------------------------
class IComponent : public FRefCountable {
public:
    STATIC_CONST_INTEGRAL(ComponentID::value_type, InvalidID, -1);
    STATIC_CONST_INTEGRAL(ComponentTag::value_type, InvalidTag, 0);

    virtual ~IComponent() {}

    virtual ComponentTag Tag() const = 0;

    virtual void RemoveComponent(EntityID id) = 0;

    virtual void Clear() = 0;

    virtual void CloneEntity(EntityID dst, EntityID src) = 0;

    template <typename T> ITypedComponent<T> *As() { return checked_cast<ITypedComponent<T> *>(this); }
    template <typename T> const ITypedComponent<T> *As() const { return checked_cast<ITypedComponent<T> *>(this); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace PPE
