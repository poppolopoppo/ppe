#pragma once

#include "RTTI.h"

#include "RTTI/TypeInfos.h"

#include "HAL/PlatformMemory.h"
#include "IO/String_fwd.h"
#include "IO/TextWriter_fwd.h"
#include "Misc/Function_fwd.h"
#include "Memory/InSituPtr.h"
#include "Meta/TypeTraits.h"

namespace PPE {
class FLinearHeap;
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FAtom;
class FMetaClass;
//----------------------------------------------------------------------------
class ITypeTraits;
class IScalarTraits;
class ITupleTraits;
class IListTraits;
class IDicoTraits;
//----------------------------------------------------------------------------
class IAtomVisitor;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PTypeTraits;
//----------------------------------------------------------------------------
class PPE_RTTI_API ITypeTraits {
public:
    // !! NON VIRTUAL DTOR !!
    // There's nothing to destroy, and it disables constexpr semantics
    /*
    virtual ~ITypeTraits() = default;
    */

    virtual void Construct(void* data) const = 0;
    virtual void ConstructCopy(void* data, const void* other) const = 0;
    virtual void ConstructMove(void* data, void* rvalue) const NOEXCEPT = 0;
    virtual void ConstructMoveDestroy(void* data, void* rvalue) const NOEXCEPT = 0;
    virtual void ConstructSwap(void* data, void* other) const NOEXCEPT = 0;
    virtual void Destroy(void* data) const NOEXCEPT = 0;

    virtual FStringView TypeName() const = 0;

    virtual bool IsDefaultValue(const void* data) const NOEXCEPT = 0;
    virtual void ResetToDefaultValue(void* data) const = 0;

    virtual bool Equals(const void* lhs, const void* rhs) const NOEXCEPT = 0;
    virtual void Copy(const void* src, void* dst) const = 0;
    virtual void Move(void* src, void* dst) const NOEXCEPT = 0;

    virtual void Swap(void* lhs, void* rhs) const NOEXCEPT = 0;

    virtual bool DeepEquals(const void* lhs, const void* rhs) const = 0;
    virtual void DeepCopy(const void* src, void* dst) const = 0;

    virtual bool PromoteCopy(const void* src, const FAtom& dst) const = 0;
    virtual bool PromoteMove(void* src, const FAtom& dst) const NOEXCEPT = 0;

    virtual void* Cast(void* data, const PTypeTraits& dst) const = 0;

    virtual hash_t HashValue(const void* data) const NOEXCEPT = 0;

    virtual bool Accept(IAtomVisitor* visitor, void* data) const = 0;

public: // non-virtual helpers
    CONSTEXPR explicit ITypeTraits(FTypeInfos type)
        : _type(type)
    {}

    FTypeId TypeId() const { return _type.TypeId; }
    ETypeFlags TypeFlags() const { return _type.Flags(); }
    FTypeInfos TypeInfos() const { return _type; }
    size_t SizeInBytes() const { return _type.SizeInBytes(); }
    FSizeAndFlags SizeAndFlags() const { return _type.SizeAndFlags; }
    FNamedTypeInfos NamedTypeInfos() const { return FNamedTypeInfos(TypeName(), _type); }

    const IScalarTraits* AsScalar() const NOEXCEPT { return (TypeFlags() ^ ETypeFlags::Scalar ? checked_cast<const IScalarTraits*>(this) : nullptr); }
    const ITupleTraits* AsTuple() const NOEXCEPT { return (TypeFlags() ^ ETypeFlags::Tuple ? checked_cast<const ITupleTraits*>(this) : nullptr); }
    const IListTraits* AsList() const NOEXCEPT { return (TypeFlags() ^ ETypeFlags::List ? checked_cast<const IListTraits*>(this) : nullptr); }
    const IDicoTraits* AsDico() const NOEXCEPT { return (TypeFlags() ^ ETypeFlags::Dico ? checked_cast<const IDicoTraits*>(this) : nullptr); }

    const IScalarTraits& ToScalar() const { Assert_NoAssume(TypeFlags() ^ ETypeFlags::Scalar); return (*checked_cast<const IScalarTraits*>(this)); }
    const ITupleTraits& ToTuple() const { Assert_NoAssume(TypeFlags() ^ ETypeFlags::Tuple); return (*checked_cast<const ITupleTraits*>(this)); }
    const IListTraits& ToList() const { Assert_NoAssume(TypeFlags() ^ ETypeFlags::List); return (*checked_cast<const IListTraits*>(this)); }
    const IDicoTraits& ToDico() const { Assert_NoAssume(TypeFlags() ^ ETypeFlags::Dico); return (*checked_cast<const IDicoTraits*>(this)); }

    friend bool operator ==(const ITypeTraits& lhs, const ITypeTraits& rhs) {
#if 0
        return (lhs._typeId == rhs._typeId && lhs._sizeAndFlags == rhs._sizeAndFlags);
#else   // need to also compare the vtable : different allocators could be wrapped for instance
        return (FPlatformMemory::Memcmp(&lhs, &rhs, sizeof(ITypeTraits)) == 0);
#endif
    }
    friend bool operator !=(const ITypeTraits& lhs, const ITypeTraits& rhs) { return not operator ==(lhs, rhs); }

private:
    FTypeInfos _type;
};
STATIC_ASSERT(sizeof(ITypeTraits) == sizeof(i64)+sizeof(intptr_t));
//----------------------------------------------------------------------------
struct PTypeTraits {
    const ITypeTraits* PTraits;

    explicit PTypeTraits(Meta::FNoInit) NOEXCEPT {}
    CONSTEXPR PTypeTraits() NOEXCEPT : PTraits(nullptr) {}
    CONSTEXPR explicit PTypeTraits(const ITypeTraits* cpy) NOEXCEPT : PTraits(cpy) {}

    bool Valid() const { return (!!PTraits); }
    PPE_FAKEBOOL_OPERATOR_DECL() { return PTraits; }

    CONSTEXPR const ITypeTraits& operator *() const { Assert_NoAssume(Valid()); return *PTraits; }
    CONSTEXPR const ITypeTraits* operator ->() const { Assert_NoAssume(Valid()); return PTraits; }

    CONSTEXPR friend bool operator ==(const PTypeTraits& lhs, const PTypeTraits& rhs) {
        return (lhs.PTraits == rhs.PTraits);
    }
    CONSTEXPR friend bool operator !=(const PTypeTraits& lhs, const PTypeTraits& rhs) {
        return (not operator ==(lhs, rhs));
    }

    friend void swap(PTypeTraits& lhs, PTypeTraits& rhs) {
        std::swap(lhs.PTraits, rhs.PTraits);
    }

    CONSTEXPR void CreateRawCopy_AssumeNotInitialized(const ITypeTraits& traits) {
        PTraits = &traits;
    }

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CONSTEXPR inline PTypeTraits Traits(TType<void>) NOEXCEPT { return PTypeTraits(); }
//----------------------------------------------------------------------------
template <typename T>
PTypeTraits MakeTraits() NOEXCEPT; // defined in NativeTypes.h
//----------------------------------------------------------------------------
// only supports enums and classes :
PPE_RTTI_API PTypeTraits MakeTraitsFromTypename(const FName& typename_);
PPE_RTTI_API PTypeTraits MakeTraitsFromTypename(const FStringView& typename_);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IScalarTraits : public ITypeTraits {
public:
    CONSTEXPR explicit IScalarTraits(FTypeInfos type)
    :   ITypeTraits(type) {
        Assert_NoAssume(type.Flags() ^ ETypeFlags::Scalar);
    }

    virtual const FMetaEnum* EnumClass() const NOEXCEPT = 0;
    virtual const FMetaClass* ObjectClass() const NOEXCEPT = 0;
};
//----------------------------------------------------------------------------
class ITupleTraits : public ITypeTraits {
public: // ITypeTraits
    CONSTEXPR explicit ITupleTraits(FTypeInfos type)
    :   ITypeTraits(type) {
        Assert_NoAssume(type.Flags() ^ ETypeFlags::Tuple);
    }

    PPE_RTTI_API virtual bool Accept(IAtomVisitor* visitor, void* data) const override final;

public:
    virtual size_t Arity() const NOEXCEPT = 0;
    virtual TMemoryView<const PTypeTraits> TupleTraits() const NOEXCEPT = 0;

    virtual FAtom At(void* data, size_t index) const NOEXCEPT = 0;

    typedef TFunction<bool(const FAtom&)> foreach_fun;
    virtual bool ForEach(void* data, const foreach_fun& foreach) const NOEXCEPT = 0;
};
//----------------------------------------------------------------------------
class IListTraits : public ITypeTraits {
public: // ITypeTraits
    CONSTEXPR explicit IListTraits(FTypeInfos type)
    :   ITypeTraits(type) {
        Assert_NoAssume(type.Flags() ^ ETypeFlags::List);
    }

    PPE_RTTI_API virtual bool Accept(IAtomVisitor* visitor, void* data) const override final;

public: // IListTraits
    virtual PTypeTraits ValueTraits() const NOEXCEPT = 0;

    virtual size_t Count(const void* data) const NOEXCEPT = 0;
    virtual bool IsEmpty(const void* data) const NOEXCEPT = 0;

    virtual FAtom At(void* data, size_t index) const NOEXCEPT = 0;
    virtual size_t Find(const void* data, const FAtom& item) const NOEXCEPT = 0;

    virtual FAtom AddDefault(void* data) const = 0;
    virtual void AddCopy(void* data, const FAtom& item) const = 0;
    virtual void AddMove(void* data, const FAtom& item) const = 0;
    virtual void Erase(void* data, size_t index) const = 0;
    virtual bool Remove(void* data, const FAtom& item) const = 0;

    virtual void Reserve(void* data, size_t capacity) const = 0;
    virtual void Clear(void* data) const = 0;
    virtual void Empty(void* data, size_t capacity) const = 0;

    typedef TFunction<bool(const FAtom&)> foreach_fun;
    virtual bool ForEach(void* data, const foreach_fun& foreach) const NOEXCEPT = 0;

};
//----------------------------------------------------------------------------
class IDicoTraits : public ITypeTraits {
public: // ITypeTraits
    CONSTEXPR explicit IDicoTraits(FTypeInfos type)
    :   ITypeTraits(type) {
        Assert_NoAssume(type.Flags() ^ ETypeFlags::Dico);
    }

    PPE_RTTI_API virtual bool Accept(IAtomVisitor* visitor, void* data) const override final;

public: // IDicoTraits
    virtual PTypeTraits KeyTraits() const NOEXCEPT = 0;
    virtual PTypeTraits ValueTraits() const NOEXCEPT = 0;

    virtual size_t Count(const void* data) const NOEXCEPT = 0;
    virtual bool IsEmpty(const void* data) const NOEXCEPT = 0;

    virtual FAtom Find(const void* data, const FAtom& key) const NOEXCEPT = 0;

    virtual FAtom AddDefaultCopy(void* data, const FAtom& key) const = 0;
    virtual FAtom AddDefaultMove(void* data, const FAtom& key) const = 0;

    virtual void AddCopy(void* data, const FAtom& key, const FAtom& value) const = 0;
    virtual void AddMove(void* data, const FAtom& key, const FAtom& value) const = 0;
    virtual bool Remove(void* data, const FAtom& key) const = 0;

    virtual void Reserve(void* data, size_t capacity) const = 0;
    virtual void Clear(void* data) const = 0;
    virtual void Empty(void* data, size_t capacity) const = 0;

    typedef TFunction<bool(const FAtom&, const FAtom&)> foreach_fun;
    virtual bool ForEach(void* data, const foreach_fun& foreach) const NOEXCEPT = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Consider PTypeTraits as POD since it's a simple pointer wrapper
PPE_ASSUME_TYPE_AS_POD(RTTI::PTypeTraits)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
