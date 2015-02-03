#pragma once

#include "Core/Core.h"

#include "Core/Container/Pair.h"
#include "Core/Container/Vector.h"
#include "Core/Meta/Callback.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Event, typename... _Observables>
using Observer = Meta::Callback<void (*)(_Event, _Observables...)>;
//----------------------------------------------------------------------------
template <typename _Event, typename... _Observables>
using RegisteredObserver = Pair<_Event, Observer<_Event, _Observables...> >;
//----------------------------------------------------------------------------
template <typename _Allocator, typename _Event, typename... _Observables>
using ObserverContainer = Vector<RegisteredObserver<_Event, _Observables...>, _Allocator >;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Allocator, typename _Event, typename... _Observables>
void RegisterObserver(  ObserverContainer<_Allocator, _Event, _Observables...>& observers,
                        const Observer<_Event, _Observables...>& observer,
                        const _Event eventFlags );
//----------------------------------------------------------------------------
template <typename _Allocator, typename _Event, typename... _Observables>
void UnregisterObserver(ObserverContainer<_Allocator, _Event, _Observables...>& observers,
                        const Observer<_Event, _Observables...>& observer,
                        const _Event eventFlags );
//----------------------------------------------------------------------------
template <typename _Allocator, typename _Event, typename... _Observables>
void NotifyObservers(   ObserverContainer<_Allocator, _Event, _Observables...>& observers,
                        const _Event eventFlags,
                        const _Observables&... observables );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define OBSERVERCONTAINER(_DOMAIN, _Event, ...) \
    VECTOR(_DOMAIN, Core::RegisteredObserver<_Event COMMA __VA_ARGS__> )
//----------------------------------------------------------------------------
#define OBSERVERCONTAINER_THREAD_LOCAL(_DOMAIN, _Event, ...) \
    VECTOR_THREAD_LOCAL(_DOMAIN, ::Core::RegisteredObserver<_Event COMMA __VA_ARGS__> )
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Container/Observer-inl.h"
