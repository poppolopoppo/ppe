#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core/Meta/OneTimeInitialize.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMetaAtom;
class FMetaObject;
class FMetaProperty;
template <typename T>
struct TMetaTypeTraits;
//----------------------------------------------------------------------------
class IMetaTypeVirtualTraits {
public:
    virtual ~IMetaTypeVirtualTraits() {}

    virtual FMetaAtom *CreateDefaultValue() const = 0;

    virtual bool AssignMove(FMetaAtom *dst, FMetaAtom *src) const = 0;
    virtual bool AssignCopy(FMetaAtom *dst, const FMetaAtom *src) const = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FAbstractMetaTypeScalarTraits : public IMetaTypeVirtualTraits {
public:
    virtual bool AssignMove(FMetaAtom *dst, FMetaAtom *src) const override final;
    virtual bool AssignCopy(FMetaAtom *dst, const FMetaAtom *src) const override final;
};
//----------------------------------------------------------------------------
template <typename T>
class TMetaTypeScalarTraits : public FAbstractMetaTypeScalarTraits {
public:
    STATIC_ASSERT(false == TMetaTypeTraits< T >::Wrapping);

    virtual FMetaAtom* CreateDefaultValue() const override final {
        typedef typename TMetaAtomWrapper<T>::type atom_type;
        FMetaAtom* const result = MakeAtom(std::move(TMetaTypeTraits<T>::meta_type::DefaultValue()));
        Assert(result->IsDefaultValue());
        return result;
    }

    static const TMetaTypeScalarTraits* Instance() {
        ONE_TIME_INITIALIZE_TPL(const TMetaTypeScalarTraits, GInstance);
        return &GInstance;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FAbstractMetaTypePairTraits : public IMetaTypeVirtualTraits {
public:
    virtual bool AssignMove(FMetaAtom *dst, FMetaAtom *src) const override final;
    virtual bool AssignCopy(FMetaAtom *dst, const FMetaAtom *src) const override final;
};
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
class TMetaTypePairTraits : public FAbstractMetaTypePairTraits {
public:
    STATIC_ASSERT(false == TMetaTypeTraits< _First >::Wrapping);
    STATIC_ASSERT(false == TMetaTypeTraits< _Second >::Wrapping);

    virtual FMetaAtom* CreateDefaultValue() const override final {
        typedef typename TMetaAtomWrapper< RTTI::TPair<_First, _Second> >::type atom_type;
        FMetaAtom* const result = new atom_type();
        Assert(result->IsDefaultValue());
        return result;
    }

    static const TMetaTypePairTraits* Instance() {
        ONE_TIME_INITIALIZE_TPL(const TMetaTypePairTraits, GInstance);
        return &GInstance;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FAbstractMetaTypeVectorTraits : public IMetaTypeVirtualTraits {
public:
    virtual bool AssignMove(FMetaAtom *dst, FMetaAtom *src) const override final;
    virtual bool AssignCopy(FMetaAtom *dst, const FMetaAtom *src) const override final;
};
//----------------------------------------------------------------------------
template <typename T>
class TMetaTypeVectorTraits : public FAbstractMetaTypeVectorTraits {
public:
    STATIC_ASSERT(false == TMetaTypeTraits< T >::Wrapping);

    virtual FMetaAtom* CreateDefaultValue() const override final {
        typedef typename TMetaAtomWrapper< RTTI::TVector<T> >::type atom_type;
        FMetaAtom* const result = new atom_type();
        Assert(result->IsDefaultValue());
        return result;
    }

    static const TMetaTypeVectorTraits* Instance() {
        ONE_TIME_INITIALIZE_TPL(const TMetaTypeVectorTraits, GInstance);
        return &GInstance;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FAbstractMetaTypeDictionaryTraits : public IMetaTypeVirtualTraits {
public:
    virtual bool AssignMove(FMetaAtom *dst, FMetaAtom *src) const override final;
    virtual bool AssignCopy(FMetaAtom *dst, const FMetaAtom *src) const override final;
};
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
class TMetaTypeDictionaryTraits : public FAbstractMetaTypeDictionaryTraits {
public:
    STATIC_ASSERT(false == TMetaTypeTraits< _Key >::Wrapping);
    STATIC_ASSERT(false == TMetaTypeTraits< _Value >::Wrapping);

    virtual FMetaAtom* CreateDefaultValue() const override final {
        typedef typename TMetaAtomWrapper< RTTI::TDictionary<_Key, _Value> >::type atom_type;
        FMetaAtom* const result = new atom_type();
        Assert(result->IsDefaultValue());
        return result;
    }

    static const TMetaTypeDictionaryTraits* Instance() {
        ONE_TIME_INITIALIZE_TPL(const TMetaTypeDictionaryTraits, GInstance);
        return &GInstance;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool AssignMove(FMetaAtom *dst, FMetaAtom *src);
bool AssignCopy(FMetaAtom *dst, const FMetaAtom *src);
//----------------------------------------------------------------------------
template <typename T>
bool AssignMove(T *dst, FMetaAtom *src);
template <typename T>
bool AssignCopy(T *dst, const FMetaAtom *src);
//----------------------------------------------------------------------------
template <typename T>
bool AssignMove(FMetaAtom *dst, T *src);
template <typename T>
bool AssignCopy(FMetaAtom *dst, const T *src);
//----------------------------------------------------------------------------
template <typename T>
bool AssignMove(T *dst, T *src);
template <typename T>
bool AssignCopy(T *dst, const T *src);
//----------------------------------------------------------------------------
template <typename _Dst, typename _Src>
bool AssignMove(_Dst *dst, _Src *src);
template <typename _Dst, typename _Src>
bool AssignCopy(_Dst *dst, const _Src *src);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

#include "Core.RTTI/MetaTypeVirtualTraits-inl.h"
