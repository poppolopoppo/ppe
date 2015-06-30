#pragma once

#include "Core.Logic/Logic.h"

#include "Core.Logic/Component/ComponentTag.h"
#include "Core.Logic/Component/IComponent.h"

namespace Core {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class ITypedComponent : public IComponent {
public:
    virtual ~ITypedComponent() {}

    typedef typename std::add_pointer<T>::type pointer;
    typedef typename std::add_pointer<const T>::type const_pointer;
    typedef typename std::add_lvalue_reference<T>::type reference;
    typedef typename std::add_lvalue_reference<const T>::type const_reference;
    typedef T value_type;

    virtual ComponentTag Tag() const { return ComponentTag<T>::Tag(); }

    virtual void AddComponent(EntityID id, T&& data) = 0;

    virtual reference GetComponent(EntityID id) = 0;
    virtual const_reference GetComponent(EntityID id) const = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
