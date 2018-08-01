#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core.RTTI/TypeInfos.h"

#include "Core/IO/String_fwd.h"
#include "Core/IO/TextWriter_fwd.h"
#include "Core/Memory/InSituPtr.h"
#include "Core/Misc/Function.h"
#include "Core/Meta/TypeTraits.h"

namespace Core {
class FLinearHeap;
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FAtom;
class IAtomVisitor;
class FMetaClass;
//----------------------------------------------------------------------------
class ITypeTraits;
class IScalarTraits;
class ITupleTraits;
class IListTraits;
class IDicoTraits;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using PTypeTraits = TInSituPtr<ITypeTraits>;
//----------------------------------------------------------------------------
class ITypeTraits {
public:
    virtual ~ITypeTraits() {}

    virtual void Construct(void* data) const = 0;
    virtual void ConstructCopy(void* data, const void* other) const = 0;
    virtual void ConstructMove(void* data, void* rvalue) const = 0;
    virtual void ConstructMoveDestroy(void* data, void* rvalue) const = 0;
    virtual void ConstructSwap(void* data, void* other) const = 0;
    virtual void Destroy(void* data) const = 0;

    virtual FTypeId TypeId() const = 0;
    virtual ETypeFlags TypeFlags() const = 0;
    virtual FTypeInfos TypeInfos() const = 0;
    virtual size_t SizeInBytes() const = 0;
    virtual FSizeAndFlags SizeAndFlags() const = 0;

    virtual bool IsDefaultValue(const void* data) const = 0;
    virtual void ResetToDefaultValue(void* data) const = 0;

    virtual bool Equals(const void* lhs, const void* rhs) const = 0;
    virtual void Copy(const void* src, void* dst) const = 0;
    virtual void Move(void* src, void* dst) const = 0;

    virtual void Swap(void* lhs, void* rhs) const = 0;

    virtual bool DeepEquals(const void* lhs, const void* rhs) const = 0;
    virtual void DeepCopy(const void* src, void* dst) const = 0;

    virtual bool PromoteCopy(const void* src, const FAtom& dst) const = 0;
    virtual bool PromoteMove(void* src, const FAtom& dst) const = 0;

    virtual void* Cast(void* data, const PTypeTraits& dst) const = 0;

    virtual hash_t HashValue(const void* data) const = 0;

    virtual bool Accept(IAtomVisitor* visitor, void* data) const = 0;

    virtual const IScalarTraits* AsScalar() const = 0;
    virtual const ITupleTraits* AsTuple() const = 0;
    virtual const IListTraits* AsList() const = 0;
    virtual const IDicoTraits* AsDico() const = 0;

public: // non-virtual helpers
    const IScalarTraits& ToScalar() const { Assert(AsScalar()); return (*checked_cast<const IScalarTraits*>(this)); }
    const ITupleTraits& ToTuple() const { Assert(AsTuple()); return (*checked_cast<const ITupleTraits*>(this)); }
    const IListTraits& ToList() const { Assert(AsList()); return (*checked_cast<const IListTraits*>(this)); }
    const IDicoTraits& ToDico() const { Assert(AsDico()); return (*checked_cast<const IDicoTraits*>(this)); }

    inline friend bool operator ==(const ITypeTraits& lhs, const ITypeTraits& rhs) { return (lhs.VTable() == rhs.VTable()); }
    inline friend bool operator !=(const ITypeTraits& lhs, const ITypeTraits& rhs) { return (lhs.VTable() != rhs.VTable()); }

protected:
    // only used internally to compare traits together
    intptr_t VTable() const {
        STATIC_ASSERT(sizeof(*this) == sizeof(intptr_t));
        return (*(const intptr_t*)this);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline PTypeTraits Traits(Meta::TType<void>) NOEXCEPT { return PTypeTraits(); }
//----------------------------------------------------------------------------
template <typename T>
PTypeTraits MakeTraits() NOEXCEPT; // defined in NativeTypes.h
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_RTTI_API FTypeId MakeTupleTypeId(const TMemoryView<const PTypeTraits>& elements);
CORE_RTTI_API ETypeFlags MakeTupleTypeFlags(const TMemoryView<const PTypeTraits>& elements);
CORE_RTTI_API FString MakeTupleTypeName(const TMemoryView<const PTypeTraits>& elements);
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
    virtual const IScalarTraits* AsScalar() const override final { return this; }
    virtual const ITupleTraits* AsTuple() const override final { return nullptr; }
    virtual const IListTraits* AsList() const override final { return nullptr; }
    virtual const IDicoTraits* AsDico() const override final { return nullptr; }

public:
    virtual const FMetaClass* ObjectClass() const = 0;
};
//----------------------------------------------------------------------------
class ITupleTraits : public ITypeTraits {
public: // ITypeTraits
    virtual bool Accept(IAtomVisitor* visitor, void* data) const override final;

public:
    virtual size_t Arity() const = 0;
    virtual TMemoryView<const PTypeTraits> TupleTraits() const = 0;

    virtual FAtom At(void* data, size_t index) const = 0;

    typedef TFunction<bool(const FAtom&)> foreach_fun;
    virtual bool ForEach(void* data, const foreach_fun& foreach) const = 0;

private:
    virtual const IScalarTraits* AsScalar() const override final { return nullptr; }
    virtual const ITupleTraits* AsTuple() const override final { return this; }
    virtual const IListTraits* AsList() const override final { return nullptr; }
    virtual const IDicoTraits* AsDico() const override final { return nullptr; }
};
//----------------------------------------------------------------------------
class IListTraits : public ITypeTraits {
public: // ITypeTraits
    virtual bool Accept(IAtomVisitor* visitor, void* data) const override final;

public: // IListTraits
    virtual PTypeTraits ValueTraits() const = 0;

    virtual size_t Count(const void* data) const = 0;
    virtual bool IsEmpty(const void* data) const = 0;

    virtual FAtom At(void* data, size_t index) const = 0;
    virtual size_t Find(const void* data, const FAtom& item) const = 0;

    virtual FAtom AddDefault(void* data) const = 0;
    virtual void AddCopy(void* data, const FAtom& item) const = 0;
    virtual void AddMove(void* data, const FAtom& item) const = 0;
    virtual void Erase(void* data, size_t index) const = 0;
    virtual bool Remove(void* data, const FAtom& item) const = 0;

    virtual void Reserve(void* data, size_t capacity) const = 0;
    virtual void Clear(void* data) const = 0;
    virtual void Empty(void* data, size_t capacity) const = 0;

    typedef TFunction<bool(const FAtom&)> foreach_fun;
    virtual bool ForEach(void* data, const foreach_fun& foreach) const = 0;

private:
    virtual const IScalarTraits* AsScalar() const override final { return nullptr; }
    virtual const ITupleTraits* AsTuple() const override final { return nullptr; }
    virtual const IListTraits* AsList() const override final { return (this); }
    virtual const IDicoTraits* AsDico() const override final { return nullptr; }
};
//----------------------------------------------------------------------------
class IDicoTraits : public ITypeTraits {
public: // ITypeTraits
    virtual bool Accept(IAtomVisitor* visitor, void* data) const override final;

public: // IDicoTraits
    virtual PTypeTraits KeyTraits() const = 0;
    virtual PTypeTraits ValueTraits() const = 0;

    virtual size_t Count(const void* data) const = 0;
    virtual bool IsEmpty(const void* data) const = 0;

    virtual FAtom Find(const void* data, const FAtom& key) const = 0;

    virtual FAtom AddDefaultCopy(void* data, const FAtom& key) const = 0;
    virtual FAtom AddDefaultMove(void* data, const FAtom& key) const = 0;

    virtual void AddCopy(void* data, const FAtom& key, const FAtom& value) const = 0;
    virtual void AddMove(void* data, const FAtom& key, const FAtom& value) const = 0;
    virtual bool Remove(void* data, const FAtom& key) const = 0;

    virtual void Reserve(void* data, size_t capacity) const = 0;
    virtual void Clear(void* data) const = 0;
    virtual void Empty(void* data, size_t capacity) const = 0;

    typedef TFunction<bool(const FAtom&, const FAtom&)> foreach_fun;
    virtual bool ForEach(void* data, const foreach_fun& foreach) const = 0;

private:
    virtual const IScalarTraits* AsScalar() const override final { return nullptr; }
    virtual const ITupleTraits* AsTuple() const override final { return nullptr; }
    virtual const IListTraits* AsList() const override final { return nullptr; }
    virtual const IDicoTraits* AsDico() const override final { return (this); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
