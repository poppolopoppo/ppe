#pragma once

#include "Core.Logic/Logic.h"

#include "Core.Logic/Component/Component_fwd.h"
#include "Core.Logic/Entity/Entity_fwd.h"

namespace Core {
namespace Logic {
class ComponentContainer;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class SystemAspect {
public:
    SystemAspect(ComponentFlag all, ComponentFlag any, ComponentFlag none) 
    :   _all(all), _any(any), _none(none) {}
    SystemAspect() : SystemAspect(ComponentFlag(0), ComponentFlag(0), ComponentFlag(0) ) {}

    ComponentFlag All() const { return _all; }
    ComponentFlag Any() const { return _any; }
    ComponentFlag None() const { return _none; }

    void SetAll(ComponentFlag value) { _all = value; }
    void SetAny(ComponentFlag value) { _any = value; }
    void SetNone(ComponentFlag value) { _none = value; }

    bool Matches(ComponentFlag components) const {
        return (_all.Value == (_all & components)) &&
               (0 == _any || 0 != (_any & components)) &&
               (0 == (_none & components));
    }

    bool operator ==(const SystemAspect& other) const { return _all == other._all && _any == other._any && _none == other._none; }
    bool operator !=(const SystemAspect& other) const { return !operator ==(other); }

    static SystemAspect Everything() { return SystemAspect(0, ComponentFlag(ComponentFlag::value_type(-1)), 0); }
    static SystemAspect Nothing()    { return SystemAspect(0, 0, ComponentFlag(ComponentFlag::value_type(-1))); }

    template <typename T0, typename... TN>
    static SystemAspect MatchAll(const ComponentContainer& components, ComponentFlag any = 0, ComponentFlag none = 0);
    template <typename T0, typename... TN>
    static SystemAspect MatchAny(const ComponentContainer& components, ComponentFlag all = 0, ComponentFlag none = 0);
    template <typename T0, typename... TN>
    static SystemAspect MatchNone(const ComponentContainer& components, ComponentFlag all = 0, ComponentFlag any = 0);

    template <typename T0, typename... TN>
    static ComponentFlag MakeFlags(const ComponentContainer& components);

private:
    template <typename T0, typename... TN>
    struct MergedTags_ {
        FORCE_INLINE static ComponentFlag value(const ComponentContainer& components) { 
            return container.Flag<T0>() | MergedTags_<TN...>::value(container); 
        }
    };

    template <typename T>
    struct MergedTags_<T> {
        FORCE_INLINE static ComponentFlag value(const ComponentContainer& components) { 
            return container.Flag<T>(); 
        }
    };

    ComponentFlag _all;     // matches if all of the components are present
    ComponentFlag _any;     // matches if any of the components are present
    ComponentFlag _none;    // matches if none of the components are present
};
//----------------------------------------------------------------------------
template <typename T0, typename... TN> 
SystemAspect SystemAspect::MatchAll(const ComponentContainer& components, ComponentFlag any/* = 0 */, ComponentFlag none/* = 0 */) {
    return SystemAspect(MakeFlags<T0, TN...>(components), any, none); 
}
//----------------------------------------------------------------------------
template <typename T0, typename... TN> 
SystemAspect SystemAspect::MatchAny(const ComponentContainer& components, ComponentFlag all/* = 0 */, ComponentFlag none/* = 0 */) {
    return SystemAspect(all, MakeFlags<T0, TN...>(components), none); 
}
//----------------------------------------------------------------------------
template <typename T0, typename... TN> 
SystemAspect SystemAspect::MatchNone(const ComponentContainer& components, ComponentFlag all/* = 0 */, ComponentFlag any/* = 0 */) {
    return SystemAspect(all, any, MakeFlags<T0, TN...>(components));
}
//----------------------------------------------------------------------------
template <typename T0, typename... TN>
ComponentFlag SystemAspect::MakeFlags(const ComponentContainer& components) {
    return MergedTags_<T0, TN...>::value(components);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
