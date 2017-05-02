#pragma once

#include "Core.RTTI/RTTI_fwd.h"

#include "Core/Memory/RefPtr.h"

#include "Core.RTTI/MetaAtomVisitor.h"

#include "Core.RTTI/MetaType.h"
#include "Core.RTTI/MetaTypeTraits.h"
#include "Core.RTTI/MetaTypeVirtualTraits.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/ScalarMatrix.h"

#include <iosfwd>

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TMetaTypedAtom;
FWD_REFPTR(MetaAtom);
class IMetaAtomPair;
class IMetaAtomVector;
class IMetaAtomDictionary;
//----------------------------------------------------------------------------
class FMetaAtom : public FRefCountable {
public:
    FMetaAtom() {}
    virtual ~FMetaAtom() {}

    virtual FMetaTypeInfo TypeInfo() const = 0;
    virtual const IMetaTypeVirtualTraits *Traits() const = 0;

    virtual bool IsDefaultValue() const = 0;

    virtual void MoveTo(FMetaAtom *atom) = 0;
    virtual void CopyTo(FMetaAtom *atom) const = 0;

    virtual FMetaAtom *WrapMoveTo() = 0;
    virtual FMetaAtom *WrapCopyTo() const = 0;

    virtual void MoveFrom(FMetaAtom *atom) = 0;
    virtual void CopyFrom(const FMetaAtom *atom) = 0;

    virtual bool Equals(const FMetaAtom *atom) const = 0;
    virtual bool DeepEquals(const FMetaAtom *atom) const = 0;

    virtual size_t HashValue() const = 0;
    virtual FString ToString() const = 0;

    virtual IMetaAtomPair *AsPair() = 0;
    virtual const IMetaAtomPair *AsPair() const = 0;

    virtual IMetaAtomVector *AsVector() = 0;
    virtual const IMetaAtomVector *AsVector() const = 0;

    virtual IMetaAtomDictionary *AsDictionary() = 0;
    virtual const IMetaAtomDictionary *AsDictionary() const = 0;

    virtual void Accept(IMetaAtomVisitor* visitor) = 0;
    virtual void Accept(IMetaAtomConstVisitor* visitor) const = 0;

    template <typename T>
    TMetaTypedAtom< typename TMetaTypeTraits<T>::wrapper_type > *Cast() {
        typedef typename TMetaTypeTraits<T>::meta_type meta_type;
        Assert(TypeInfo().Id == meta_type::TypeId);
        return checked_cast<TMetaTypedAtom< typename TMetaTypeTraits<T>::wrapper_type > *>(this);
    }

    template <typename T>
    const TMetaTypedAtom< typename TMetaTypeTraits<T>::wrapper_type > *Cast() const {
        return const_cast<FMetaAtom*>(this)->Cast<T>();
    }

    template <typename T>
    TMetaTypedAtom< typename TMetaTypeTraits<T>::wrapper_type > *As() {
        typedef typename TMetaTypeTraits<T>::meta_type meta_type;
        return (TypeInfo().Id == meta_type::TypeId ? Cast<T>() : nullptr);
    }

    template <typename T>
    const TMetaTypedAtom< typename TMetaTypeTraits<T>::wrapper_type > *As() const {
        return const_cast<FMetaAtom*>(this)->As<T>();
    }
};
//----------------------------------------------------------------------------
template <typename T>
T *AtomValueAs(FMetaAtom *atom);
//----------------------------------------------------------------------------
template <typename T>
const T *AtomValueAs(const FMetaAtom *atom);
//----------------------------------------------------------------------------
inline hash_t hash_value(const FMetaAtom& atom) { return atom.HashValue(); }
inline hash_t hash_value(const PMetaAtom& patom) { return patom ? patom->HashValue() : 0; }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IMetaAtomPair {
public:
    virtual ~IMetaAtomPair() {}

    virtual const FMetaAtom* Atom() const = 0;

    virtual FMetaTypeInfo FirstTypeInfo() const = 0;
    virtual FMetaTypeInfo SecondTypeInfo() const = 0;

    virtual const IMetaTypeVirtualTraits *FirstTraits() const = 0;
    virtual const IMetaTypeVirtualTraits *SecondTraits() const = 0;

    virtual void MoveTo(RTTI::TPair<PMetaAtom, PMetaAtom>& pair) = 0;
    virtual void CopyTo(RTTI::TPair<PMetaAtom, PMetaAtom>& pair) const = 0;

    virtual void WrapMoveTo(RTTI::TPair<PMetaAtom, PMetaAtom>& pair) = 0;
    virtual void WrapCopyTo(RTTI::TPair<PMetaAtom, PMetaAtom>& pair) const = 0;

    virtual void MoveFrom(const RTTI::TPair<PMetaAtom, PMetaAtom>& pair) = 0;
    virtual void CopyFrom(const RTTI::TPair<PMetaAtom, PMetaAtom>& pair) = 0;

    virtual bool UnwrapMoveFrom(const RTTI::TPair<PMetaAtom, PMetaAtom>& pair) = 0;
    virtual bool UnwrapCopyFrom(const RTTI::TPair<PMetaAtom, PMetaAtom>& pair) = 0;

    virtual bool Equals(const RTTI::TPair<PMetaAtom, PMetaAtom>& pair) const = 0;
};
//----------------------------------------------------------------------------
class IMetaAtomVector {
public:
    virtual ~IMetaAtomVector() {}

    virtual const FMetaAtom* Atom() const = 0;

    virtual FMetaTypeInfo ValueTypeInfo() const = 0;
    virtual const IMetaTypeVirtualTraits *ValueTraits() const = 0;

    virtual void MoveTo(RTTI::TVector<PMetaAtom>& vector) = 0;
    virtual void CopyTo(RTTI::TVector<PMetaAtom>& vector) const = 0;

    virtual void WrapMoveTo(RTTI::TVector<PMetaAtom>& vector) = 0;
    virtual void WrapCopyTo(RTTI::TVector<PMetaAtom>& vector) const = 0;

    virtual void MoveFrom(const RTTI::TVector<PMetaAtom>& vector) = 0;
    virtual void CopyFrom(const RTTI::TVector<PMetaAtom>& vector) = 0;

    virtual bool UnwrapMoveFrom(const RTTI::TVector<PMetaAtom>& vector) = 0;
    virtual bool UnwrapCopyFrom(const RTTI::TVector<PMetaAtom>& vector) = 0;

    virtual bool Equals(const RTTI::TVector<PMetaAtom>& vector) const = 0;
};
//----------------------------------------------------------------------------
class IMetaAtomDictionary {
public:
    virtual ~IMetaAtomDictionary() {}

    virtual const FMetaAtom* Atom() const = 0;

    virtual FMetaTypeInfo KeyTypeInfo() const = 0;
    virtual FMetaTypeInfo ValueTypeInfo() const = 0;

    virtual const IMetaTypeVirtualTraits *KeyTraits() const = 0;
    virtual const IMetaTypeVirtualTraits *ValueTraits() const = 0;

    virtual void MoveTo(RTTI::TDictionary<PMetaAtom, PMetaAtom>& dict) = 0;
    virtual void CopyTo(RTTI::TDictionary<PMetaAtom, PMetaAtom>& dict) const = 0;

    virtual void WrapMoveTo(RTTI::TDictionary<PMetaAtom, PMetaAtom>& dict) = 0;
    virtual void WrapCopyTo(RTTI::TDictionary<PMetaAtom, PMetaAtom>& dict) const = 0;

    virtual void MoveFrom(const RTTI::TDictionary<PMetaAtom, PMetaAtom>& dict) = 0;
    virtual void CopyFrom(const RTTI::TDictionary<PMetaAtom, PMetaAtom>& dict) = 0;

    virtual bool UnwrapMoveFrom(const RTTI::TDictionary<PMetaAtom, PMetaAtom>& dict) = 0;
    virtual bool UnwrapCopyFrom(const RTTI::TDictionary<PMetaAtom, PMetaAtom>& dict) = 0;

    virtual bool Equals(const RTTI::TDictionary<PMetaAtom, PMetaAtom>& dict) const = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TMetaTypedAtomImpl : public FMetaAtom {
public:
    typedef TMetaType<T> meta_type;

    TMetaTypedAtomImpl();
    virtual ~TMetaTypedAtomImpl();

    TMetaTypedAtomImpl(T&& wrapper);
    TMetaTypedAtomImpl& operator =(T&& wrapper);

    TMetaTypedAtomImpl(const T& wrapper);
    TMetaTypedAtomImpl& operator =(const T& wrapper);

    TMetaTypedAtomImpl(TMetaTypedAtomImpl&& rvalue);
    TMetaTypedAtomImpl& operator =(TMetaTypedAtomImpl&& rvalue);

    TMetaTypedAtomImpl(const TMetaTypedAtomImpl& other);
    TMetaTypedAtomImpl& operator =(const TMetaTypedAtomImpl& other);

    FORCE_INLINE T& Wrapper() { return _wrapper; }
    FORCE_INLINE const T& Wrapper() const { return _wrapper; }

    virtual FMetaTypeInfo TypeInfo() const override final;
    virtual const IMetaTypeVirtualTraits *Traits() const override final;

    virtual bool IsDefaultValue() const override final;

    virtual void MoveTo(FMetaAtom *atom) override final;
    virtual void CopyTo(FMetaAtom *atom) const override final;

    virtual FMetaAtom *WrapMoveTo() override final;
    virtual FMetaAtom *WrapCopyTo() const override final;

    virtual void MoveFrom(FMetaAtom *atom) override final;
    virtual void CopyFrom(const FMetaAtom *atom) override final;

    virtual bool Equals(const FMetaAtom *atom) const override final;
    virtual bool DeepEquals(const FMetaAtom *atom) const override final;

    virtual size_t HashValue() const override final;
    virtual FString ToString() const override final;

    void Swap(T& wrapper);

private:
    T _wrapper;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TMetaTypedAtom : public TMetaTypedAtomImpl<T> {
public:
    typedef T wrapper_type;
    typedef TMetaTypedAtomImpl< wrapper_type > impl_type;

    TMetaTypedAtom() {}
    virtual ~TMetaTypedAtom() {}

    TMetaTypedAtom(wrapper_type&& wrapper) : impl_type(std::move(wrapper)) {}
    TMetaTypedAtom& operator =(T&& wrapper) { impl_type::operator =(std::move(wrapper)); return *this; }

    TMetaTypedAtom(const wrapper_type& wrapper) : impl_type(wrapper) {}
    TMetaTypedAtom& operator =(const wrapper_type& wrapper) { impl_type::operator =(wrapper); return *this; }

    TMetaTypedAtom(TMetaTypedAtom&& rvalue) : impl_type(std::move(rvalue)) {}
    TMetaTypedAtom& operator =(TMetaTypedAtom&& rvalue) { impl_type::operator =(std::move(rvalue)); return *this; }

    TMetaTypedAtom(const TMetaTypedAtom& other) : impl_type(other) {}
    TMetaTypedAtom& operator =(const TMetaTypedAtom& other) { impl_type::operator =(other); return *this; }

    virtual IMetaAtomPair *AsPair() override final { return nullptr; }
    virtual const IMetaAtomPair *AsPair() const override final { return nullptr; }

    virtual IMetaAtomVector *AsVector() override final { return nullptr; }
    virtual const IMetaAtomVector *AsVector() const override final { return nullptr; }

    virtual IMetaAtomDictionary *AsDictionary() override final { return nullptr; }
    virtual const IMetaAtomDictionary *AsDictionary() const override final { return nullptr; }

    virtual void Accept(IMetaAtomVisitor* visitor) override final { visitor->Visit(this); }
    virtual void Accept(IMetaAtomConstVisitor* visitor) const override final { visitor->Visit(this); }

    SINGLETON_POOL_ALLOCATED_DECL();
};
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
class TMetaTypedAtom< RTTI::TPair<_First, _Second> > : public TMetaTypedAtomImpl< RTTI::TPair<_First, _Second> >, public IMetaAtomPair {
public:
    typedef RTTI::TPair<_First, _Second> wrapper_type;
    typedef TMetaTypedAtomImpl< wrapper_type > impl_type;

    TMetaTypedAtom() {}
    virtual ~TMetaTypedAtom() {}

    TMetaTypedAtom(wrapper_type&& wrapper) : impl_type(std::move(wrapper)) {}
    TMetaTypedAtom& operator =(wrapper_type&& wrapper) { impl_type::operator =(std::move(wrapper)); return *this; }

    TMetaTypedAtom(const wrapper_type& wrapper) : impl_type(wrapper) {}
    TMetaTypedAtom& operator =(const wrapper_type& wrapper) { impl_type::operator =(wrapper); return *this; }

    TMetaTypedAtom(TMetaTypedAtom&& rvalue) : impl_type(std::move(rvalue)) {}
    TMetaTypedAtom& operator =(TMetaTypedAtom&& rvalue) { impl_type::operator =(std::move(rvalue)); return *this; }

    TMetaTypedAtom(const TMetaTypedAtom& other) : impl_type(other) {}
    TMetaTypedAtom& operator =(const TMetaTypedAtom& other) { impl_type::operator =(other); return *this; }

    virtual IMetaAtomPair *AsPair() override final { return this; }
    virtual const IMetaAtomPair *AsPair() const override final { return this; }

    virtual IMetaAtomVector *AsVector() override final { return nullptr; }
    virtual const IMetaAtomVector *AsVector() const override final { return nullptr; }

    virtual IMetaAtomDictionary *AsDictionary() override final { return nullptr; }
    virtual const IMetaAtomDictionary *AsDictionary() const override final { return nullptr; }

    using impl_type::IsDefaultValue;
    using impl_type::MoveTo;
    using impl_type::CopyTo;
    using impl_type::WrapMoveTo;
    using impl_type::WrapCopyTo;
    using impl_type::MoveFrom;
    using impl_type::CopyFrom;
    using impl_type::Equals;

    virtual void Accept(IMetaAtomVisitor* visitor) override final { visitor->Visit(this); }
    virtual void Accept(IMetaAtomConstVisitor* visitor) const override final { visitor->Visit(this); }

    SINGLETON_POOL_ALLOCATED_DECL();

    // IMetaAtomPair interface

private:
    virtual const FMetaAtom* Atom() const override final { return this; }

    virtual FMetaTypeInfo FirstTypeInfo() const override final { return RTTI::TypeInfo< _First >(); }
    virtual FMetaTypeInfo SecondTypeInfo() const override final { return RTTI::TypeInfo< _Second >(); }

    virtual const IMetaTypeVirtualTraits *FirstTraits() const override final { return TMetaTypeTraits< _First >::VirtualTraits(); }
    virtual const IMetaTypeVirtualTraits *SecondTraits() const override final { return TMetaTypeTraits< _Second >::VirtualTraits(); }

    virtual void MoveTo(RTTI::TPair<PMetaAtom, PMetaAtom>& pair) override final;
    virtual void CopyTo(RTTI::TPair<PMetaAtom, PMetaAtom>& pair) const override final;

    virtual void WrapMoveTo(RTTI::TPair<PMetaAtom, PMetaAtom>& pair) override final;
    virtual void WrapCopyTo(RTTI::TPair<PMetaAtom, PMetaAtom>& pair) const override final;

    virtual void MoveFrom(const RTTI::TPair<PMetaAtom, PMetaAtom>& pair) override final;
    virtual void CopyFrom(const RTTI::TPair<PMetaAtom, PMetaAtom>& pair) override final;

    virtual bool UnwrapMoveFrom(const RTTI::TPair<PMetaAtom, PMetaAtom>& pair) override final;
    virtual bool UnwrapCopyFrom(const RTTI::TPair<PMetaAtom, PMetaAtom>& pair) override final;

    virtual bool Equals(const RTTI::TPair<PMetaAtom, PMetaAtom>& pair) const override final;
};
//----------------------------------------------------------------------------
template <typename T>
class TMetaTypedAtom< RTTI::TVector<T> > : public TMetaTypedAtomImpl< RTTI::TVector<T> >, public IMetaAtomVector {
public:
    typedef RTTI::TVector<T> wrapper_type;
    typedef TMetaTypedAtomImpl< wrapper_type > impl_type;

    TMetaTypedAtom() {}
    virtual ~TMetaTypedAtom() {}

    TMetaTypedAtom(wrapper_type&& wrapper) : impl_type(std::move(wrapper)) {}
    TMetaTypedAtom& operator =(wrapper_type&& wrapper) { impl_type::operator =(std::move(wrapper)); return *this; }

    TMetaTypedAtom(const wrapper_type& wrapper) : impl_type(wrapper) {}
    TMetaTypedAtom& operator =(const wrapper_type& wrapper) { impl_type::operator =(wrapper); return *this; }

    TMetaTypedAtom(TMetaTypedAtom&& rvalue) : impl_type(std::move(rvalue)) {}
    TMetaTypedAtom& operator =(TMetaTypedAtom&& rvalue) { impl_type::operator =(std::move(rvalue)); return *this; }

    TMetaTypedAtom(const TMetaTypedAtom& other) : impl_type(other) {}
    TMetaTypedAtom& operator =(const TMetaTypedAtom& other) { impl_type::operator =(other); return *this; }

    virtual IMetaAtomPair *AsPair() override final { return nullptr; }
    virtual const IMetaAtomPair *AsPair() const override final { return nullptr; }

    virtual IMetaAtomVector *AsVector() override final { return this; }
    virtual const IMetaAtomVector *AsVector() const override final { return this; }

    virtual IMetaAtomDictionary *AsDictionary() override final { return nullptr; }
    virtual const IMetaAtomDictionary *AsDictionary() const override final { return nullptr; }

    using impl_type::IsDefaultValue;
    using impl_type::MoveTo;
    using impl_type::CopyTo;
    using impl_type::WrapMoveTo;
    using impl_type::WrapCopyTo;
    using impl_type::MoveFrom;
    using impl_type::CopyFrom;
    using impl_type::Equals;

    virtual void Accept(IMetaAtomVisitor* visitor) override final { visitor->Visit(this); }
    virtual void Accept(IMetaAtomConstVisitor* visitor) const override final { visitor->Visit(this); }

    SINGLETON_POOL_ALLOCATED_DECL();

    // IMetaAtomVector interface

private:
    virtual const FMetaAtom* Atom() const override final { return this; }

    virtual FMetaTypeInfo ValueTypeInfo() const override final { return RTTI::TypeInfo< T >(); }
    virtual const IMetaTypeVirtualTraits *ValueTraits() const override final { return TMetaTypeTraits< T >::VirtualTraits(); }

    virtual void MoveTo(RTTI::TVector<PMetaAtom>& vector) override final;
    virtual void CopyTo(RTTI::TVector<PMetaAtom>& vector) const override final;

    virtual void WrapMoveTo(RTTI::TVector<PMetaAtom>& vector) override final;
    virtual void WrapCopyTo(RTTI::TVector<PMetaAtom>& vector) const override final;

    virtual void MoveFrom(const RTTI::TVector<PMetaAtom>& vector) override final;
    virtual void CopyFrom(const RTTI::TVector<PMetaAtom>& vector) override final;

    virtual bool UnwrapMoveFrom(const RTTI::TVector<PMetaAtom>& vector) override final;
    virtual bool UnwrapCopyFrom(const RTTI::TVector<PMetaAtom>& vector) override final;

    virtual bool Equals(const RTTI::TVector<PMetaAtom>& vector) const override final;
};
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
class TMetaTypedAtom< RTTI::TDictionary<_Key, _Value> > : public TMetaTypedAtomImpl< RTTI::TDictionary<_Key, _Value> >, public IMetaAtomDictionary {
public:
    typedef RTTI::TDictionary<_Key, _Value> wrapper_type;
    typedef TMetaTypedAtomImpl< wrapper_type > impl_type;

    TMetaTypedAtom() {}
    virtual ~TMetaTypedAtom() {}

    TMetaTypedAtom(wrapper_type&& wrapper) : impl_type(std::move(wrapper)) {}
    TMetaTypedAtom& operator =(wrapper_type&& wrapper) { impl_type::operator =(std::move(wrapper)); return *this; }

    TMetaTypedAtom(const wrapper_type& wrapper) : impl_type(wrapper) {}
    TMetaTypedAtom& operator =(const wrapper_type& wrapper) { impl_type::operator =(wrapper); return *this; }

    TMetaTypedAtom(TMetaTypedAtom&& rvalue) : impl_type(std::move(rvalue)) {}
    TMetaTypedAtom& operator =(TMetaTypedAtom&& rvalue) { impl_type::operator =(std::move(rvalue)); return *this; }

    TMetaTypedAtom(const TMetaTypedAtom& other) : impl_type(other) {}
    TMetaTypedAtom& operator =(const TMetaTypedAtom& other) { impl_type::operator =(other); return *this; }

    virtual IMetaAtomPair *AsPair() override final { return nullptr; }
    virtual const IMetaAtomPair *AsPair() const override final { return nullptr; }

    virtual IMetaAtomVector *AsVector() override final { return nullptr; }
    virtual const IMetaAtomVector *AsVector() const override final { return nullptr; }

    virtual IMetaAtomDictionary *AsDictionary() override final { return this; }
    virtual const IMetaAtomDictionary *AsDictionary() const override final { return this; }

    using impl_type::IsDefaultValue;
    using impl_type::MoveTo;
    using impl_type::CopyTo;
    using impl_type::WrapMoveTo;
    using impl_type::WrapCopyTo;
    using impl_type::MoveFrom;
    using impl_type::CopyFrom;
    using impl_type::Equals;

    virtual void Accept(IMetaAtomVisitor* visitor) override final { visitor->Visit(this); }
    virtual void Accept(IMetaAtomConstVisitor* visitor) const override final { visitor->Visit(this); }

    SINGLETON_POOL_ALLOCATED_DECL();

    // IMetaAtomDictionary interface

private:
    virtual const FMetaAtom* Atom() const override final { return this; }

    virtual FMetaTypeInfo KeyTypeInfo() const override final { return RTTI::TypeInfo< _Key >(); }
    virtual FMetaTypeInfo ValueTypeInfo() const override final { return RTTI::TypeInfo< _Value >(); }

    virtual const IMetaTypeVirtualTraits *KeyTraits() const override final { return TMetaTypeTraits< _Key >::VirtualTraits(); }
    virtual const IMetaTypeVirtualTraits *ValueTraits() const override final { return TMetaTypeTraits< _Value >::VirtualTraits(); }

    virtual void MoveTo(RTTI::TDictionary<PMetaAtom, PMetaAtom>& dict) override final;
    virtual void CopyTo(RTTI::TDictionary<PMetaAtom, PMetaAtom>& dict) const override final;

    virtual void WrapMoveTo(RTTI::TDictionary<PMetaAtom, PMetaAtom>& dict) override final;
    virtual void WrapCopyTo(RTTI::TDictionary<PMetaAtom, PMetaAtom>& dict) const override final;

    virtual void MoveFrom(const RTTI::TDictionary<PMetaAtom, PMetaAtom>& dict) override final;
    virtual void CopyFrom(const RTTI::TDictionary<PMetaAtom, PMetaAtom>& dict) override final;

    virtual bool UnwrapMoveFrom(const RTTI::TDictionary<PMetaAtom, PMetaAtom>& dict) override final;
    virtual bool UnwrapCopyFrom(const RTTI::TDictionary<PMetaAtom, PMetaAtom>& dict) override final;

    virtual bool Equals(const RTTI::TDictionary<PMetaAtom, PMetaAtom>& dict) const override final;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct TMetaAtomWrapper {
    typedef TMetaTypeTraits< Meta::TDecay<T> > trais_type;
    typedef typename std::is_same<
        typename trais_type::wrapper_type,
        typename trais_type::wrapped_type
    >::type dont_need_wrapper;
    typedef std::integral_constant<bool, not dont_need_wrapper::value> need_wrapper;
    typedef TMetaTypedAtom< typename trais_type::wrapper_type > type;
};
//----------------------------------------------------------------------------
/*inline FMetaAtom* MakeAtom(FMetaAtom* atom) { return atom; }
inline const FMetaAtom* MakeAtom(const FMetaAtom* atom) { return atom; }
inline FMetaAtom* MakeAtom(PMetaAtom&& ratom) { return ratom.get(); }
inline FMetaAtom* MakeAtom(const PMetaAtom& atom) { return atom.get(); }
inline const FMetaAtom* MakeAtom(const PCMetaAtom& atom) { return atom.get(); }*/
//----------------------------------------------------------------------------
template <typename T>
typename TMetaAtomWrapper<T>::type *MakeAtom(T&& rvalue, typename std::enable_if< TMetaAtomWrapper<T>::need_wrapper::value >::type* = 0 );
//----------------------------------------------------------------------------
template <typename T>
typename TMetaAtomWrapper<T>::type *MakeAtom(const T& value, typename std::enable_if< TMetaAtomWrapper<T>::need_wrapper::value >::type* = 0 );
//----------------------------------------------------------------------------
template <typename T>
typename TMetaAtomWrapper<T>::type *MakeAtom(T&& rvalue, typename std::enable_if< TMetaAtomWrapper<T>::dont_need_wrapper::value >::type* = 0 );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const RTTI::TMetaTypedAtom<T>& typedAtom) {
    return oss << typedAtom.Wrapper();
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const RTTI::FMetaAtom *atom) {
    Assert(atom);
    return oss << atom->ToString();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core.RTTI/MetaAtom-inl.h"
