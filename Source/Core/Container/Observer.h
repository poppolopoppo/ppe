#pragma once

#include "Core/Core.h"

#include "Core/Container/Pair.h"
#include "Core/Container/Vector.h"

#include <functional>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Event, typename... _Observables>
class ObserverFunction;
//----------------------------------------------------------------------------
template <typename _Event, typename... _Observables>
class IObserver {
public:
    typedef Pair<_Event, IObserver<_Event, _Observables...> * > entry_type;
    typedef ObserverFunction<_Event, _Observables...> function_type;

    virtual ~IObserver() {}

    virtual void NotifyEvent(const _Event eventFlags, const _Observables&... observables) = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Event, typename... _Observables>
class ObserverFunction : public IObserver<_Event, _Observables...> {
public:
    typedef IObserver<_Event, _Observables...> observer_type;
    typedef void (notifyevent_type)(const _Event eventFlags, const _Observables&... observables);
    typedef std::function<notifyevent_type> function_type;

    ObserverFunction(notifyevent_type fn) : _fn(fn) {}
    ObserverFunction(function_type&& fn) : _fn(std::move(fn)) {}
    virtual ~ObserverFunction() {}

    template <typename _Arg0, typename... _Args>
    ObserverFunction(   void (*func)(const _Event eventFlags, const _Observables&... observables, const _Arg0& , const _Args&... ),
                        _Arg0&& arg0, _Args&&... args)
    :   ObserverFunction(std::bind(func, _1, _2, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...))
    {}

    template <typename _Object, typename _Arg0, typename... _Args>
    ObserverFunction(   void (_Object::* member)(const _Event eventFlags, const _Observables&... observables, const _Arg0& , const _Args&... ),
                        _Object *instance, _Arg0&& arg0, _Args&&... args)
    :   ObserverFunction(std::bind(member, instance, _1, _2, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...))
    {}

    virtual void NotifyEvent(const _Event eventFlags, const _Observables&... observables) override {
        _fn(eventFlags, observables...);
    }

private:
    function_type _fn;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Allocator, typename _Event, typename... _Observables>
void RegisterObserver(  Vector<typename IObserver<_Event, _Observables...>::entry_type, _Allocator >& observers,
                        IObserver<_Event, _Observables...> *observer,
                        const _Event eventFlags );
//----------------------------------------------------------------------------
template <typename _Allocator, typename _Event, typename... _Observables>
void UnregisterObserver(Vector<typename IObserver<_Event, _Observables...>::entry_type, _Allocator >& observers,
                        IObserver<_Event, _Observables...> *observer,
                        const _Event eventFlags );
//----------------------------------------------------------------------------
template <typename _Allocator, typename _Event, typename... _Observables>
void NotifyObservers(   Vector<typename IObserver<_Event, _Observables...>::entry_type, _Allocator >& observers,
                        const _Event eventFlags,
                        const _Observables&... observables );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Container/Observer-inl.h"
