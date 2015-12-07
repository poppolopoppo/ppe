#pragma once

#include "Core.RTTI/MetaAtom.h"

#include "Core.RTTI/MetaTypePromote.h"

#include "Core/Allocator/PoolAllocator-impl.h"

#include "Core/Container/Hash.h"
#include "Core/IO/Format.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
T *AtomValueAs(MetaAtom *atom) {
    Assert(atom);
    auto *typed_atom = atom->As<T>;
    return (typed_atom)
        ? &typed_atom->Wrapper()
        : nullptr;
}
//----------------------------------------------------------------------------
template <typename T>
const T *AtomValueAs(const MetaAtom *atom) {
    Assert(atom);
    const auto *typed_atom = atom->As<T>;
    return (typed_atom)
        ? &typed_atom->Wrapper()
        : nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
MetaTypedAtomImpl<T>::MetaTypedAtomImpl() {}
//----------------------------------------------------------------------------
template <typename T>
MetaTypedAtomImpl<T>::~MetaTypedAtomImpl() {}
//----------------------------------------------------------------------------
template <typename T>
MetaTypedAtomImpl<T>::MetaTypedAtomImpl(T&& wrapper)
:   _wrapper(std::move(wrapper)) {}
//----------------------------------------------------------------------------
template <typename T>
MetaTypedAtomImpl<T>& MetaTypedAtomImpl<T>::operator =(T&& wrapper) {
    _wrapper = std::move(wrapper);
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
MetaTypedAtomImpl<T>::MetaTypedAtomImpl(const T& wrapper)
:   _wrapper(wrapper) {}
//----------------------------------------------------------------------------
template <typename T>
MetaTypedAtomImpl<T>& MetaTypedAtomImpl<T>::operator =(const T& wrapper) {
    _wrapper = wrapper;
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
MetaTypedAtomImpl<T>::MetaTypedAtomImpl(MetaTypedAtomImpl&& rvalue)
:   _wrapper(std::move(rvalue._wrapper)) {}
//----------------------------------------------------------------------------
template <typename T>
MetaTypedAtomImpl<T>& MetaTypedAtomImpl<T>::operator =(MetaTypedAtomImpl&& rvalue) {
    _wrapper = std::move(rvalue._wrapper);
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
MetaTypedAtomImpl<T>::MetaTypedAtomImpl(const MetaTypedAtomImpl& other)
:   _wrapper(other._wrapper) {}
//----------------------------------------------------------------------------
template <typename T>
MetaTypedAtomImpl<T>& MetaTypedAtomImpl<T>::operator =(const MetaTypedAtomImpl& other) {
    _wrapper = other._wrapper;
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
MetaTypeInfo MetaTypedAtomImpl<T>::TypeInfo() const {
    return RTTI::TypeInfo< T >();
}
//----------------------------------------------------------------------------
template <typename T>
bool MetaTypedAtomImpl<T>::IsDefaultValue() const {
    return MetaType< T >::IsDefaultValue(_wrapper);
}
//----------------------------------------------------------------------------
template <typename T>
void MetaTypedAtomImpl<T>::MoveTo(MetaAtom *atom) {
    Assert(atom);
    Assert(atom->TypeInfo().Id == meta_type::TypeId);

    MetaTypedAtomImpl<T> *const other = checked_cast<MetaTypedAtomImpl<T> *>(atom);
    other->_wrapper = std::move(_wrapper);
}
//----------------------------------------------------------------------------
template <typename T>
void MetaTypedAtomImpl<T>::CopyTo(MetaAtom *atom) const {
    Assert(atom);
    Assert(atom->TypeInfo().Id == meta_type::TypeId);

    MetaTypedAtomImpl<T> *const other = checked_cast<MetaTypedAtomImpl<T> *>(atom);
    other->_wrapper = _wrapper;
}
//----------------------------------------------------------------------------
template <typename T>
MetaAtom *MetaTypedAtomImpl<T>::WrapMoveTo() {
    return MakeAtom(std::move(_wrapper));
}
//----------------------------------------------------------------------------
template <typename T>
MetaAtom *MetaTypedAtomImpl<T>::WrapCopyTo() const {
    return MakeAtom(_wrapper);
}
//----------------------------------------------------------------------------
template <typename T>
void MetaTypedAtomImpl<T>::MoveFrom(MetaAtom *atom) {
    Assert(atom);
    Assert(atom->TypeInfo().Id == meta_type::TypeId);
    MetaTypedAtomImpl<T> *const other = checked_cast<MetaTypedAtomImpl<T> *>(atom);
    _wrapper = std::move(other->_wrapper);
}
//----------------------------------------------------------------------------
template <typename T>
void MetaTypedAtomImpl<T>::CopyFrom(const MetaAtom *atom) {
    Assert(atom);
    Assert(atom->TypeInfo().Id == meta_type::TypeId);
    const MetaTypedAtomImpl<T> *const other = checked_cast<const MetaTypedAtomImpl<T> *>(atom);
    _wrapper = other->_wrapper;
}
//----------------------------------------------------------------------------
template <typename T>
bool MetaTypedAtomImpl<T>::Equals(const MetaAtom *atom) const {
    Assert(atom);
    if (atom->TypeInfo().Id != meta_type::TypeId)
        return false;

    return atom->Cast<T>()->Wrapper() == _wrapper;
}
//----------------------------------------------------------------------------
template <typename T>
size_t MetaTypedAtomImpl<T>::HashValue() const {
    using Core::hash_value;
    return hash_tuple(u64(meta_type::TypeId), _wrapper);
}
//----------------------------------------------------------------------------
template <typename T>
String MetaTypedAtomImpl<T>::ToString() const {
    using Core::ToString;
    return ToString(_wrapper);
}
//----------------------------------------------------------------------------
template <typename T>
void MetaTypedAtomImpl<T>::Swap(T& wrapper) {
    swap(wrapper, _wrapper);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
void MetaTypedAtom< RTTI::Pair<_First, _Second> >::MoveTo(RTTI::Pair<PMetaAtom, PMetaAtom>& pair) {
    Assert(pair.first);
    Assert(pair.second);
    Assert(pair.first->TypeInfo().Id == MetaType<_First>::TypeId);
    Assert(pair.second->TypeInfo().Id == MetaType<_Second>::TypeId);

    pair.first->Cast<_First>()->Wrapper() = std::move(_wrapper.first);
    pair.second->Cast<_Second>()->Wrapper() = std::move(_wrapper.second);
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
void MetaTypedAtom< RTTI::Pair<_First, _Second> >::CopyTo(RTTI::Pair<PMetaAtom, PMetaAtom>& pair) const {
    Assert(pair.first);
    Assert(pair.second);
    Assert(pair.first->TypeInfo().Id == MetaType<_First>::TypeId);
    Assert(pair.second->TypeInfo().Id == MetaType<_Second>::TypeId);

    pair.first->Cast<_First>()->Wrapper() = _wrapper.first;
    pair.second->Cast<_Second>()->Wrapper() = _wrapper.second;
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
void MetaTypedAtom< RTTI::Pair<_First, _Second> >::WrapMoveTo(RTTI::Pair<PMetaAtom, PMetaAtom>& pair) {
    pair.first = MakeAtom(std::move(_wrapper.first));
    pair.second = MakeAtom(std::move(_wrapper.second));
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
void MetaTypedAtom< RTTI::Pair<_First, _Second> >::WrapCopyTo(RTTI::Pair<PMetaAtom, PMetaAtom>& pair) const {
    pair.first = MakeAtom(_wrapper.first);
    pair.second = MakeAtom(_wrapper.second);
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
void MetaTypedAtom< RTTI::Pair<_First, _Second> >::MoveFrom(const RTTI::Pair<PMetaAtom, PMetaAtom>& pair) {
    Assert(pair.first);
    Assert(pair.second);
    Assert(pair.first->TypeInfo().Id == MetaType<_First>::TypeId);
    Assert(pair.second->TypeInfo().Id == MetaType<_Second>::TypeId);

    _wrapper.first = std::move(pair.first->Cast<_First>()->Wrapper());
    _wrapper.second = std::move(pair.second->Cast<_Second>()->Wrapper());
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
void MetaTypedAtom< RTTI::Pair<_First, _Second> >::CopyFrom(const RTTI::Pair<PMetaAtom, PMetaAtom>& pair) {
    Assert(pair.first);
    Assert(pair.second);
    Assert(pair.first->TypeInfo().Id == MetaType<_First>::TypeId);
    Assert(pair.second->TypeInfo().Id == MetaType<_Second>::TypeId);

    _wrapper.first = pair.first->Cast<_First>()->Wrapper();
    _wrapper.second = pair.second->Cast<_Second>()->Wrapper();
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
bool MetaTypedAtom< RTTI::Pair<_First, _Second> >::UnwrapMoveFrom(const RTTI::Pair<PMetaAtom, PMetaAtom>& pair) {
    Assert(pair.first);
    Assert(pair.second);

    if (!AssignMove(&_wrapper.first, pair.first.get()))
        return false;

    if (AssignMove(&_wrapper.second, pair.second.get()))
        return true;

    AssignMove(pair.first.get(), &_wrapper.first);
    return false;
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
bool MetaTypedAtom< RTTI::Pair<_First, _Second> >::UnwrapCopyFrom(const RTTI::Pair<PMetaAtom, PMetaAtom>& pair) {
    Assert(pair.first);
    Assert(pair.second);

    if (false == AssignCopy(&_wrapper.first, pair.first.get()))
        return false;

    // TODO - revert _wrapper.first to its original value ?
    return AssignCopy(&_wrapper.second, pair.second.get());
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
bool MetaTypedAtom< RTTI::Pair<_First, _Second> >::Equals(const RTTI::Pair<PMetaAtom, PMetaAtom>& pair) const {
    Assert(pair.first);
    Assert(pair.second);

    const MetaTypedAtom<_First> *other_first = pair.first->As<_First>();
    if (false == (other_first && (other_first->Wrapper() == _wrapper.first)))
        return false;

    const MetaTypedAtom<_Second> *other_second = pair.second->As<_Second>();
    if (false == (other_second && (other_second->Wrapper() == _wrapper.second)))
        return false;

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
void MetaTypedAtom< RTTI::Vector<T> >::MoveTo(RTTI::Vector<PMetaAtom>& vector) {
    Assert(vector.size() == _wrapper.size());

    size_t i = 0;
    for (const PMetaAtom& atom : vector) {
        Assert(atom);
        Assert(atom->TypeInfo().Id == MetaType<T>::TypeId);

        atom->Cast<T>()->Wrapper() = std::move(_wrapper.at(i++));
    }
    Assert(_wrapper.size() == i);
}
//----------------------------------------------------------------------------
template <typename T>
void MetaTypedAtom< RTTI::Vector<T> >::CopyTo(RTTI::Vector<PMetaAtom>& vector) const {
    Assert(vector.size() == _wrapper.size());

    size_t i = 0;
    for (const PMetaAtom& atom : vector) {
        Assert(atom);
        Assert(atom->TypeInfo().Id == MetaType<T>::TypeId);

        atom->Cast<T>()->Wrapper() = _wrapper.at(i++);
    }
    Assert(_wrapper.size() == i);
}
//----------------------------------------------------------------------------
template <typename T>
void MetaTypedAtom< RTTI::Vector<T> >::WrapMoveTo(RTTI::Vector<PMetaAtom>& vector) {
    vector.reserve(_wrapper.size());

    typedef typename wrapper_type::iterator iterator;
    const iterator end = _wrapper.end();
    for (iterator it = _wrapper.begin(); it != end; ++it )
        vector.emplace_back(MakeAtom(std::move(*it)));

    Assert(_wrapper.size() == vector.size());
    _wrapper.clear();
}
//----------------------------------------------------------------------------
template <typename T>
void MetaTypedAtom< RTTI::Vector<T> >::WrapCopyTo(RTTI::Vector<PMetaAtom>& vector) const {
    vector.reserve(_wrapper.size());

    typedef typename wrapper_type::const_iterator const_iterator;
    const const_iterator end = _wrapper.end();
    for (const_iterator it = _wrapper.begin(); it != end; ++it )
        vector.emplace_back(MakeAtom(*it));

    Assert(_wrapper.size() == vector.size());
}
//----------------------------------------------------------------------------
template <typename T>
void MetaTypedAtom< RTTI::Vector<T> >::MoveFrom(const RTTI::Vector<PMetaAtom>& vector) {
    _wrapper.clear();
    _wrapper.reserve(vector.size());

    for (const PMetaAtom& atom : vector) {
        Assert(atom);
        Assert(atom->TypeInfo().Id == MetaType<T>::TypeId);

        _wrapper.emplace_back(std::move(atom->Cast<T>()->Wrapper()));
    }
}
//----------------------------------------------------------------------------
template <typename T>
void MetaTypedAtom< RTTI::Vector<T> >::CopyFrom(const RTTI::Vector<PMetaAtom>& vector) {
    _wrapper.clear();
    _wrapper.reserve(vector.size());

    for (const PMetaAtom& atom : vector) {
        Assert(atom);
        Assert(atom->TypeInfo().Id == MetaType<T>::TypeId);

        _wrapper.emplace_back(atom->Cast<T>()->Wrapper());
    }
}
//----------------------------------------------------------------------------
template <typename T>
bool MetaTypedAtom< RTTI::Vector<T> >::UnwrapMoveFrom(const RTTI::Vector<PMetaAtom>& vector) {
    const size_t n = vector.size();
    for (size_t i = 0; i < n; ++i) {
        const PMetaAtom& atom = vector[i];
        Assert(atom);

        T item;
        if (false == AssignMove(&item, atom.get()) ) {
            Assert(0 == i); // _wrapper is still untouched
            return false;
        }
        else if (0 == i) {
            // now assumes that the conversion will go on
            _wrapper.clear();
            _wrapper.reserve(n);
        }

        _wrapper.emplace_back(std::move(item));
    }

    if (vector.empty())
        _wrapper.clear();

    return true;
}
//----------------------------------------------------------------------------
template <typename T>
bool MetaTypedAtom< RTTI::Vector<T> >::UnwrapCopyFrom(const RTTI::Vector<PMetaAtom>& vector) {
    const size_t n = vector.size();
    for (size_t i = 0; i < n; ++i) {
        const PMetaAtom& atom = vector[i];
        Assert(atom);

        T item;
        if (false == AssignCopy(&item, atom.get())) {
            Assert(0 == i); // _wrapper is still untouched
            return false;
        }
        else if (0 == i) {
            // now assumes that the conversion will go on
            _wrapper.clear();
            _wrapper.reserve(n);
        }

        _wrapper.emplace_back(std::move(item));
    }

    if (vector.empty())
        _wrapper.clear();

    return true;
}
//----------------------------------------------------------------------------
template <typename T>
bool MetaTypedAtom< RTTI::Vector<T> >::Equals(const RTTI::Vector<PMetaAtom>& vector) const {
    if (_wrapper.size() != vector.size())
        return false;

    const size_t n = vector.size();
    for (size_t i = 0; i < n; ++i) {
        const PMetaAtom& atom = vector[i];

        if (nullptr == atom || atom->TypeInfo().Id == MetaType<T>::TypeId)
            return false;

        if (atom->Cast<T>()->Wrapper() != _wrapper[i])
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
void MetaTypedAtom< RTTI::Dictionary<_Key, _Value> >::MoveTo(RTTI::Dictionary<PMetaAtom, PMetaAtom>& dict) {
    Assert(dict.size() == _wrapper.size());

    const size_t n = _wrapper.size();
    for (size_t i = 0; i < n; ++i) {
        Pair<_Key, _Value>& src = _wrapper.Vector()[i];
        Pair<PMetaAtom, PMetaAtom>& dst = dict.Vector()[i];

        Assert(dst.first && dst.first->TypeInfo().Id == MetaType<_Key>::TypeId);
        Assert(dst.second && dst.second->TypeInfo().Id == MetaType<_Value>::TypeId);

        dst.first->Cast<_Key>()->Wrapper() = std::move(src.first);
        dst.second->Cast<_Value>()->Wrapper() = std::move(src.second);
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
void MetaTypedAtom< RTTI::Dictionary<_Key, _Value> >::CopyTo(RTTI::Dictionary<PMetaAtom, PMetaAtom>& dict) const {
    Assert(dict.size() == _wrapper.size());

    const size_t n = _wrapper.size();
    for (size_t i = 0; i < n; ++i) {
        const Pair<_Key, _Value>& src = _wrapper.Vector()[i];
        Pair<PMetaAtom, PMetaAtom>& dst = dict.Vector()[i];

        Assert(dst.first && dst.first->TypeInfo().Id == MetaType<_Key>::TypeId);
        Assert(dst.second && dst.second->TypeInfo().Id == MetaType<_Value>::TypeId);

        dst.first->Cast<_Key>()->Wrapper() = src.first;
        dst.second->Cast<_Value>()->Wrapper() = src.second;
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
void MetaTypedAtom< RTTI::Dictionary<_Key, _Value> >::WrapMoveTo(RTTI::Dictionary<PMetaAtom, PMetaAtom>& dict) {
    dict.Vector().resize(_wrapper.Vector().size());

    const size_t n = _wrapper.size();
    for (size_t i = 0; i < n; ++i) {
        Pair<_Key, _Value>& src = _wrapper.Vector()[i];
        Pair<PMetaAtom, PMetaAtom>& dst = dict.Vector()[i];

        dst.first = MakeAtom(std::move(src.first));
        dst.second = MakeAtom(std::move(src.second));
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
void MetaTypedAtom< RTTI::Dictionary<_Key, _Value> >::WrapCopyTo(RTTI::Dictionary<PMetaAtom, PMetaAtom>& dict) const {
    dict.Vector().resize(_wrapper.Vector().size());

    const size_t n = _wrapper.size();
    for (size_t i = 0; i < n; ++i) {
        const Pair<_Key, _Value>& src = _wrapper.Vector()[i];
        Pair<PMetaAtom, PMetaAtom>& dst = dict.Vector()[i];

        dst.first = MakeAtom(src.first);
        dst.second = MakeAtom(src.second);
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
void MetaTypedAtom< RTTI::Dictionary<_Key, _Value> >::MoveFrom(const RTTI::Dictionary<PMetaAtom, PMetaAtom>& dict) {
    _wrapper.Vector().resize(dict.size());

    const size_t n = dict.size();
    for (size_t i = 0; i < n; ++i) {
        const Pair<PMetaAtom, PMetaAtom>& src = dict.Vector()[i];
        Pair<_Key, _Value>& dst = _wrapper.Vector()[i];

        Assert(src.first && src.first->TypeInfo().Id == MetaType<_Key>::TypeId);
        Assert(src.second && src.second->TypeInfo().Id == MetaType<_Value>::TypeId);

        dst.first = std::move(src.first->Cast<_Key>()->Wrapper());
        dst.second = std::move(src.second->Cast<_Value>()->Wrapper());
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
void MetaTypedAtom< RTTI::Dictionary<_Key, _Value> >::CopyFrom(const RTTI::Dictionary<PMetaAtom, PMetaAtom>& dict) {
    _wrapper.Vector().resize(dict.size());

    const size_t n = dict.size();
    for (size_t i = 0; i < n; ++i) {
        const Pair<PMetaAtom, PMetaAtom>& src = dict.Vector()[i];
        Pair<_Key, _Value>& dst = _wrapper.Vector()[i];

        Assert(src.first && src.first->TypeInfo().Id == MetaType<_Key>::TypeId);
        Assert(src.second && src.second->TypeInfo().Id == MetaType<_Value>::TypeId);

        dst.first = src.first->Cast<_Key>()->Wrapper();
        dst.second = src.second->Cast<_Value>()->Wrapper();
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
bool MetaTypedAtom< RTTI::Dictionary<_Key, _Value> >::UnwrapMoveFrom(const RTTI::Dictionary<PMetaAtom, PMetaAtom>& dict) {
    const size_t n = dict.size();
    for (size_t i = 0; i < n; ++i) {
        const Pair<PMetaAtom, PMetaAtom>& src = dict.Vector()[i];

        Pair<_Key, _Value> dst;

        if (false == AssignMove(&dst.first, src.first.get())) {
            Assert(0 == i);
            return false;
        }

        if (false == AssignMove(&dst.second, src.second.get())) {
            Assert(0 == i);
            AssignMove(src.first.get(), &dst.first);
            return false;
        }
        else if (0 == i) {
            _wrapper.clear();
            _wrapper.reserve(dict.size());
        }

        _wrapper.Vector().emplace_back(std::move(dst));
    }

    return true;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
bool MetaTypedAtom< RTTI::Dictionary<_Key, _Value> >::UnwrapCopyFrom(const RTTI::Dictionary<PMetaAtom, PMetaAtom>& dict) {
    const size_t n = dict.size();
    for (size_t i = 0; i < n; ++i) {
        const Pair<PMetaAtom, PMetaAtom>& src = dict.Vector()[i];

        Pair<_Key, _Value> dst;

        if (false == AssignCopy(&dst.first, src.first.get())) {
            Assert(0 == i);
            return false;
        }

        if (false == AssignCopy(&dst.second, src.second.get())) {
            Assert(0 == i);
            return false;
        }
        else if (0 == i) {
            _wrapper.clear();
            _wrapper.reserve(dict.size());
        }

        _wrapper.Vector().emplace_back(std::move(dst));
    }

    return true;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
bool MetaTypedAtom< RTTI::Dictionary<_Key, _Value> >::Equals(const RTTI::Dictionary<PMetaAtom, PMetaAtom>& dict) const {
    if (_wrapper.size() != dict.size())
        return false;

    const size_t n = dict.size();
    for (size_t i = 0; i < n; ++i) {
        const Pair<PMetaAtom, PMetaAtom>& v = dict.Vector()[i];

        if (false == (v.first && v.first->TypeInfo().Id == MetaType<_Key>::TypeId))
            return false;

        if (false == (v.second && v.second->TypeInfo().Id == MetaType<_Value>::TypeId))
            return false;

        const _Key& key = v.first->Cast<_Key>()->Wrapper();
        const _Value& value = v.second->Cast<_Value>()->Wrapper();

        const auto it = _wrapper.Find(key);
        if (_wrapper.end() == it)
            return false;

        if (value != it->second)
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, bool _Wrapping>
MetaWrappedAtom<T, _Wrapping>::MetaWrappedAtom() {}
//----------------------------------------------------------------------------
template <typename T, bool _Wrapping>
MetaWrappedAtom<T, _Wrapping>::~MetaWrappedAtom() {}
//----------------------------------------------------------------------------
template <typename T, bool _Wrapping>
MetaWrappedAtom<T, _Wrapping>::MetaWrappedAtom(MetaWrappedAtom&& rvalue)
:   typed_atom_type(std::move(rvalue)) {}
//----------------------------------------------------------------------------
template <typename T, bool _Wrapping>
auto MetaWrappedAtom<T, _Wrapping>::operator =(MetaWrappedAtom&& rvalue) -> MetaWrappedAtom& {
    typed_atom_type::operator =(std::move(rvalue));
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, bool _Wrapping>
MetaWrappedAtom<T, _Wrapping>::MetaWrappedAtom(const MetaWrappedAtom& other)
:   typed_atom_type(other) {}
//----------------------------------------------------------------------------
template <typename T, bool _Wrapping>
auto MetaWrappedAtom<T, _Wrapping>::operator =(const MetaWrappedAtom& other) -> MetaWrappedAtom& {
    typed_atom_type::operator =(other);
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, bool _Wrapping>
MetaWrappedAtom<T, _Wrapping>::MetaWrappedAtom(wrapper_type&& wrapper)
:   typed_atom_type(std::move(wrapper)) {}
//----------------------------------------------------------------------------
template <typename T, bool _Wrapping>
auto MetaWrappedAtom<T, _Wrapping>::operator =(wrapper_type&& wrapper) -> MetaWrappedAtom& {
    typed_atom_type::operator=(std::move(wrapper));
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, bool _Wrapping>
MetaWrappedAtom<T, _Wrapping>::MetaWrappedAtom(const wrapper_type& wrapper)
:   typed_atom_type(wrapper) {}
//----------------------------------------------------------------------------
template <typename T, bool _Wrapping>
auto MetaWrappedAtom<T, _Wrapping>::operator =(const wrapper_type& wrapper) -> MetaWrappedAtom& {
    typed_atom_type::operator =(wrapper);
    return *this;
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(RTTI, MetaWrappedAtom<T COMMA _Wrapping>, template <typename T COMMA bool _Wrapping>)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
MetaWrappedAtom<T, true>::MetaWrappedAtom() {}
//----------------------------------------------------------------------------
template <typename T>
MetaWrappedAtom<T, true>::~MetaWrappedAtom() {}
//----------------------------------------------------------------------------
template <typename T>
MetaWrappedAtom<T, true>::MetaWrappedAtom(MetaWrappedAtom&& rvalue)
:   typed_atom_type(std::move(rvalue)) {}
//----------------------------------------------------------------------------
template <typename T>
auto MetaWrappedAtom<T, true>::operator =(MetaWrappedAtom&& rvalue) -> MetaWrappedAtom& {
    typed_atom_type::operator =(std::move(rvalue));
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
MetaWrappedAtom<T, true>::MetaWrappedAtom(const MetaWrappedAtom& other)
:   typed_atom_type(other) {}
//----------------------------------------------------------------------------
template <typename T>
auto MetaWrappedAtom<T, true>::operator =(const MetaWrappedAtom& other) -> MetaWrappedAtom& {
    typed_atom_type::operator =(other);
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
MetaWrappedAtom<T, true>::MetaWrappedAtom(wrapper_type&& wrapper)
:   typed_atom_type(std::move(wrapper)) {}
//----------------------------------------------------------------------------
template <typename T>
auto MetaWrappedAtom<T, true>::operator =(wrapper_type&& wrapper) -> MetaWrappedAtom& {
    typed_atom_type::operator=(std::move(wrapper));
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
MetaWrappedAtom<T, true>::MetaWrappedAtom(const wrapper_type& wrapper)
:   typed_atom_type(wrapper) {}
//----------------------------------------------------------------------------
template <typename T>
auto MetaWrappedAtom<T, true>::operator =(const wrapper_type& wrapper) -> MetaWrappedAtom& {
    typed_atom_type::operator =(wrapper);
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
MetaWrappedAtom<T, true>::MetaWrappedAtom(wrapped_type&& wrapped) {
    meta_type_traits::WrapMove(_wrapper, std::move(wrapped));
}
//----------------------------------------------------------------------------
template <typename T>
auto MetaWrappedAtom<T, true>::operator =(wrapped_type&& wrapped) -> MetaWrappedAtom& {
    meta_type_traits::WrapMove(_wrapper, std::move(wrapped));
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
MetaWrappedAtom<T, true>::MetaWrappedAtom(const wrapped_type& wrapped) {
    meta_type_traits::WrapCopy(_wrapper, wrapped);
}
//----------------------------------------------------------------------------
template <typename T>
auto MetaWrappedAtom<T, true>::operator =(const wrapped_type& wrapped) -> MetaWrappedAtom& {
    meta_type_traits::WrapCopy(_wrapper, wrapped);
    return *this;
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(RTTI, MetaWrappedAtom<T COMMA true>, template <typename T>)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
