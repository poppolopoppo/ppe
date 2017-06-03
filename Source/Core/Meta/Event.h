#pragma once

#include "Core/Core.h"

#include "Core/Container/Vector.h"
#include "Core/Meta/Delegate.h"
#include "Core/Meta/ThreadResource.h"

#include <algorithm>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Delegate>
class TEvent {
public:
    TEvent() { static_assert(false, "TEvent<T> accepts only delegates"); }
};
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Args >
class TEvent< TDelegate<_Ret (*)(_Args... )> > : public Meta::FThreadResource {
public:
    typedef TDelegate<_Ret (*)(_Args... )> delegate_type;
    typedef VECTORINSITU(Event, delegate_type, 3) vector_type;

    bool empty() const { return _delegates.empty(); }
    size_t size() const { return _delegates.size(); }

    operator const void *() const { return _delegates.empty() ? nullptr : this; }

    const vector_type& Delegates() const { return _delegates; }

    void Add(const delegate_type& d);
    void Remove(const delegate_type& d);

    template <typename _It>
    void Add(_It begin, _It end);

    void clear();
    void reserve(size_t capacity);

    TEvent& operator +=(const delegate_type& d) { Add(d); return *this; }
    TEvent& operator -=(const delegate_type& d) { Remove(d); return *this; }

    TEvent& operator <<(const delegate_type& d) { Add(d); return *this; }
    TEvent& operator >>(const delegate_type& d) { Remove(d); return *this; }

    void Invoke(_Args... args) const;
    void operator ()(_Args... args) const { Invoke(std::forward<_Args>(args)...); }

private:
    vector_type _delegates;
};
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Args >
void TEvent< TDelegate<_Ret (*)(_Args... )> >::Add(const delegate_type& d) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_delegates.end() == std::find(_delegates.begin(), _delegates.end(), d));
    _delegates.push_back(d);
}
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Args >
void TEvent< TDelegate<_Ret (*)(_Args... )> >::Remove(const delegate_type& d) {
    THIS_THREADRESOURCE_CHECKACCESS();
    const auto it = std::find(_delegates.begin(), _delegates.end(), d);
    Assert(_delegates.end() != it);
    _delegates.erase(it);
}
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Args >
template <typename _It>
void TEvent< TDelegate<_Ret (*)(_Args... )> >::Add(_It begin, _It end) {
    THIS_THREADRESOURCE_CHECKACCESS();
#ifdef WITH_CORE_ASSERT
    for (auto it = begin; it != end; ++it)
        Add(*it); // will check for doubloons in debug
#else
    _delegates.insert(_delegates.end(), begin, end);
#endif
}
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Args >
void TEvent< TDelegate<_Ret (*)(_Args... )> >::clear() {
    THIS_THREADRESOURCE_CHECKACCESS();
    _delegates.clear();
}
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Args >
void TEvent< TDelegate<_Ret (*)(_Args... )> >::reserve(size_t capacity) {
    THIS_THREADRESOURCE_CHECKACCESS();
    _delegates.reserve(capacity);
}
//----------------------------------------------------------------------------
template <typename _Ret, typename... _Args >
void TEvent< TDelegate<_Ret (*)(_Args... )> >::Invoke(_Args... args) const {
    THIS_THREADRESOURCE_CHECKACCESS();
    for (const delegate_type& d : _delegates)
        d.Invoke(std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Public interface of event, hides owner-only methods
// ex :
//      TPublicEvent<TDelegate> Event() { return _event; }
//      TEvent<TDelegate> _event;
//----------------------------------------------------------------------------
template <typename _Delegate>
class TPublicEvent {
public:
    typedef _Delegate delegate_type;
    typedef TEvent<_Delegate> event_type;

    TPublicEvent(event_type& owner) : _owner(owner) {}

    void Add(const delegate_type& d) { _owner.Add(d); }
    void Remove(const delegate_type& d) { _owner.Remove(d); }

    template <typename _It>
    void Add(_It begin, _It end) { Add(std::forward<_It>(begin), std::forward<_It>(end)); }

private:
    event_type& _owner;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
