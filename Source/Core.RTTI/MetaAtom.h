#pragma once

#include "Core.RTTI/RTTI_fwd.h"

#include "Core/Memory/RefPtr.h"

#include "Core.RTTI/MetaAtomVisitor.h"

#include "Core.RTTI/MetaType.h"
#include "Core.RTTI/MetaTypeTraits.h"
#include "Core.RTTI/MetaTypeVirtualTraits.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Maths/Geometry/ScalarVector.h"
#include "Core/Maths/Transform/ScalarMatrix.h"

#include <iosfwd>

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class MetaTypedAtom;
FWD_REFPTR(MetaAtom);
class IMetaAtomPair;
class IMetaAtomVector;
class IMetaAtomDictionary;
//----------------------------------------------------------------------------
class MetaAtom : public RefCountable {
public:
    MetaAtom() {}
    virtual ~MetaAtom() {}

    virtual MetaTypeInfo TypeInfo() const = 0;
    virtual const IMetaTypeVirtualTraits *Traits() const = 0;

    virtual bool IsDefaultValue() const = 0;

    virtual void MoveTo(MetaAtom *atom) = 0;
    virtual void CopyTo(MetaAtom *atom) const = 0;

    virtual MetaAtom *WrapMoveTo() = 0;
    virtual MetaAtom *WrapCopyTo() const = 0;

    virtual void MoveFrom(MetaAtom *atom) = 0;
    virtual void CopyFrom(const MetaAtom *atom) = 0;

    virtual bool Equals(const MetaAtom *atom) const = 0;

    virtual size_t HashValue() const = 0;
    virtual String ToString() const = 0;

    virtual IMetaAtomPair *AsPair() = 0;
    virtual const IMetaAtomPair *AsPair() const = 0;

    virtual IMetaAtomVector *AsVector() = 0;
    virtual const IMetaAtomVector *AsVector() const = 0;

    virtual IMetaAtomDictionary *AsDictionary() = 0;
    virtual const IMetaAtomDictionary *AsDictionary() const = 0;

    virtual void Accept(IMetaAtomVisitor* visitor) = 0;
    virtual void Accept(IMetaAtomConstVisitor* visitor) const = 0;

    template <typename T>
    MetaTypedAtom< typename MetaTypeTraits<T>::wrapper_type > *Cast() {
        return checked_cast<MetaTypedAtom< typename MetaTypeTraits<T>::wrapper_type > *>(this);
    }

    template <typename T>
    const MetaTypedAtom< typename MetaTypeTraits<T>::wrapper_type > *Cast() const {
        return checked_cast<const MetaTypedAtom< typename MetaTypeTraits<T>::wrapper_type > *>(this);
    }

    template <typename T>
    MetaTypedAtom< typename MetaTypeTraits<T>::wrapper_type > *As() {
        return dynamic_cast<MetaTypedAtom< typename MetaTypeTraits<T>::wrapper_type > *>(this);
    }

    template <typename T>
    const MetaTypedAtom< typename MetaTypeTraits<T>::wrapper_type > *As() const {
        return dynamic_cast<const MetaTypedAtom< typename MetaTypeTraits<T>::wrapper_type > *>(this);
    }
};
//----------------------------------------------------------------------------
template <typename T>
T *AtomValueAs(MetaAtom *atom);
//----------------------------------------------------------------------------
template <typename T>
const T *AtomValueAs(const MetaAtom *atom);
//----------------------------------------------------------------------------
inline hash_t hash_value(const MetaAtom& atom) { return atom.HashValue(); }
inline hash_t hash_value(const PMetaAtom& patom) { return patom ? patom->HashValue() : 0; }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IMetaAtomPair {
public:
    virtual ~IMetaAtomPair() {}

    virtual const MetaAtom* Atom() const = 0;

    virtual MetaTypeInfo FirstTypeInfo() const = 0;
    virtual MetaTypeInfo SecondTypeInfo() const = 0;

    virtual const IMetaTypeVirtualTraits *FirstTraits() const = 0;
    virtual const IMetaTypeVirtualTraits *SecondTraits() const = 0;

    virtual void MoveTo(RTTI::Pair<PMetaAtom, PMetaAtom>& pair) = 0;
    virtual void CopyTo(RTTI::Pair<PMetaAtom, PMetaAtom>& pair) const = 0;

    virtual void WrapMoveTo(RTTI::Pair<PMetaAtom, PMetaAtom>& pair) = 0;
    virtual void WrapCopyTo(RTTI::Pair<PMetaAtom, PMetaAtom>& pair) const = 0;

    virtual void MoveFrom(const RTTI::Pair<PMetaAtom, PMetaAtom>& pair) = 0;
    virtual void CopyFrom(const RTTI::Pair<PMetaAtom, PMetaAtom>& pair) = 0;

    virtual bool UnwrapMoveFrom(const RTTI::Pair<PMetaAtom, PMetaAtom>& pair) = 0;
    virtual bool UnwrapCopyFrom(const RTTI::Pair<PMetaAtom, PMetaAtom>& pair) = 0;

    virtual bool Equals(const RTTI::Pair<PMetaAtom, PMetaAtom>& pair) const = 0;
};
//----------------------------------------------------------------------------
class IMetaAtomVector {
public:
    virtual ~IMetaAtomVector() {}

    virtual const MetaAtom* Atom() const = 0;

    virtual MetaTypeInfo ValueTypeInfo() const = 0;
    virtual const IMetaTypeVirtualTraits *ValueTraits() const = 0;

    virtual void MoveTo(RTTI::Vector<PMetaAtom>& vector) = 0;
    virtual void CopyTo(RTTI::Vector<PMetaAtom>& vector) const = 0;

    virtual void WrapMoveTo(RTTI::Vector<PMetaAtom>& vector) = 0;
    virtual void WrapCopyTo(RTTI::Vector<PMetaAtom>& vector) const = 0;

    virtual void MoveFrom(const RTTI::Vector<PMetaAtom>& vector) = 0;
    virtual void CopyFrom(const RTTI::Vector<PMetaAtom>& vector) = 0;

    virtual bool UnwrapMoveFrom(const RTTI::Vector<PMetaAtom>& vector) = 0;
    virtual bool UnwrapCopyFrom(const RTTI::Vector<PMetaAtom>& vector) = 0;

    virtual bool Equals(const RTTI::Vector<PMetaAtom>& vector) const = 0;
};
//----------------------------------------------------------------------------
class IMetaAtomDictionary {
public:
    virtual ~IMetaAtomDictionary() {}

    virtual const MetaAtom* Atom() const = 0;

    virtual MetaTypeInfo KeyTypeInfo() const = 0;
    virtual MetaTypeInfo ValueTypeInfo() const = 0;

    virtual const IMetaTypeVirtualTraits *KeyTraits() const = 0;
    virtual const IMetaTypeVirtualTraits *ValueTraits() const = 0;

    virtual void MoveTo(RTTI::Dictionary<PMetaAtom, PMetaAtom>& dict) = 0;
    virtual void CopyTo(RTTI::Dictionary<PMetaAtom, PMetaAtom>& dict) const = 0;

    virtual void WrapMoveTo(RTTI::Dictionary<PMetaAtom, PMetaAtom>& dict) = 0;
    virtual void WrapCopyTo(RTTI::Dictionary<PMetaAtom, PMetaAtom>& dict) const = 0;

    virtual void MoveFrom(const RTTI::Dictionary<PMetaAtom, PMetaAtom>& dict) = 0;
    virtual void CopyFrom(const RTTI::Dictionary<PMetaAtom, PMetaAtom>& dict) = 0;

    virtual bool UnwrapMoveFrom(const RTTI::Dictionary<PMetaAtom, PMetaAtom>& dict) = 0;
    virtual bool UnwrapCopyFrom(const RTTI::Dictionary<PMetaAtom, PMetaAtom>& dict) = 0;

    virtual bool Equals(const RTTI::Dictionary<PMetaAtom, PMetaAtom>& dict) const = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class MetaTypedAtomImpl : public MetaAtom {
public:
    typedef MetaType<T> meta_type;

    MetaTypedAtomImpl();
    virtual ~MetaTypedAtomImpl();

    MetaTypedAtomImpl(T&& wrapper);
    MetaTypedAtomImpl& operator =(T&& wrapper);

    MetaTypedAtomImpl(const T& wrapper);
    MetaTypedAtomImpl& operator =(const T& wrapper);

    MetaTypedAtomImpl(MetaTypedAtomImpl&& rvalue);
    MetaTypedAtomImpl& operator =(MetaTypedAtomImpl&& rvalue);

    MetaTypedAtomImpl(const MetaTypedAtomImpl& other);
    MetaTypedAtomImpl& operator =(const MetaTypedAtomImpl& other);

    T& Wrapper() { return _wrapper; }
    const T& Wrapper() const { return _wrapper; }

    virtual MetaTypeInfo TypeInfo() const override;
    virtual const IMetaTypeVirtualTraits *Traits() const override;

    virtual bool IsDefaultValue() const override;

    virtual void MoveTo(MetaAtom *atom) override;
    virtual void CopyTo(MetaAtom *atom) const override;

    virtual MetaAtom *WrapMoveTo() override;
    virtual MetaAtom *WrapCopyTo() const override;

    virtual void MoveFrom(MetaAtom *atom) override;
    virtual void CopyFrom(const MetaAtom *atom) override;

    virtual bool Equals(const MetaAtom *atom) const override;

    virtual size_t HashValue() const override;
    virtual String ToString() const override;

    void Swap(T& wrapper);

protected:
    T _wrapper;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class MetaTypedAtom : public MetaTypedAtomImpl<T> {
public:
    typedef T wrapper_type;
    typedef MetaTypedAtomImpl< wrapper_type > impl_type;

    MetaTypedAtom() {}
    virtual ~MetaTypedAtom() {}

    MetaTypedAtom(wrapper_type&& wrapper) : impl_type(std::move(wrapper)) {}
    MetaTypedAtom& operator =(T&& wrapper) { impl_type::operator =(std::move(wrapper)); return *this; }

    MetaTypedAtom(const wrapper_type& wrapper) : impl_type(wrapper) {}
    MetaTypedAtom& operator =(const wrapper_type& wrapper) { impl_type::operator =(wrapper); return *this; }

    MetaTypedAtom(MetaTypedAtom&& rvalue) : impl_type(std::move(rvalue)) {}
    MetaTypedAtom& operator =(MetaTypedAtom&& rvalue) { impl_type::operator =(std::move(rvalue)); return *this; }

    MetaTypedAtom(const MetaTypedAtom& other) : impl_type(other) {}
    MetaTypedAtom& operator =(const MetaTypedAtom& other) { impl_type::operator =(other); return *this; }

    virtual IMetaAtomPair *AsPair() override { return nullptr; }
    virtual const IMetaAtomPair *AsPair() const override { return nullptr; }

    virtual IMetaAtomVector *AsVector() override { return nullptr; }
    virtual const IMetaAtomVector *AsVector() const override { return nullptr; }

    virtual IMetaAtomDictionary *AsDictionary() override { return nullptr; }
    virtual const IMetaAtomDictionary *AsDictionary() const override { return nullptr; }

    virtual void Accept(IMetaAtomVisitor* visitor) override { visitor->Visit(this); }
    virtual void Accept(IMetaAtomConstVisitor* visitor) const override { visitor->Visit(this); }

    SINGLETON_POOL_ALLOCATED_DECL();
};
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
class MetaTypedAtom< RTTI::Pair<_First, _Second> > : public MetaTypedAtomImpl< RTTI::Pair<_First, _Second> >, public IMetaAtomPair {
public:
    typedef RTTI::Pair<_First, _Second> wrapper_type;
    typedef MetaTypedAtomImpl< wrapper_type > impl_type;

    MetaTypedAtom() {}
    virtual ~MetaTypedAtom() {}

    MetaTypedAtom(wrapper_type&& wrapper) : impl_type(std::move(wrapper)) {}
    MetaTypedAtom& operator =(wrapper_type&& wrapper) { impl_type::operator =(std::move(wrapper)); return *this; }

    MetaTypedAtom(const wrapper_type& wrapper) : impl_type(wrapper) {}
    MetaTypedAtom& operator =(const wrapper_type& wrapper) { impl_type::operator =(wrapper); return *this; }

    MetaTypedAtom(MetaTypedAtom&& rvalue) : impl_type(std::move(rvalue)) {}
    MetaTypedAtom& operator =(MetaTypedAtom&& rvalue) { impl_type::operator =(std::move(rvalue)); return *this; }

    MetaTypedAtom(const MetaTypedAtom& other) : impl_type(other) {}
    MetaTypedAtom& operator =(const MetaTypedAtom& other) { impl_type::operator =(other); return *this; }

    virtual IMetaAtomPair *AsPair() override { return this; }
    virtual const IMetaAtomPair *AsPair() const override { return this; }

    virtual IMetaAtomVector *AsVector() override { return nullptr; }
    virtual const IMetaAtomVector *AsVector() const override { return nullptr; }

    virtual IMetaAtomDictionary *AsDictionary() override { return nullptr; }
    virtual const IMetaAtomDictionary *AsDictionary() const override { return nullptr; }

    using impl_type::IsDefaultValue;
    using impl_type::MoveTo;
    using impl_type::CopyTo;
    using impl_type::WrapMoveTo;
    using impl_type::WrapCopyTo;
    using impl_type::MoveFrom;
    using impl_type::CopyFrom;
    using impl_type::Equals;

    virtual void Accept(IMetaAtomVisitor* visitor) override { visitor->Visit(this); }
    virtual void Accept(IMetaAtomConstVisitor* visitor) const override { visitor->Visit(this); }

    SINGLETON_POOL_ALLOCATED_DECL();

    // IMetaAtomPair interface

private:
    virtual const MetaAtom* Atom() const override { return this; }

    virtual MetaTypeInfo FirstTypeInfo() const override { return RTTI::TypeInfo< _First >(); }
    virtual MetaTypeInfo SecondTypeInfo() const override { return RTTI::TypeInfo< _Second >(); }

    virtual const IMetaTypeVirtualTraits *FirstTraits() const override { return MetaTypeTraits< _First >::VirtualTraits(); }
    virtual const IMetaTypeVirtualTraits *SecondTraits() const override { return MetaTypeTraits< _Second >::VirtualTraits(); }

    virtual void MoveTo(RTTI::Pair<PMetaAtom, PMetaAtom>& pair) override;
    virtual void CopyTo(RTTI::Pair<PMetaAtom, PMetaAtom>& pair) const override;

    virtual void WrapMoveTo(RTTI::Pair<PMetaAtom, PMetaAtom>& pair) override;
    virtual void WrapCopyTo(RTTI::Pair<PMetaAtom, PMetaAtom>& pair) const override;

    virtual void MoveFrom(const RTTI::Pair<PMetaAtom, PMetaAtom>& pair) override;
    virtual void CopyFrom(const RTTI::Pair<PMetaAtom, PMetaAtom>& pair) override;

    virtual bool UnwrapMoveFrom(const RTTI::Pair<PMetaAtom, PMetaAtom>& pair) override;
    virtual bool UnwrapCopyFrom(const RTTI::Pair<PMetaAtom, PMetaAtom>& pair) override;

    virtual bool Equals(const RTTI::Pair<PMetaAtom, PMetaAtom>& pair) const override;
};
//----------------------------------------------------------------------------
template <typename T>
class MetaTypedAtom< RTTI::Vector<T> > : public MetaTypedAtomImpl< RTTI::Vector<T> >, public IMetaAtomVector {
public:
    typedef RTTI::Vector<T> wrapper_type;
    typedef MetaTypedAtomImpl< wrapper_type > impl_type;

    MetaTypedAtom() {}
    virtual ~MetaTypedAtom() {}

    MetaTypedAtom(wrapper_type&& wrapper) : impl_type(std::move(wrapper)) {}
    MetaTypedAtom& operator =(wrapper_type&& wrapper) { impl_type::operator =(std::move(wrapper)); return *this; }

    MetaTypedAtom(const wrapper_type& wrapper) : impl_type(wrapper) {}
    MetaTypedAtom& operator =(const wrapper_type& wrapper) { impl_type::operator =(wrapper); return *this; }

    MetaTypedAtom(MetaTypedAtom&& rvalue) : impl_type(std::move(rvalue)) {}
    MetaTypedAtom& operator =(MetaTypedAtom&& rvalue) { impl_type::operator =(std::move(rvalue)); return *this; }

    MetaTypedAtom(const MetaTypedAtom& other) : impl_type(other) {}
    MetaTypedAtom& operator =(const MetaTypedAtom& other) { impl_type::operator =(other); return *this; }

    virtual IMetaAtomPair *AsPair() override { return nullptr; }
    virtual const IMetaAtomPair *AsPair() const override { return nullptr; }

    virtual IMetaAtomVector *AsVector() override { return this; }
    virtual const IMetaAtomVector *AsVector() const override { return this; }

    virtual IMetaAtomDictionary *AsDictionary() override { return nullptr; }
    virtual const IMetaAtomDictionary *AsDictionary() const override { return nullptr; }

    using impl_type::IsDefaultValue;
    using impl_type::MoveTo;
    using impl_type::CopyTo;
    using impl_type::WrapMoveTo;
    using impl_type::WrapCopyTo;
    using impl_type::MoveFrom;
    using impl_type::CopyFrom;
    using impl_type::Equals;

    virtual void Accept(IMetaAtomVisitor* visitor) override { visitor->Visit(this); }
    virtual void Accept(IMetaAtomConstVisitor* visitor) const override { visitor->Visit(this); }

    SINGLETON_POOL_ALLOCATED_DECL();

    // IMetaAtomVector interface

private:
    virtual const MetaAtom* Atom() const override { return this; }

    virtual MetaTypeInfo ValueTypeInfo() const override { return RTTI::TypeInfo< T >(); }
    virtual const IMetaTypeVirtualTraits *ValueTraits() const override { return MetaTypeTraits< T >::VirtualTraits(); }

    virtual void MoveTo(RTTI::Vector<PMetaAtom>& vector) override;
    virtual void CopyTo(RTTI::Vector<PMetaAtom>& vector) const override;

    virtual void WrapMoveTo(RTTI::Vector<PMetaAtom>& vector) override;
    virtual void WrapCopyTo(RTTI::Vector<PMetaAtom>& vector) const override;

    virtual void MoveFrom(const RTTI::Vector<PMetaAtom>& vector) override;
    virtual void CopyFrom(const RTTI::Vector<PMetaAtom>& vector) override;

    virtual bool UnwrapMoveFrom(const RTTI::Vector<PMetaAtom>& vector) override;
    virtual bool UnwrapCopyFrom(const RTTI::Vector<PMetaAtom>& vector) override;

    virtual bool Equals(const RTTI::Vector<PMetaAtom>& vector) const override;
};
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
class MetaTypedAtom< RTTI::Dictionary<_Key, _Value> > : public MetaTypedAtomImpl< RTTI::Dictionary<_Key, _Value> >, public IMetaAtomDictionary {
public:
    typedef RTTI::Dictionary<_Key, _Value> wrapper_type;
    typedef MetaTypedAtomImpl< wrapper_type > impl_type;

    MetaTypedAtom() {}
    virtual ~MetaTypedAtom() {}

    MetaTypedAtom(wrapper_type&& wrapper) : impl_type(std::move(wrapper)) {}
    MetaTypedAtom& operator =(wrapper_type&& wrapper) { impl_type::operator =(std::move(wrapper)); return *this; }

    MetaTypedAtom(const wrapper_type& wrapper) : impl_type(wrapper) {}
    MetaTypedAtom& operator =(const wrapper_type& wrapper) { impl_type::operator =(wrapper); return *this; }

    MetaTypedAtom(MetaTypedAtom&& rvalue) : impl_type(std::move(rvalue)) {}
    MetaTypedAtom& operator =(MetaTypedAtom&& rvalue) { impl_type::operator =(std::move(rvalue)); return *this; }

    MetaTypedAtom(const MetaTypedAtom& other) : impl_type(other) {}
    MetaTypedAtom& operator =(const MetaTypedAtom& other) { impl_type::operator =(other); return *this; }

    virtual IMetaAtomPair *AsPair() override { return nullptr; }
    virtual const IMetaAtomPair *AsPair() const override { return nullptr; }

    virtual IMetaAtomVector *AsVector() override { return nullptr; }
    virtual const IMetaAtomVector *AsVector() const override { return nullptr; }

    virtual IMetaAtomDictionary *AsDictionary() override { return this; }
    virtual const IMetaAtomDictionary *AsDictionary() const override { return this; }

    using impl_type::IsDefaultValue;
    using impl_type::MoveTo;
    using impl_type::CopyTo;
    using impl_type::WrapMoveTo;
    using impl_type::WrapCopyTo;
    using impl_type::MoveFrom;
    using impl_type::CopyFrom;
    using impl_type::Equals;

    virtual void Accept(IMetaAtomVisitor* visitor) override { visitor->Visit(this); }
    virtual void Accept(IMetaAtomConstVisitor* visitor) const override { visitor->Visit(this); }

    SINGLETON_POOL_ALLOCATED_DECL();

    // IMetaAtomDictionary interface

private:
    virtual const MetaAtom* Atom() const override { return this; }

    virtual MetaTypeInfo KeyTypeInfo() const override { return RTTI::TypeInfo< _Key >(); }
    virtual MetaTypeInfo ValueTypeInfo() const override { return RTTI::TypeInfo< _Value >(); }

    virtual const IMetaTypeVirtualTraits *KeyTraits() const override { return MetaTypeTraits< _Key >::VirtualTraits(); }
    virtual const IMetaTypeVirtualTraits *ValueTraits() const override { return MetaTypeTraits< _Value >::VirtualTraits(); }

    virtual void MoveTo(RTTI::Dictionary<PMetaAtom, PMetaAtom>& dict) override;
    virtual void CopyTo(RTTI::Dictionary<PMetaAtom, PMetaAtom>& dict) const override;

    virtual void WrapMoveTo(RTTI::Dictionary<PMetaAtom, PMetaAtom>& dict) override;
    virtual void WrapCopyTo(RTTI::Dictionary<PMetaAtom, PMetaAtom>& dict) const override;

    virtual void MoveFrom(const RTTI::Dictionary<PMetaAtom, PMetaAtom>& dict) override;
    virtual void CopyFrom(const RTTI::Dictionary<PMetaAtom, PMetaAtom>& dict) override;

    virtual bool UnwrapMoveFrom(const RTTI::Dictionary<PMetaAtom, PMetaAtom>& dict) override;
    virtual bool UnwrapCopyFrom(const RTTI::Dictionary<PMetaAtom, PMetaAtom>& dict) override;

    virtual bool Equals(const RTTI::Dictionary<PMetaAtom, PMetaAtom>& dict) const override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct MetaAtomWrapper {
    typedef MetaTypeTraits< typename std::decay<T>::type > trais_type;
    typedef typename std::is_same<
        typename trais_type::wrapper_type,
        typename trais_type::wrapped_type
    >::type dont_need_wrapper;
    typedef std::integral_constant<bool, false == dont_need_wrapper::value> need_wrapper;
    typedef MetaTypedAtom< typename trais_type::wrapper_type > type;
};
//----------------------------------------------------------------------------
template <typename T>
typename MetaAtomWrapper<T>::type *MakeAtom(T&& rvalue, typename std::enable_if< MetaAtomWrapper<T>::need_wrapper::value >::type* = 0 );
//----------------------------------------------------------------------------
template <typename T>
typename MetaAtomWrapper<T>::type *MakeAtom(const T& value, typename std::enable_if< MetaAtomWrapper<T>::need_wrapper::value >::type* = 0 );
//----------------------------------------------------------------------------
template <typename T>
typename MetaAtomWrapper<T>::type *MakeAtom(T&& rvalue, typename std::enable_if< MetaAtomWrapper<T>::dont_need_wrapper::value >::type* = 0 );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const MetaTypedAtom<T>& typedAtom) {
    return oss << typedAtom.Wrapper();
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const MetaAtom *atom) {
    Assert(atom);
    return oss << atom->ToString();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

#include "Core.RTTI/MetaAtom-inl.h"
