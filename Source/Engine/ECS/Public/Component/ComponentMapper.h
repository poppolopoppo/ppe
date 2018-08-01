#pragma once

#include "Core.Logic/Logic.h"

#include "Core.Logic/Component/Component_fwd.h"

#include <tuple>

namespace PPE {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T0, typename... TN >
class TComponentMapper;
//----------------------------------------------------------------------------
template <typename T>
class TComponentMapper<T> {
public:
    TComponentMapper(FComponentContainer& components) { TMap(components); }

    ComponentID ID() const { return _id; }
    ComponentTag Tag() const { return Logic::TComponent<T>::Tag(); }
    ITypedComponent<T> *TComponent() const { return _component.get(); }

    ITypedComponent<T> *operator ->() const { Assert(_component); return _component.get(); }
    ITypedComponent<T>& operator  *() const { Assert(_component); return *_component.get(); }

    void TMap(FComponentContainer& components);

private:
    ComponentID _id;
    TPTypedComponent<T> _component;
};
//----------------------------------------------------------------------------
template <typename T>
void TComponentMapper<T>::TMap(FComponentContainer& components) {
    _id = components.TagToID(Tag());
    Assert(IComponent::InvalidID != _id);

    _component = components.TypedGet<T>(_id);
    Assert(_component);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T0, typename... TN >
class TComponentMapper {
public:
    typedef std::tuple<T0, TN... > tuple_type;

    TComponentMapper(FComponentContainer& components) { TMap(components); }

    void TMap(FComponentContainer& components);

    template <size_t _Index>
    typename std::add_reference<typename std::add_const<
        typename std::tuple_element<_Index, tuple_type>::type >::type
    >::type Get() const {
        return std::get<_Index>(TTuple);
    }

    tuple_type TTuple;
};
//----------------------------------------------------------------------------
template <typename T0, typename... TN >
void TComponentMapper<T0, TN... >::TMap(FComponentContainer& components) {
    TTuple = tuple_type(TComponentMapper<T0>(components), TComponentMapper<TN>(components)...);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace PPE
