#pragma once

#include "Core.Logic/Logic.h"

#include "Core.Logic/Component/Component_fwd.h"
#include "Core.Logic/Entity/Entity_fwd.h"

namespace Core {
namespace Logic {
class FComponentContainer;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FSystemAspect {
public:
    FSystemAspect(ComponentFlag all, ComponentFlag any, ComponentFlag none) 
    :   _all(all), _any(any), _none(none) {}
    FSystemAspect() : FSystemAspect(ComponentFlag(0), ComponentFlag(0), ComponentFlag(0) ) {}

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

    bool operator ==(const FSystemAspect& other) const { return _all == other._all && _any == other._any && _none == other._none; }
    bool operator !=(const FSystemAspect& other) const { return !operator ==(other); }

    static FSystemAspect Everything() { return FSystemAspect(ComponentFlag(0), ComponentFlag(ComponentFlag::value_type(-1)), ComponentFlag(0)); }
    static FSystemAspect Nothing()    { return FSystemAspect(ComponentFlag(0), ComponentFlag(0), ComponentFlag(ComponentFlag::value_type(-1))); }

    template <typename T0, typename... TN>
    static FSystemAspect MatchAll(const FComponentContainer& components, ComponentFlag any = 0, ComponentFlag none = 0);
    template <typename T0, typename... TN>
    static FSystemAspect MatchAny(const FComponentContainer& components, ComponentFlag all = 0, ComponentFlag none = 0);
    template <typename T0, typename... TN>
    static FSystemAspect MatchNone(const FComponentContainer& components, ComponentFlag all = 0, ComponentFlag any = 0);

    template <typename T0, typename... TN>
    static ComponentFlag MakeFlags(const FComponentContainer& components);

private:
    template <typename T0, typename... TN>
    struct TMergedTags_ {
        FORCE_INLINE static ComponentFlag value(const FComponentContainer& components) { 
            return container.Flag<T0>() | TMergedTags_<TN...>::value(container); 
        }
    };

    template <typename T>
    struct TMergedTags_<T> {
        FORCE_INLINE static ComponentFlag value(const FComponentContainer& components) { 
            return container.Flag<T>(); 
        }
    };

    ComponentFlag _all;     // matches if all of the components are present
    ComponentFlag _any;     // matches if any of the components are present
    ComponentFlag _none;    // matches if none of the components are present
};
//----------------------------------------------------------------------------
template <typename T0, typename... TN> 
FSystemAspect FSystemAspect::MatchAll(const FComponentContainer& components, ComponentFlag any/* = 0 */, ComponentFlag none/* = 0 */) {
    return FSystemAspect(MakeFlags<T0, TN...>(components), any, none); 
}
//----------------------------------------------------------------------------
template <typename T0, typename... TN> 
FSystemAspect FSystemAspect::MatchAny(const FComponentContainer& components, ComponentFlag all/* = 0 */, ComponentFlag none/* = 0 */) {
    return FSystemAspect(all, MakeFlags<T0, TN...>(components), none); 
}
//----------------------------------------------------------------------------
template <typename T0, typename... TN> 
FSystemAspect FSystemAspect::MatchNone(const FComponentContainer& components, ComponentFlag all/* = 0 */, ComponentFlag any/* = 0 */) {
    return FSystemAspect(all, any, MakeFlags<T0, TN...>(components));
}
//----------------------------------------------------------------------------
template <typename T0, typename... TN>
ComponentFlag FSystemAspect::MakeFlags(const FComponentContainer& components) {
    return TMergedTags_<T0, TN...>::value(components);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
