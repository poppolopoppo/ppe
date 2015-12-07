#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/MetaType.h"

#include "Core/Meta/OneTimeInitialize.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MetaAtom;
class MetaObject;
class MetaProperty;
template <typename T>
struct MetaTypeTraits;
//----------------------------------------------------------------------------
class IMetaTypeVirtualTraits {
public:
    virtual ~IMetaTypeVirtualTraits() {}

    virtual MetaAtom *CreateDefaultValue() const = 0;

    virtual bool AssignMove(MetaAtom *dst, MetaAtom *src) const = 0;
    virtual bool AssignCopy(MetaAtom *dst, const MetaAtom *src) const = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class AbstractMetaTypeScalarTraits : public IMetaTypeVirtualTraits {
public:
    virtual bool AssignMove(MetaAtom *dst, MetaAtom *src) const override;
    virtual bool AssignCopy(MetaAtom *dst, const MetaAtom *src) const override;
};
//----------------------------------------------------------------------------
template <typename T>
class MetaTypeScalarTraits : public AbstractMetaTypeScalarTraits {
public:
    STATIC_ASSERT(false == MetaTypeTraits< T >::Wrapping);

    virtual MetaAtom* CreateDefaultValue() const override {
        MetaAtom* const result = new MetaWrappedAtom< T >(
            std::move(MetaTypeTraits<T>::meta_type::DefaultValue())
        );
        Assert(result->Traits() == this);
        Assert(result->IsDefaultValue());
        return result;
    }

    static const MetaTypeScalarTraits* Instance() {
        ONE_TIME_INITIALIZE_TPL(const MetaTypeScalarTraits, gInstance);
        return &gInstance;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class AbstractMetaTypePairTraits : public IMetaTypeVirtualTraits {
public:
    virtual bool AssignMove(MetaAtom *dst, MetaAtom *src) const override;
    virtual bool AssignCopy(MetaAtom *dst, const MetaAtom *src) const override;
};
//----------------------------------------------------------------------------
template <typename _First, typename _Second>
class MetaTypePairTraits : public AbstractMetaTypePairTraits {
public:
    STATIC_ASSERT(false == MetaTypeTraits< _First >::Wrapping);
    STATIC_ASSERT(false == MetaTypeTraits< _Second >::Wrapping);

    virtual MetaAtom* CreateDefaultValue() const override {
        MetaAtom* const result = new MetaWrappedAtom< RTTI::Pair<_First, _Second> >();
        Assert(result->Traits() == this);
        Assert(result->IsDefaultValue());
        return result;
    }

    static const MetaTypePairTraits* Instance() {
        ONE_TIME_INITIALIZE_TPL(const MetaTypePairTraits, gInstance);
        return &gInstance;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class AbstractMetaTypeVectorTraits : public IMetaTypeVirtualTraits {
public:
    virtual bool AssignMove(MetaAtom *dst, MetaAtom *src) const override;
    virtual bool AssignCopy(MetaAtom *dst, const MetaAtom *src) const override;
};
//----------------------------------------------------------------------------
template <typename T>
class MetaTypeVectorTraits : public AbstractMetaTypeVectorTraits {
public:
    STATIC_ASSERT(false == MetaTypeTraits< T >::Wrapping);

    virtual MetaAtom* CreateDefaultValue() const override {
        MetaAtom* const result = new MetaWrappedAtom< RTTI::Vector<T> >();
        Assert(result->Traits() == this);
        Assert(result->IsDefaultValue());
        return result;
    }

    static const MetaTypeVectorTraits* Instance() {
        ONE_TIME_INITIALIZE_TPL(const MetaTypeVectorTraits, gInstance);
        return &gInstance;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class AbstractMetaTypeDictionaryTraits : public IMetaTypeVirtualTraits {
public:
    virtual bool AssignMove(MetaAtom *dst, MetaAtom *src) const override;
    virtual bool AssignCopy(MetaAtom *dst, const MetaAtom *src) const override;
};
//----------------------------------------------------------------------------
template <typename _Key, typename _Value>
class MetaTypeDictionaryTraits : public AbstractMetaTypeDictionaryTraits {
public:
    STATIC_ASSERT(false == MetaTypeTraits< _Key >::Wrapping);
    STATIC_ASSERT(false == MetaTypeTraits< _Value >::Wrapping);

    virtual MetaAtom* CreateDefaultValue() const override {
        MetaAtom* const result = new MetaWrappedAtom< RTTI::Dictionary<_Key, _Value> >();
        Assert(result->Traits() == this);
        Assert(result->IsDefaultValue());
        return result;
    }

    static const MetaTypeDictionaryTraits* Instance() {
        ONE_TIME_INITIALIZE_TPL(const MetaTypeDictionaryTraits, gInstance);
        return &gInstance;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool AssignMove(MetaAtom *dst, MetaAtom *src);
bool AssignCopy(MetaAtom *dst, const MetaAtom *src);
//----------------------------------------------------------------------------
template <typename T>
bool AssignMove(T *dst, MetaAtom *src);
template <typename T>
bool AssignCopy(T *dst, const MetaAtom *src);
//----------------------------------------------------------------------------
template <typename T>
bool AssignMove(MetaAtom *dst, T *src);
template <typename T>
bool AssignCopy(MetaAtom *dst, const T *src);
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
