#pragma once

#include "Core.Logic/Logic.h"

#include "Core.Logic/Component/Component_fwd.h"

#include <tuple>

namespace Core {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T0, typename... TN >
class ComponentMapper;
//----------------------------------------------------------------------------
template <typename T>
class ComponentMapper<T> {
public:
    ComponentMapper(ComponentContainer& components) { Map(components); }

    ComponentID ID() const { return _id; }
    ComponentTag Tag() const { return Logic::Component<T>::Tag(); }
    ITypedComponent<T> *Component() const { return _component.get(); }

    ITypedComponent<T> *operator ->() const { Assert(_component); return _component.get(); }
    ITypedComponent<T>& operator  *() const { Assert(_component); return *_component.get(); }

    void Map(ComponentContainer& components);

private:
    ComponentID _id;
    PTypedComponent<T> _component;
};
//----------------------------------------------------------------------------
template <typename T>
void ComponentMapper<T>::Map(ComponentContainer& components) {
    _id = components.TagToID(Tag());
    Assert(IComponent::InvalidID != _id);

    _component = components.TypedGet<T>(_id);
    Assert(_component);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T0, typename... TN >
class ComponentMapper {
public:
    typedef std::tuple<T0, TN... > tuple_type;

    ComponentMapper(ComponentContainer& components) { Map(components); }

    void Map(ComponentContainer& components);

    template <size_t _Index>
    typename std::add_reference<typename std::add_const<
        typename std::tuple_element<_Index, tuple_type>::type >::type
    >::type Get() const {
        return std::get<_Index>(Tuple);
    }

    tuple_type Tuple;
};
//----------------------------------------------------------------------------
template <typename T0, typename... TN >
void ComponentMapper<T0, TN... >::Map(ComponentContainer& components) {
    Tuple = tuple_type(ComponentMapper<T0>(components), ComponentMapper<TN>(components)...);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
