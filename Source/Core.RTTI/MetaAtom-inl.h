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
T *AtomValueAs(FMetaAtom *atom) {
    Assert(atom);
    auto *typed_atom = atom->As<T>;
    return (typed_atom)
        ? &typed_atom->Wrapper()
        : nullptr;
}
//----------------------------------------------------------------------------
template <typename T>
const T *AtomValueAs(const FMetaAtom *atom) {
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
TMetaTypedAtomImpl<T>::TMetaTypedAtomImpl()
    : _wrapper(meta_type::DefaultValue())
{}
//----------------------------------------------------------------------------
template <typename T>
TMetaTypedAtomImpl<T>::~TMetaTypedAtomImpl() {}
//----------------------------------------------------------------------------
template <typename T>
TMetaTypedAtomImpl<T>::TMetaTypedAtomImpl(T&& wrapper)
    : _wrapper(std::move(wrapper))
{}
//----------------------------------------------------------------------------
template <typename T>
TMetaTypedAtomImpl<T>& TMetaTypedAtomImpl<T>::operator =(T&& wrapper) {
    _wrapper = std::move(wrapper);
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
TMetaTypedAtomImpl<T>::TMetaTypedAtomImpl(const T& wrapper)
    : _wrapper(wrapper)
{}
//----------------------------------------------------------------------------
template <typename T>
TMetaTypedAtomImpl<T>& TMetaTypedAtomImpl<T>::operator =(const T& wrapper) {
    _wrapper = wrapper;
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
TMetaTypedAtomImpl<T>::TMetaTypedAtomImpl(TMetaTypedAtomImpl&& rvalue)
    : TMetaTypedAtomImpl(std::move(rvalue._wrapper))
{}
//----------------------------------------------------------------------------
template <typename T>
TMetaTypedAtomImpl<T>& TMetaTypedAtomImpl<T>::operator =(TMetaTypedAtomImpl&& rvalue) {
    return operator =(std::move(rvalue._wrapper));
}
//----------------------------------------------------------------------------
template <typename T>
TMetaTypedAtomImpl<T>::TMetaTypedAtomImpl(const TMetaTypedAtomImpl& other)
    : TMetaTypedAtomImpl(other._wrapper)
{}
//----------------------------------------------------------------------------
template <typename T>
TMetaTypedAtomImpl<T>& TMetaTypedAtomImpl<T>::operator =(const TMetaTypedAtomImpl& other) {
    _wrapper = other._wrapper;
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
FMetaTypeInfo TMetaTypedAtomImpl<T>::TypeInfo() const {
    return RTTI::TypeInfo< T >();
}
//----------------------------------------------------------------------------
template <typename T>
const IMetaTypeVirtualTraits *TMetaTypedAtomImpl<T>::Traits() const {
    return TMetaTypeTraits<T>::VirtualTraits();
}
//----------------------------------------------------------------------------
template <typename T>
bool TMetaTypedAtomImpl<T>::IsDefaultValue() const {
    return TMetaType< T >::IsDefaultValue(_wrapper);
}
//----------------------------------------------------------------------------
template <typename T>
void TMetaTypedAtomImpl<T>::MoveTo(FMetaAtom *atom) {
    Assert(atom);
    Assert(atom->TypeInfo().Id == meta_type::TypeId);

    TMetaTypedAtomImpl<T> *const other = checked_cast<TMetaTypedAtomImpl<T> *>(atom);
    other->_wrapper = std::move(_wrapper);
}
//----------------------------------------------------------------------------
template <typename T>
void TMetaTypedAtomImpl<T>::CopyTo(FMetaAtom *atom) const {
    Assert(atom);
    Assert(atom->TypeInfo().Id == meta_type::TypeId);

    TMetaTypedAtomImpl<T> *const other = checked_cast<TMetaTypedAtomImpl<T> *>(atom);
    other->_wrapper = _wrapper;
}
//----------------------------------------------------------------------------
template <typename T>
FMetaAtom *TMetaTypedAtomImpl<T>::WrapMoveTo() {
    return MakeAtom(std::move(_wrapper));
}
//----------------------------------------------------------------------------
template <typename T>
FMetaAtom *TMetaTypedAtomImpl<T>::WrapCopyTo() const {
    return MakeAtom(_wrapper);
}
//----------------------------------------------------------------------------
template <typename T>
void TMetaTypedAtomImpl<T>::MoveFrom(FMetaAtom *atom) {
    Assert(atom);
    Assert(atom->TypeInfo().Id == meta_type::TypeId);

    TMetaTypedAtomImpl<T> *const other = checked_cast<TMetaTypedAtomImpl<T> *>(atom);
    _wrapper = std::move(other->_wrapper);
}
//----------------------------------------------------------------------------
template <typename T>
void TMetaTypedAtomImpl<T>::CopyFrom(const FMetaAtom *atom) {
    Assert(atom);
    Assert(atom->TypeInfo().Id == meta_type::TypeId);

    const TMetaTypedAtomImpl<T> *const other = checked_cast<const TMetaTypedAtomImpl<T> *>(atom);
    _wrapper = other->_wrapper;
}
//----------------------------------------------------------------------------
template <typename T>
bool TMetaTypedAtomImpl<T>::Equals(const FMetaAtom *atom) const {
    Assert(atom);
    if (atom->TypeInfo().Id != meta_type::TypeId)
        return false;

    return (atom->Cast<T>()->_wrapper == _wrapper);
}
//----------------------------------------------------------------------------
template <typename T>
bool TMetaTypedAtomImpl<T>::DeepEquals(const FMetaAtom *atom) const {
    Assert(atom);
    if (atom->TypeInfo().Id != meta_type::TypeId)
        return false;

    return meta_type::DeepEquals(atom->Cast<T>()->_wrapper, _wrapper);
}
//----------------------------------------------------------------------------
template <typename T>
size_t TMetaTypedAtomImpl<T>::HashValue() const {
    using Core::hash_value;
    return hash_tuple(u64(meta_type::TypeId), _wrapper);
}
//----------------------------------------------------------------------------
template <typename T>
FString TMetaTypedAtomImpl<T>::ToString() const {
    using Core::ToString;
    return ToString(_wrapper);
}
//----------------------------------------------------------------------------
template <typename T>
void TMetaTypedAtomImpl<T>::Swap(T& wrapper) {
    swap(wrapper, _wrapper);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(RTTI, TMetaTypedAtom< T >, template <typename T>)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(RTTI, TMetaTypedAtom< RTTI::TPair<_First COMMA _Second> >, template <typename _First COMMA typename _Second>)
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
void TMetaTypedAtom< RTTI::TPair<_First, _Second> >::MoveTo(RTTI::TPair<PMetaAtom, PMetaAtom>& pair) {
    Assert(pair.first);
    Assert(pair.second);
    Assert(pair.first->TypeInfo().Id == TMetaType<_First>::TypeId);
    Assert(pair.second->TypeInfo().Id == TMetaType<_Second>::TypeId);

    pair.first->Cast<_First>()->Wrapper() = std::move(Wrapper().first);
    pair.second->Cast<_Second>()->Wrapper() = std::move(Wrapper().second);
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
void TMetaTypedAtom< RTTI::TPair<_First, _Second> >::CopyTo(RTTI::TPair<PMetaAtom, PMetaAtom>& pair) const {
    Assert(pair.first);
    Assert(pair.second);
    Assert(pair.first->TypeInfo().Id == TMetaType<_First>::TypeId);
    Assert(pair.second->TypeInfo().Id == TMetaType<_Second>::TypeId);

    pair.first->Cast<_First>()->Wrapper() = Wrapper().first;
    pair.second->Cast<_Second>()->Wrapper() = Wrapper().second;
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
void TMetaTypedAtom< RTTI::TPair<_First, _Second> >::WrapMoveTo(RTTI::TPair<PMetaAtom, PMetaAtom>& pair) {
    pair.first = MakeAtom(std::move(Wrapper().first));
    pair.second = MakeAtom(std::move(Wrapper().second));
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
void TMetaTypedAtom< RTTI::TPair<_First, _Second> >::WrapCopyTo(RTTI::TPair<PMetaAtom, PMetaAtom>& pair) const {
    pair.first = MakeAtom(Wrapper().first);
    pair.second = MakeAtom(Wrapper().second);
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
void TMetaTypedAtom< RTTI::TPair<_First, _Second> >::MoveFrom(const RTTI::TPair<PMetaAtom, PMetaAtom>& pair) {
    Assert(pair.first);
    Assert(pair.second);
    Assert(pair.first->TypeInfo().Id == TMetaType<_First>::TypeId);
    Assert(pair.second->TypeInfo().Id == TMetaType<_Second>::TypeId);

    Wrapper().first = std::move(pair.first->Cast<_First>()->Wrapper());
    Wrapper().second = std::move(pair.second->Cast<_Second>()->Wrapper());
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
void TMetaTypedAtom< RTTI::TPair<_First, _Second> >::CopyFrom(const RTTI::TPair<PMetaAtom, PMetaAtom>& pair) {
    Assert(pair.first);
    Assert(pair.second);
    Assert(pair.first->TypeInfo().Id == TMetaType<_First>::TypeId);
    Assert(pair.second->TypeInfo().Id == TMetaType<_Second>::TypeId);

    Wrapper().first = pair.first->Cast<_First>()->Wrapper();
    Wrapper().second = pair.second->Cast<_Second>()->Wrapper();
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
bool TMetaTypedAtom< RTTI::TPair<_First, _Second> >::UnwrapMoveFrom(const RTTI::TPair<PMetaAtom, PMetaAtom>& pair) {
    Assert(pair.first);
    Assert(pair.second);

    if (!AssignMove(&Wrapper().first, pair.first.get()))
        return false;

    if (AssignMove(&Wrapper().second, pair.second.get()))
        return true;

    AssignMove(pair.first.get(), &Wrapper().first);
    return false;
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
bool TMetaTypedAtom< RTTI::TPair<_First, _Second> >::UnwrapCopyFrom(const RTTI::TPair<PMetaAtom, PMetaAtom>& pair) {
    Assert(pair.first);
    Assert(pair.second);

    if (false == AssignCopy(&Wrapper().first, pair.first.get()))
        return false;

    // TODO - revert Wrapper().first to its original value ?
    return AssignCopy(&Wrapper().second, pair.second.get());
}
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
bool TMetaTypedAtom< RTTI::TPair<_First, _Second> >::Equals(const RTTI::TPair<PMetaAtom, PMetaAtom>& pair) const {
    Assert(pair.first);
    Assert(pair.second);

    const TMetaTypedAtom<_First> *other_first = pair.first->As<_First>();
    if (false == (other_first && (other_first->Wrapper() == Wrapper().first)))
        return false;

    const TMetaTypedAtom<_Second> *other_second = pair.second->As<_Second>();
    if (false == (other_second && (other_second->Wrapper() == Wrapper().second)))
        return false;

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(RTTI, TMetaTypedAtom< RTTI::TVector<T> >, template <typename T>)
//----------------------------------------------------------------------------
template <typename T>
void TMetaTypedAtom< RTTI::TVector<T> >::MoveTo(RTTI::TVector<PMetaAtom>& vector) {
    Assert(vector.size() == Wrapper().size());

    size_t i = 0;
    for (const PMetaAtom& atom : vector) {
        Assert(atom);
        Assert(atom->TypeInfo().Id == TMetaType<T>::TypeId);

        atom->Cast<T>()->Wrapper() = std::move(Wrapper().at(i++));
    }
    Assert(Wrapper().size() == i);
}
//----------------------------------------------------------------------------
template <typename T>
void TMetaTypedAtom< RTTI::TVector<T> >::CopyTo(RTTI::TVector<PMetaAtom>& vector) const {
    Assert(vector.size() == Wrapper().size());

    size_t i = 0;
    for (const PMetaAtom& atom : vector) {
        Assert(atom);
        Assert(atom->TypeInfo().Id == TMetaType<T>::TypeId);

        atom->Cast<T>()->Wrapper() = Wrapper().at(i++);
    }
    Assert(Wrapper().size() == i);
}
//----------------------------------------------------------------------------
template <typename T>
void TMetaTypedAtom< RTTI::TVector<T> >::WrapMoveTo(RTTI::TVector<PMetaAtom>& vector) {
    vector.reserve(Wrapper().size());

    typedef typename wrapper_type::iterator iterator;
    const iterator end = Wrapper().end();
    for (iterator it = Wrapper().begin(); it != end; ++it )
        vector.emplace_back(MakeAtom(std::move(*it)));

    Assert(Wrapper().size() == vector.size());
    Wrapper().clear();
}
//----------------------------------------------------------------------------
template <typename T>
void TMetaTypedAtom< RTTI::TVector<T> >::WrapCopyTo(RTTI::TVector<PMetaAtom>& vector) const {
    vector.reserve(Wrapper().size());

    typedef typename wrapper_type::const_iterator const_iterator;
    const const_iterator end = Wrapper().end();
    for (const_iterator it = Wrapper().begin(); it != end; ++it )
        vector.emplace_back(MakeAtom(*it));

    Assert(Wrapper().size() == vector.size());
}
//----------------------------------------------------------------------------
template <typename T>
void TMetaTypedAtom< RTTI::TVector<T> >::MoveFrom(const RTTI::TVector<PMetaAtom>& vector) {
    Wrapper().clear();
    Wrapper().reserve(vector.size());

    for (const PMetaAtom& atom : vector) {
        Assert(atom);
        Assert(atom->TypeInfo().Id == TMetaType<T>::TypeId);

        Wrapper().emplace_back(std::move(atom->Cast<T>()->Wrapper()));
    }
}
//----------------------------------------------------------------------------
template <typename T>
void TMetaTypedAtom< RTTI::TVector<T> >::CopyFrom(const RTTI::TVector<PMetaAtom>& vector) {
    Wrapper().clear();
    Wrapper().reserve(vector.size());

    for (const PMetaAtom& atom : vector) {
        Assert(atom);
        Assert(atom->TypeInfo().Id == TMetaType<T>::TypeId);

        Wrapper().emplace_back(atom->Cast<T>()->Wrapper());
    }
}
//----------------------------------------------------------------------------
template <typename T>
bool TMetaTypedAtom< RTTI::TVector<T> >::UnwrapMoveFrom(const RTTI::TVector<PMetaAtom>& vector) {
    const size_t n = vector.size();
    for (size_t i = 0; i < n; ++i) {
        const PMetaAtom& atom = vector[i];
        Assert(atom);

        T item;
        if (false == AssignMove(&item, atom.get()) ) {
            Assert(0 == i); // Wrapper() is still untouched
            return false;
        }
        else if (0 == i) {
            // now assumes that the conversion will go on
            Wrapper().clear();
            Wrapper().reserve(n);
        }

        Wrapper().emplace_back(std::move(item));
    }

    if (vector.empty())
        Wrapper().clear();

    return true;
}
//----------------------------------------------------------------------------
template <typename T>
bool TMetaTypedAtom< RTTI::TVector<T> >::UnwrapCopyFrom(const RTTI::TVector<PMetaAtom>& vector) {
    const size_t n = vector.size();
    for (size_t i = 0; i < n; ++i) {
        const PMetaAtom& atom = vector[i];
        Assert(atom);

        T item;
        if (false == AssignCopy(&item, atom.get())) {
            Assert(0 == i); // Wrapper() is still untouched
            return false;
        }
        else if (0 == i) {
            // now assumes that the conversion will go on
            Wrapper().clear();
            Wrapper().reserve(n);
        }

        Wrapper().emplace_back(std::move(item));
    }

    if (vector.empty())
        Wrapper().clear();

    return true;
}
//----------------------------------------------------------------------------
template <typename T>
bool TMetaTypedAtom< RTTI::TVector<T> >::Equals(const RTTI::TVector<PMetaAtom>& vector) const {
    if (Wrapper().size() != vector.size())
        return false;

    const size_t n = vector.size();
    for (size_t i = 0; i < n; ++i) {
        const PMetaAtom& atom = vector[i];

        if (nullptr == atom || atom->TypeInfo().Id == TMetaType<T>::TypeId)
            return false;

        if (atom->Cast<T>()->Wrapper() != Wrapper()[i])
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(RTTI, TMetaTypedAtom< RTTI::TDictionary<_Key COMMA _Value> >, template <typename _Key COMMA typename _Value>)
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
void TMetaTypedAtom< RTTI::TDictionary<_Key, _Value> >::MoveTo(RTTI::TDictionary<PMetaAtom, PMetaAtom>& dict) {
    Assert(dict.size() == Wrapper().size());

    const size_t n = Wrapper().size();
    for (size_t i = 0; i < n; ++i) {
        TPair<_Key, _Value>& src = Wrapper().Vector()[i];
        TPair<PMetaAtom, PMetaAtom>& dst = dict.Vector()[i];

        Assert(dst.first && dst.first->TypeInfo().Id == TMetaType<_Key>::TypeId);
        Assert(dst.second && dst.second->TypeInfo().Id == TMetaType<_Value>::TypeId);

        dst.first->Cast<_Key>()->Wrapper() = std::move(src.first);
        dst.second->Cast<_Value>()->Wrapper() = std::move(src.second);
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
void TMetaTypedAtom< RTTI::TDictionary<_Key, _Value> >::CopyTo(RTTI::TDictionary<PMetaAtom, PMetaAtom>& dict) const {
    Assert(dict.size() == Wrapper().size());

    const size_t n = Wrapper().size();
    for (size_t i = 0; i < n; ++i) {
        const TPair<_Key, _Value>& src = Wrapper().Vector()[i];
        TPair<PMetaAtom, PMetaAtom>& dst = dict.Vector()[i];

        Assert(dst.first && dst.first->TypeInfo().Id == TMetaType<_Key>::TypeId);
        Assert(dst.second && dst.second->TypeInfo().Id == TMetaType<_Value>::TypeId);

        dst.first->Cast<_Key>()->Wrapper() = src.first;
        dst.second->Cast<_Value>()->Wrapper() = src.second;
    }
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
void TMetaTypedAtom< RTTI::TDictionary<_Key, _Value> >::WrapMoveTo(RTTI::TDictionary<PMetaAtom, PMetaAtom>& dict) {
    const size_t n = Wrapper().size();
    dict.Vector().reserve(n);

    for (size_t i = 0; i < n; ++i) {
        TPair<_Key, _Value>& src = Wrapper().Vector()[i];
        TPair<PMetaAtom, PMetaAtom>& dst = dict.Vector().push_back_Default();;

        dst.first = MakeAtom(std::move(src.first));
        dst.second = MakeAtom(std::move(src.second));
    }

    Assert(dict.size() == n);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
void TMetaTypedAtom< RTTI::TDictionary<_Key, _Value> >::WrapCopyTo(RTTI::TDictionary<PMetaAtom, PMetaAtom>& dict) const {
    const size_t n = Wrapper().size();
    dict.Vector().reserve(n);

    for (size_t i = 0; i < n; ++i) {
        const TPair<_Key, _Value>& src = Wrapper().Vector()[i];
        TPair<PMetaAtom, PMetaAtom>& dst = dict.Vector().push_back_Default();

        dst.first = MakeAtom(src.first);
        dst.second = MakeAtom(src.second);
    }

    Assert(dict.size() == n);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
void TMetaTypedAtom< RTTI::TDictionary<_Key, _Value> >::MoveFrom(const RTTI::TDictionary<PMetaAtom, PMetaAtom>& dict) {
    const size_t n = dict.size();
    Wrapper().Vector().reserve(n);

    for (size_t i = 0; i < n; ++i) {
        const TPair<PMetaAtom, PMetaAtom>& src = dict.Vector()[i];

        Assert(src.first && src.first->TypeInfo().Id == TMetaType<_Key>::TypeId);
        Assert(src.second && src.second->TypeInfo().Id == TMetaType<_Value>::TypeId);

        Wrapper().Vector().emplace_back(
            std::move(src.first->Cast<_Key>()->Wrapper()),
            std::move(src.second->Cast<_Value>()->Wrapper()) );
    }

    Assert(Wrapper().size() == n);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
void TMetaTypedAtom< RTTI::TDictionary<_Key, _Value> >::CopyFrom(const RTTI::TDictionary<PMetaAtom, PMetaAtom>& dict) {
    const size_t n = dict.size();
    Wrapper().Vector().reserve(n);

    for (size_t i = 0; i < n; ++i) {
        const TPair<PMetaAtom, PMetaAtom>& src = dict.Vector()[i];

        Assert(src.first && src.first->TypeInfo().Id == TMetaType<_Key>::TypeId);
        Assert(src.second && src.second->TypeInfo().Id == TMetaType<_Value>::TypeId);

        Wrapper().Vector().emplace_back(
            src.first->Cast<_Key>()->Wrapper(),
            src.second->Cast<_Value>()->Wrapper() );
    }

    Assert(Wrapper().size() == n);
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
bool TMetaTypedAtom< RTTI::TDictionary<_Key, _Value> >::UnwrapMoveFrom(const RTTI::TDictionary<PMetaAtom, PMetaAtom>& dict) {
    const size_t n = dict.size();
    for (size_t i = 0; i < n; ++i) {
        const TPair<PMetaAtom, PMetaAtom>& src = dict.Vector()[i];

        TPair<_Key, _Value> dst;

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
            Wrapper().clear();
            Wrapper().reserve(dict.size());
        }

        Wrapper().Vector().emplace_back(std::move(dst));
    }

    Assert(Wrapper().size() == n);
    return true;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
bool TMetaTypedAtom< RTTI::TDictionary<_Key, _Value> >::UnwrapCopyFrom(const RTTI::TDictionary<PMetaAtom, PMetaAtom>& dict) {
    const size_t n = dict.size();
    for (size_t i = 0; i < n; ++i) {
        const TPair<PMetaAtom, PMetaAtom>& src = dict.Vector()[i];

        TPair<_Key, _Value> dst;

        if (false == AssignCopy(&dst.first, src.first.get())) {
            Assert(0 == i);
            return false;
        }

        if (false == AssignCopy(&dst.second, src.second.get())) {
            Assert(0 == i);
            return false;
        }
        else if (0 == i) {
            Wrapper().clear();
            Wrapper().reserve(dict.size());
        }

        Wrapper().Vector().emplace_back(std::move(dst));
    }

    return true;
}
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
bool TMetaTypedAtom< RTTI::TDictionary<_Key, _Value> >::Equals(const RTTI::TDictionary<PMetaAtom, PMetaAtom>& dict) const {
    if (Wrapper().size() != dict.size())
        return false;

    const size_t n = dict.size();
    for (size_t i = 0; i < n; ++i) {
        const TPair<PMetaAtom, PMetaAtom>& v = dict.Vector()[i];

        if (false == (v.first && v.first->TypeInfo().Id == TMetaType<_Key>::TypeId))
            return false;

        if (false == (v.second && v.second->TypeInfo().Id == TMetaType<_Value>::TypeId))
            return false;

        const _Key& key = v.first->Cast<_Key>()->Wrapper();
        const _Value& value = v.second->Cast<_Value>()->Wrapper();

        const auto it = Wrapper().Find(key);
        if (Wrapper().end() == it)
            return false;

        if (value != it->second)
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
typename TMetaAtomWrapper<T>::type *MakeAtom(T&& rvalue, typename std::enable_if< TMetaAtomWrapper<T>::need_wrapper::value >::type* /* = 0 */) {
    typedef TMetaTypeTraits<T> traits_type;
    typedef typename TMetaAtomWrapper<T>::type atom_type;
    typename traits_type::wrapper_type wrapper;
    traits_type::WrapMove(wrapper, std::move(rvalue));
    return new atom_type(std::move(wrapper));
}
//----------------------------------------------------------------------------
template <typename T>
typename TMetaAtomWrapper<T>::type *MakeAtom(const T& value, typename std::enable_if< TMetaAtomWrapper<T>::need_wrapper::value >::type* /* = 0 */) {
    typedef TMetaTypeTraits<T> traits_type;
    typedef typename TMetaAtomWrapper<T>::type atom_type;
    typename traits_type::wrapper_type wrapper;
    traits_type::WrapCopy(wrapper, value);
    return new atom_type(std::move(wrapper));
}
//----------------------------------------------------------------------------
template <typename T>
typename TMetaAtomWrapper<T>::type *MakeAtom(T&& rvalue, typename std::enable_if< TMetaAtomWrapper<T>::dont_need_wrapper::value >::type* /* = 0 */) {
    typedef typename TMetaAtomWrapper<T>::type atom_type;
    return new atom_type(std::forward<T>(rvalue));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
