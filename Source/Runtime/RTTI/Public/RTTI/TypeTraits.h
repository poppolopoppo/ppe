#pragma once

#include "RTTI_fwd.h"

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

    virtual FStringLiteral TypeName() const = 0;

    virtual bool IsDefaultValue(const void* data) const NOEXCEPT = 0;
    virtual void ResetToDefaultValue(void* data) const = 0;

    virtual bool Equals(const void* lhs, const void* rhs) const NOEXCEPT = 0;
    virtual void Copy(const void* src, void* dst) const = 0;
    virtual void Move(void* src, void* dst) const NOEXCEPT = 0;

    virtual void Swap(void* lhs, void* rhs) const NOEXCEPT = 0;

    NODISCARD virtual bool DeepEquals(const void* lhs, const void* rhs) const = 0;
    virtual void DeepCopy(const void* src, void* dst) const = 0;

    NODISCARD virtual bool PromoteCopy(const void* src, const FAtom& dst) const = 0;
    NODISCARD virtual bool PromoteMove(void* src, const FAtom& dst) const NOEXCEPT = 0;

    virtual void* Cast(void* data, const PTypeTraits& dst) const = 0;

    virtual PTypeTraits CommonType(const PTypeTraits& other) const NOEXCEPT = 0;

    virtual hash_t HashValue(const void* data) const NOEXCEPT = 0;

    NODISCARD virtual bool Accept(IAtomVisitor* visitor, void* data) const = 0;

public: // non-virtual helpers
    CONSTEXPR explicit ITypeTraits(FTypeInfos type)
        : _type(type)
    {}

    FTypeId TypeId() const { return _type.TypeId; }
    ETypeFlags TypeFlags() const { return _type.Flags(); }
    FTypeInfos TypeInfos() const { return _type; }
    size_t Alignment() const { return _type.Alignment(); }
    size_t SizeInBytes() const { return _type.SizeInBytes(); }
    FSizeAndFlags SizeAndFlags() const { return _type.SizeAndFlags; }
    FNamedTypeInfos NamedTypeInfos() const { return FNamedTypeInfos(TypeName(), _type); }

    const IScalarTraits* AsScalar() const NOEXCEPT;
    const ITupleTraits* AsTuple() const NOEXCEPT;
    const IListTraits* AsList() const NOEXCEPT;
    const IDicoTraits* AsDico() const NOEXCEPT;

    const IScalarTraits& ToScalar() const;
    const ITupleTraits& ToTuple() const;
    const IListTraits& ToList() const;
    const IDicoTraits& ToDico() const;

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

    CONSTEXPR PTypeTraits() NOEXCEPT : PTraits(nullptr) {}
    CONSTEXPR explicit PTypeTraits(const ITypeTraits* cpy) NOEXCEPT : PTraits(cpy) {}

    bool Valid() const { return (!!PTraits); }
    PPE_FAKEBOOL_OPERATOR_DECL() { return (!!PTraits); }

    CONSTEXPR const ITypeTraits& operator *() const { Assert_NoAssume(Valid()); return *PTraits; }
    CONSTEXPR const ITypeTraits* operator ->() const { Assert_NoAssume(Valid()); return PTraits; }

    CONSTEXPR friend bool operator ==(const PTypeTraits& lhs, const PTypeTraits& rhs) {
        return (lhs.PTraits == rhs.PTraits);
    }
    CONSTEXPR friend bool operator !=(const PTypeTraits& lhs, const PTypeTraits& rhs) {
        return (not operator ==(lhs, rhs));
    }

    friend hash_t hash_value(const PTypeTraits& traits) NOEXCEPT {
        return hash_ptr(traits.PTraits);
    }

    friend void swap(PTypeTraits& lhs, PTypeTraits& rhs) NOEXCEPT {
        std::swap(lhs.PTraits, rhs.PTraits);
    }

    CONSTEXPR void CreateRawCopy_AssumeNotInitialized(const ITypeTraits& traits) {
        PTraits = &traits;
    }

};
PPE_ASSUME_TYPE_AS_POD(PTypeTraits)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CONSTEXPR inline PTypeTraits Traits(void*) NOEXCEPT { return PTypeTraits(); }
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
        Assert_NoAssume(is_scalar_v(type.Flags()));
    }

    virtual int Compare(const void* lhs, const void* rhs) const NOEXCEPT = 0;

    virtual bool FromString(void* dst, const FStringConversion& iss) const NOEXCEPT = 0;

    virtual const FMetaEnum* EnumClass() const NOEXCEPT = 0;
    virtual const FMetaClass* ObjectClass() const NOEXCEPT = 0;

public: // helpers
    ENativeType NativeType() const NOEXCEPT { return static_cast<ENativeType>(TypeId()); }

    bool Less(const void* lhs, const void* rhs) const NOEXCEPT { return Compare(lhs, rhs) < 0; }
    bool LessEqual(const void* lhs, const void* rhs) const NOEXCEPT { return Compare(lhs, rhs) <= 0; }
    bool Greater(const void* lhs, const void* rhs) const NOEXCEPT { return Compare(lhs, rhs) > 0; }
    bool GreaterEqual(const void* lhs, const void* rhs) const NOEXCEPT { return Compare(lhs, rhs) >= 0; }

    void* Min(void* lhs, void* rhs) const NOEXCEPT { return (Less(lhs, rhs) ? lhs : rhs); }
    void* Max(void* lhs, void* rhs) const NOEXCEPT { return (Greater(lhs, rhs) ? lhs : rhs); }

    const void* Min(const void* lhs, const void* rhs) const NOEXCEPT { return (Less(lhs, rhs) ? lhs : rhs); }
    const void* Max(const void* lhs, const void* rhs) const NOEXCEPT { return (Greater(lhs, rhs) ? lhs : rhs); }
};
//----------------------------------------------------------------------------
class ITupleTraits : public ITypeTraits {
public: // ITypeTraits
    CONSTEXPR explicit ITupleTraits(FTypeInfos type)
    :   ITypeTraits(type) {
        Assert_NoAssume(is_tuple_v(type.Flags()));
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
        Assert_NoAssume(is_list_v(type.Flags()));
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
        Assert_NoAssume(is_dico_v(type.Flags()));
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
inline const IScalarTraits* ITypeTraits::AsScalar() const NOEXCEPT {
    return (is_scalar_v(TypeFlags()) ? checked_cast<const IScalarTraits*>(this) : nullptr);
}
inline const ITupleTraits* ITypeTraits::AsTuple() const NOEXCEPT {
    return (is_tuple_v(TypeFlags()) ? checked_cast<const ITupleTraits*>(this) : nullptr);
}
inline const IListTraits* ITypeTraits::AsList() const NOEXCEPT {
    return (is_list_v(TypeFlags()) ? checked_cast<const IListTraits*>(this) : nullptr);
}
inline const IDicoTraits* ITypeTraits::AsDico() const NOEXCEPT {
    return (is_dico_v(TypeFlags()) ? checked_cast<const IDicoTraits*>(this) : nullptr);
}
//----------------------------------------------------------------------------
inline const IScalarTraits& ITypeTraits::ToScalar() const {
    Assert_NoAssume(is_scalar_v(TypeFlags()));
    return (*checked_cast<const IScalarTraits*>(this));
}
inline const ITupleTraits& ITypeTraits::ToTuple() const {
    Assert_NoAssume(is_tuple_v(TypeFlags()));
    return (*checked_cast<const ITupleTraits*>(this));
}
inline const IListTraits& ITypeTraits::ToList() const {
    Assert_NoAssume(is_list_v(TypeFlags()));
    return (*checked_cast<const IListTraits*>(this));
}
inline const IDicoTraits& ITypeTraits::ToDico() const {
    Assert_NoAssume(is_dico_v(TypeFlags()));
    return (*checked_cast<const IDicoTraits*>(this));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
