#pragma once

#include "RTTI.h"

#include "RTTI/TypeInfos.h"

#include "HAL/PlatformMemory.h"
#include "IO/String_fwd.h"
#include "IO/TextWriter_fwd.h"
#include "Misc/Function.h"
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
bool AtomVisit(IAtomVisitor& visitor, const ITupleTraits* tuple, void* data);
bool AtomVisit(IAtomVisitor& visitor, const IListTraits* list, void* data);
bool AtomVisit(IAtomVisitor& visitor, const IDicoTraits* dico, void* data);
template <typename T>
bool AtomVisit(IAtomVisitor& visitor, const IScalarTraits* scalar, T& value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename T> struct TTraitsHolder {
static const T GInstance;
};
template <typename T>
const T TTraitsHolder<T>::GInstance;
} //!details
//----------------------------------------------------------------------------
struct PPE_RTTI_API PTypeTraits {
    const ITypeTraits* Traits;

    FORCE_INLINE explicit PTypeTraits(Meta::FNoInit) NOEXCEPT {}
    CONSTEXPR explicit PTypeTraits(const ITypeTraits* traits = nullptr) NOEXCEPT : Traits(traits) {}

    bool Valid() const { return (nullptr != Traits); }
    PPE_FAKEBOOL_OPERATOR_DECL() { return Traits; }

    const ITypeTraits* get() const { return Traits; }

    const ITypeTraits& operator *() const { Assert_NoAssume(Valid()); return *Traits; }
    const ITypeTraits* operator ->() const { Assert_NoAssume(Valid()); return Traits; }

    inline friend bool operator ==(const PTypeTraits& lhs, const PTypeTraits& rhs) {
        return (lhs.Traits == rhs.Traits);
    }
    inline friend bool operator !=(const PTypeTraits& lhs, const PTypeTraits& rhs) {
        return not operator ==(lhs, rhs);
    }

    inline friend void swap(PTypeTraits& lhs, PTypeTraits& rhs) {
        std::swap(lhs.Traits, rhs.Traits);
    }

    void CreateRawCopy_AssumeNotInitialized(const ITypeTraits& traits) {
        Traits = &traits;
    }

    /** creates type traits at compile time, no overhead at runtime **/
    template <typename T, class = Meta::TEnableIf<std::is_base_of_v<ITypeTraits, T>> >
    static CONSTEXPR PTypeTraits Make() NOEXCEPT {
        return PTypeTraits{ &details::TTraitsHolder<T>::GInstance };
    }
};
//----------------------------------------------------------------------------
class PPE_RTTI_API ITypeTraits {
public:
    virtual ~ITypeTraits() {}

    virtual void Construct(void* data) const = 0;
    virtual void ConstructCopy(void* data, const void* other) const = 0;
    virtual void ConstructMove(void* data, void* rvalue) const = 0;
    virtual void ConstructMoveDestroy(void* data, void* rvalue) const = 0;
    virtual void ConstructSwap(void* data, void* other) const = 0;
    virtual void Destroy(void* data) const = 0;

    virtual FStringView TypeName() const = 0;

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

public: // non-virtual helpers
    CONSTEXPR ITypeTraits(FTypeId typeId, ETypeFlags flags, size_t sizeInBytes)
        : _typeId(typeId)
        , _sizeAndFlags(sizeInBytes, flags)
    {}

    FTypeId TypeId() const { return _typeId; }
    ETypeFlags TypeFlags() const { return _sizeAndFlags.Flags(); }
    FTypeInfos TypeInfos() const { return FTypeInfos(TypeName(), TypeId(), TypeFlags(), SizeInBytes()); }
    size_t SizeInBytes() const { return _sizeAndFlags.SizeInBytes(); }
    FSizeAndFlags SizeAndFlags() const { return _sizeAndFlags; }

    const IScalarTraits* AsScalar() const { return (TypeFlags() ^ ETypeFlags::Scalar ? checked_cast<const IScalarTraits*>(this) : nullptr); }
    const ITupleTraits* AsTuple() const { return (TypeFlags() ^ ETypeFlags::Tuple ? checked_cast<const ITupleTraits*>(this) : nullptr); }
    const IListTraits* AsList() const { return (TypeFlags() ^ ETypeFlags::List ? checked_cast<const IListTraits*>(this) : nullptr); }
    const IDicoTraits* AsDico() const { return (TypeFlags() ^ ETypeFlags::Dico ? checked_cast<const IDicoTraits*>(this) : nullptr); }

    const IScalarTraits& ToScalar() const { Assert_NoAssume(TypeFlags() ^ ETypeFlags::Scalar); return (*checked_cast<const IScalarTraits*>(this)); }
    const ITupleTraits& ToTuple() const { Assert_NoAssume(TypeFlags() ^ ETypeFlags::Tuple); return (*checked_cast<const ITupleTraits*>(this)); }
    const IListTraits& ToList() const { Assert_NoAssume(TypeFlags() ^ ETypeFlags::List); return (*checked_cast<const IListTraits*>(this)); }
    const IDicoTraits& ToDico() const { Assert_NoAssume(TypeFlags() ^ ETypeFlags::Dico); return (*checked_cast<const IDicoTraits*>(this)); }

    inline friend bool operator ==(const ITypeTraits& lhs, const ITypeTraits& rhs) {
#if 0
        return (lhs._typeId == rhs._typeId && lhs._sizeAndFlags == rhs._sizeAndFlags);
#else   // need to also compare the vtable : different allocators could be wrapped for instance
        return (FPlatformMemory::Memcmp(&lhs, &rhs, sizeof(ITypeTraits)) == 0);
#endif
    }
    inline friend bool operator !=(const ITypeTraits& lhs, const ITypeTraits& rhs) { return not operator ==(lhs, rhs); }

private:
    FTypeId _typeId;
    FSizeAndFlags _sizeAndFlags;
};
STATIC_ASSERT(sizeof(ITypeTraits) == sizeof(i64)+sizeof(intptr_t));
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CONSTEXPR inline PTypeTraits Traits(Meta::TType<void>) NOEXCEPT { return PTypeTraits(); }
//----------------------------------------------------------------------------
template <typename T>
PTypeTraits MakeTraits() NOEXCEPT; // defined in NativeTypes.h
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_RTTI_API FTypeId MakeTupleTypeId(const TMemoryView<const PTypeTraits>& elements);
PPE_RTTI_API FTypeId MakeListTypeId(const PTypeTraits& value);
PPE_RTTI_API FTypeId MakeDicoTypeId(const PTypeTraits& key, const PTypeTraits& value);
//----------------------------------------------------------------------------
PPE_RTTI_API ETypeFlags MakeTupleTypeFlags(const TMemoryView<const PTypeTraits>& elements);
PPE_RTTI_API ETypeFlags MakeListTypeFlags(const PTypeTraits& value);
PPE_RTTI_API ETypeFlags MakeDicoTypeFlags(const PTypeTraits& key, const PTypeTraits& value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IScalarTraits : public ITypeTraits {
public:
    CONSTEXPR IScalarTraits(FTypeId typeId, ETypeFlags flags, size_t sizeInBytes)
        : ITypeTraits(typeId, flags + ETypeFlags::Scalar, sizeInBytes)
    {}

    virtual const FMetaEnum* EnumClass() const = 0;
    virtual const FMetaClass* ObjectClass() const = 0;
};
//----------------------------------------------------------------------------
class ITupleTraits : public ITypeTraits {
public: // ITypeTraits
    CONSTEXPR ITupleTraits(FTypeId typeId, ETypeFlags flags, size_t sizeInBytes)
        : ITypeTraits(typeId, flags + ETypeFlags::Tuple, sizeInBytes)
    {}

    virtual bool Accept(IAtomVisitor* visitor, void* data) const override final;

public:
    virtual size_t Arity() const = 0;
    virtual TMemoryView<const PTypeTraits> TupleTraits() const = 0;

    virtual FAtom At(void* data, size_t index) const = 0;

    typedef TFunction<bool(const FAtom&)> foreach_fun;
    virtual bool ForEach(void* data, const foreach_fun& foreach) const = 0;
};
//----------------------------------------------------------------------------
class IListTraits : public ITypeTraits {
public: // ITypeTraits
    CONSTEXPR IListTraits(FTypeId typeId, ETypeFlags flags, size_t sizeInBytes)
        : ITypeTraits(typeId, flags + ETypeFlags::List, sizeInBytes)
    {}

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
};
//----------------------------------------------------------------------------
class IDicoTraits : public ITypeTraits {
public: // ITypeTraits
    CONSTEXPR IDicoTraits(FTypeId typeId, ETypeFlags flags, size_t sizeInBytes)
        : ITypeTraits(typeId, flags + ETypeFlags::Dico, sizeInBytes)
    {}

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
