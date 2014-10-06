#pragma once

#include "Observer.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Allocator, typename _Event, typename... _Observables>
void RegisterObserver(  Vector<typename IObserver<_Event, _Observables...>::entry_type, _Allocator >& observers,
                        IObserver<_Event, _Observables...> *observer,
                        const _Event eventFlags ) {
    Assert(eventFlags);
    Assert(observer);

    typedef typename IObserver<_Event, _Observables...>::entry_type entry_type;

    for (entry_type& o : observers)
        if (o.second == observer) {
            Assert(!(o.first & eventFlags)); // double registration for at least one event
            o.first = _Event(o.first | eventFlags);
            return;
        }

    observers.emplace_back(_Event(eventFlags), observer);
}
//----------------------------------------------------------------------------
template <typename _Allocator, typename _Event, typename... _Observables>
void UnregisterObserver(Vector<typename IObserver<_Event, _Observables...>::entry_type, _Allocator >& observers,
                        IObserver<_Event, _Observables...> *observer,
                        const _Event eventFlags ) {
    Assert(eventFlags);
    Assert(observer);

    const auto end = observers.end();
    for (auto it = observers.begin(); it != end; ++it)
        if (it->second == observer) {
            Assert((it->first & eventFlags) == eventFlags); // was registered to each event
            if (it->first == eventFlags)
                observers.erase(it); // no more observed events, remove observer
            else
                it->first = _Event(it->first & (~eventFlags));
            return;
        }

    AssertNotReached(); // observer is not in the vector
}
//----------------------------------------------------------------------------
template <typename _Allocator, typename _Event, typename... _Observables>
void NotifyObservers(   Vector<typename IObserver<_Event, _Observables...>::entry_type, _Allocator >& observers,
                        const _Event eventFlags,
                        const _Observables&... observables ) {
    Assert(size_t(eventFlags));

    typedef typename IObserver<_Event, _Observables...>::entry_type entry_type;

    for (const entry_type& o : observers)
        if (o.first & eventFlags)
            o.second->NotifyEvent(eventFlags, observables... );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
