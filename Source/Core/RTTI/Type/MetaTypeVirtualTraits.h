#pragma once

#include "Core/Core.h"

#include "Core/RTTI/Type/MetaType.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MetaAtom;
class MetaObject;
class MetaProperty;
//----------------------------------------------------------------------------
class IMetaTypeVirtualTraits {
public:
    virtual ~IMetaTypeVirtualTraits() {}

    virtual bool AssignMove(MetaAtom *dst, MetaAtom *src) const = 0;
    virtual bool AssignCopy(MetaAtom *dst, const MetaAtom *src) const = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MetaTypeScalarTraits : public IMetaTypeVirtualTraits {
public:
    MetaTypeScalarTraits() {}
    virtual ~MetaTypeScalarTraits() {}

    static const MetaTypeScalarTraits *Instance();

    virtual bool AssignMove(MetaAtom *dst, MetaAtom *src) const override;
    virtual bool AssignCopy(MetaAtom *dst, const MetaAtom *src) const override;
};
//----------------------------------------------------------------------------
class MetaTypePairTraits : public IMetaTypeVirtualTraits {
public:
    MetaTypePairTraits() {}
    virtual ~MetaTypePairTraits() {}

    static const MetaTypePairTraits *Instance();

    virtual bool AssignMove(MetaAtom *dst, MetaAtom *src) const override;
    virtual bool AssignCopy(MetaAtom *dst, const MetaAtom *src) const override;
};
//----------------------------------------------------------------------------
class MetaTypeVectorTraits : public IMetaTypeVirtualTraits {
public:
    MetaTypeVectorTraits() {}
    virtual ~MetaTypeVectorTraits() {}

    static const MetaTypeVectorTraits *Instance();

    virtual bool AssignMove(MetaAtom *dst, MetaAtom *src) const override;
    virtual bool AssignCopy(MetaAtom *dst, const MetaAtom *src) const override;
};
//----------------------------------------------------------------------------
class MetaTypeDictionaryTraits : public IMetaTypeVirtualTraits {
public:
    MetaTypeDictionaryTraits() {}
    virtual ~MetaTypeDictionaryTraits() {}

    static const MetaTypeDictionaryTraits *Instance();

    virtual bool AssignMove(MetaAtom *dst, MetaAtom *src) const override;
    virtual bool AssignCopy(MetaAtom *dst, const MetaAtom *src) const override;
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

#include "Core/RTTI/Type/MetaTypeVirtualTraits-inl.h"
