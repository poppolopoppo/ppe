#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/TypeInfos.h"

#include "Core/IO/String_fwd.h"
#include "Core/Meta/Function.h"
#include "Core/Meta/InSituPtr.h"
#include "Core/Meta/TypeTraits.h"

#include <iosfwd>

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FAtom;
class IAtomVisitor;
//----------------------------------------------------------------------------
class ITypeTraits;
class IPairTraits;
class IListTraits;
class IDicoTraits;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ITypeTraits {
public:
    virtual ~ITypeTraits() {}

    virtual void Create(const FAtom& atom) const = 0;
    virtual void Destroy(const FAtom& atom) const = 0;

    virtual FTypeId TypeId() const = 0;
    virtual FTypeInfos TypeInfos() const = 0;
    virtual ETypeFlags TypeFlags() const = 0;
    virtual size_t SizeInBytes() const = 0;

    virtual bool IsDefaultValue(const FAtom& value) const = 0;

    virtual bool Equals(const FAtom& lhs, const FAtom& rhs) const = 0;
    virtual void Copy(const FAtom& src, const FAtom& dst) const = 0;
    virtual void Move(const FAtom& src, const FAtom& dst) const = 0;

    virtual void Swap(const FAtom& lhs, const FAtom& rhs) const = 0;

    virtual bool DeepEquals(const FAtom& lhs, const FAtom& rhs) const = 0;
    virtual void DeepCopy(const FAtom& src, const FAtom& dst) const = 0;

    virtual bool PromoteCopy(const FAtom& from, const FAtom& to) const = 0;
    virtual bool PromoteMove(const FAtom& from, const FAtom& to) const = 0;

    virtual hash_t HashValue(const FAtom& atom) const = 0;

    virtual void Format(std::basic_ostream<char>& oss, const FAtom& atom) const = 0;
    virtual void Format(std::basic_ostream<wchar_t>& oss, const FAtom& atom) const = 0;

    virtual bool Accept(IAtomVisitor* visitor, const FAtom& atom) const = 0;

    virtual const IPairTraits* AsPair() const = 0;
    virtual const IListTraits* AsList() const = 0;
    virtual const IDicoTraits* AsDico() const = 0;
};
//----------------------------------------------------------------------------
using PTypeTraits = Meta::TInSituPtr<ITypeTraits>;
//----------------------------------------------------------------------------
inline PTypeTraits Traits(Meta::TType<void>) { return PTypeTraits(); }
//----------------------------------------------------------------------------
template <typename T>
PTypeTraits MakeTraits() {
    return Traits(Meta::TType< Meta::TDecay<T> >{});
}
//----------------------------------------------------------------------------
CORE_RTTI_API FTypeId MakePairTypeId(const PTypeTraits& first, const PTypeTraits& second);
CORE_RTTI_API FString MakePairTypeName(const PTypeTraits& first, const PTypeTraits& second);
//----------------------------------------------------------------------------
CORE_RTTI_API FTypeId MakeListTypeId(const PTypeTraits& value);
CORE_RTTI_API FString MakeListTypeName(const PTypeTraits& value);
//----------------------------------------------------------------------------
CORE_RTTI_API FTypeId MakeDicoTypeId(const PTypeTraits& key, const PTypeTraits& value);
CORE_RTTI_API FString MakeDicoTypeName(const PTypeTraits& key, const PTypeTraits& value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IScalarTraits : public ITypeTraits {
public: // ITypeTraits
    virtual ETypeFlags TypeFlags() const override final { return ETypeFlags::Pair; }

private:
    virtual const IPairTraits* AsPair() const override final { return nullptr; }
    virtual const IListTraits* AsList() const override final { return nullptr; }
    virtual const IDicoTraits* AsDico() const override final { return nullptr; }
};
//----------------------------------------------------------------------------
class IPairTraits : public ITypeTraits {
public: // ITypeTraits
    virtual ETypeFlags TypeFlags() const override final { return ETypeFlags::Pair; }

    virtual bool Accept(IAtomVisitor* visitor, const FAtom& atom) const override final;

public:
    virtual PTypeTraits FirstTraits() const = 0;
    virtual PTypeTraits SecondTraits() const = 0;

    virtual FAtom First(const FAtom& pair) const = 0;
    virtual FAtom Second(const FAtom& pair) const = 0;

    virtual void SetFirstCopy(const FAtom& pair, const FAtom& first) const = 0;
    virtual void SetFirstMove(const FAtom& pair, const FAtom& first) const = 0;

    virtual void SetSecondCopy(const FAtom& pair, const FAtom& second) const = 0;
    virtual void SetSecondMove(const FAtom& pair, const FAtom& second) const = 0;

private:
    virtual const IPairTraits* AsPair() const override final { return this; }
    virtual const IListTraits* AsList() const override final { return nullptr; }
    virtual const IDicoTraits* AsDico() const override final { return nullptr; }
};
//----------------------------------------------------------------------------
class IListTraits : public ITypeTraits {
public: // ITypeTraits
    virtual ETypeFlags TypeFlags() const override final { return ETypeFlags::List; }

    virtual bool Accept(IAtomVisitor* visitor, const FAtom& atom) const override final;

public: // IListTraits
    virtual PTypeTraits ValueTraits() const = 0;

    virtual size_t Count(const FAtom& list) const = 0;
    virtual bool Empty(const FAtom& list) const = 0;

    virtual FAtom At(const FAtom& list, size_t index) const = 0;
    virtual size_t Find(const FAtom& list, const FAtom& item) const = 0;

    virtual FAtom AddDefault(const FAtom& list) const = 0;
    virtual void AddCopy(const FAtom& list, const FAtom& item) const = 0;
    virtual void AddMove(const FAtom& list, const FAtom& item) const = 0;
    virtual void Erase(const FAtom& list, size_t index) const = 0;
    virtual bool Remove(const FAtom& list, const FAtom& item) const = 0;

    virtual void Reserve(const FAtom& list, size_t capacity) const = 0;
    virtual void Clear(const FAtom& list) const = 0;

    typedef Meta::TFunction<bool(const FAtom&)> foreach_fun;
    virtual bool ForEach(const FAtom& list, const foreach_fun& foreach) const = 0;

private:
    virtual const IPairTraits* AsPair() const override final { return nullptr; }
    virtual const IListTraits* AsList() const override final { return this; }
    virtual const IDicoTraits* AsDico() const override final { return nullptr; }
};
//----------------------------------------------------------------------------
class IDicoTraits : public ITypeTraits {
public: // ITypeTraits
    virtual ETypeFlags TypeFlags() const override final { return ETypeFlags::Dico; }

    virtual bool Accept(IAtomVisitor* visitor, const FAtom& atom) const override final;

public: // IDicoTraits
    virtual PTypeTraits KeyTraits() const = 0;
    virtual PTypeTraits ValueTraits() const = 0;

    virtual size_t Count(const FAtom& dico) const = 0;
    virtual bool Empty(const FAtom& dico) const = 0;

    virtual FAtom Find(const FAtom& dico, const FAtom& key) const = 0;

    virtual FAtom AddDefault(const FAtom& dico, const FAtom& key) const = 0;
    virtual void AddCopy(const FAtom& dico, const FAtom& key, const FAtom& value) const = 0;
    virtual void AddMove(const FAtom& dico, const FAtom& key, const FAtom& value) const = 0;
    virtual bool Remove(const FAtom& dico, const FAtom& key) const = 0;

    virtual void Reserve(const FAtom& dico, size_t capacity) const = 0;
    virtual void Clear(const FAtom& dico) const = 0;

    typedef Meta::TFunction<bool(const FAtom&, const FAtom&)> foreach_fun;
    virtual bool ForEach(const FAtom& dico, const foreach_fun& foreach) const = 0;

private:
    virtual const IPairTraits* AsPair() const override final { return nullptr; }
    virtual const IListTraits* AsList() const override final { return nullptr; }
    virtual const IDicoTraits* AsDico() const override final { return this; }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core