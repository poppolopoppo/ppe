// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "RTTI/Any.h"

#include "MetaObject.h"

#include "HAL/PlatformMemory.h"
#include "IO/FileSystem.h"
#include "IO/String.h"
#include "Maths/ScalarMatrix.h"
#include "Maths/ScalarVector.h"

#define USE_PPE_RTTI_ANY_INSITU (not USE_PPE_MEMORY_DEBUGGING)

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
STATIC_ASSERT(sizeof(FAny) == FAny::GInSituSize + sizeof(intptr_t)); // checks for padding
constexpr size_t GNumSupportedTypes = static_cast<size_t>(0)
#define DECL_RTTI_NATIVETYPE_SUPPORTED(_Name, T, _TypeId) + size_t(1)
FOREACH_RTTI_NATIVETYPES(DECL_RTTI_NATIVETYPE_SUPPORTED)
#undef DECL_RTTI_NATIVETYPE_SUPPORTED
    ;
constexpr size_t GNumSupportedTypesFittingInSitu = static_cast<size_t>(0)
#define DECL_RTTI_NATIVETYPE_SUPPORTED(_Name, T, _TypeId) + size_t(sizeof(T) <= FAny::GInSituSize ? 1 : 0)
FOREACH_RTTI_NATIVETYPES(DECL_RTTI_NATIVETYPE_SUPPORTED)
#undef DECL_RTTI_NATIVETYPE_SUPPORTED
    ;
// checks that all types can be wrapped in except for FAny
STATIC_ASSERT(GNumSupportedTypesFittingInSitu == GNumSupportedTypes - static_cast<size_t>(1)/* FAny */);
STATIC_ASSERT(sizeof(FAny) > sizeof(FAny::GInSituSize));
//----------------------------------------------------------------------------
static FORCE_INLINE bool Any_FitInSitu_(size_t sizeInBytes) {
#if USE_PPE_RTTI_ANY_INSITU
    return (sizeInBytes <= FAny::GInSituSize);
#else
    Unused(sizeInBytes);
    return false;
#endif
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FAny::FAny(const FAny& other) : FAny() {
    if (Likely(other.Valid()))
        AssignCopy_AssumeNotInitialized_(other.Data(), *other.Traits());
    else
        Assert_NoAssume(not Valid());

    Assert(Equals(other));
}
//----------------------------------------------------------------------------
FAny& FAny::operator =(const FAny& other) {
    if (Likely(other.Valid()))
        AssignCopy_(other.Data(), *other.Traits());
    else if (Valid())
        Reset_AssumeInitialized_();

    Assert(Equals(other));
    return (*this);
}
//----------------------------------------------------------------------------
FAny::FAny(FAny&& rvalue) NOEXCEPT : FAny() {
    if (Likely(rvalue.Valid())) {
        if (rvalue.IsFittingInSitu_()) {
            AssignMoveDestroy_AssumeNotInitialized_(rvalue.Data(), *rvalue.Traits());
            rvalue._traitsWFlags = {};
        }
        else {
            using std::swap;
            swap(_traitsWFlags, rvalue._traitsWFlags);
            swap(_allocatorBlock, rvalue._allocatorBlock);
        }

        ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(&rvalue._inSitu, GInSituSize));

        Assert_NoAssume(Valid());
        Assert_NoAssume(not rvalue.Valid());
    }
    else {
        Assert_NoAssume(not Valid());
    }
}
//----------------------------------------------------------------------------
FAny& FAny::operator =(FAny&& rvalue) NOEXCEPT {
    if (Likely(rvalue.Valid())) {
        if (Likely(rvalue.IsFittingInSitu_())) {
            AssignMoveDestroy_(rvalue.Data(), *rvalue.Traits());
            rvalue._traitsWFlags = {};
        }
        else {
            if (Valid())
                Reset_AssumeInitialized_();

            using std::swap;
            swap(_traitsWFlags, rvalue._traitsWFlags);
            swap(_allocatorBlock, rvalue._allocatorBlock);
        }

        ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(&rvalue._inSitu, GInSituSize));

        Assert_NoAssume(Valid());
        Assert_NoAssume(not rvalue.Valid());
    }
    else if (Valid()) {
        Reset_AssumeInitialized_();
    }

    return (*this);
}
//----------------------------------------------------------------------------
FAny::FAny(ENativeType nativeType)
:   FAny(MakeTraits(nativeType))
{}
//----------------------------------------------------------------------------
FAny::FAny(const PTypeTraits& traits) : FAny() {
    Assert(traits);

    if (Likely(MakeTraits<FAny>() != traits)) {
        SetTraits_(*traits.PTraits);

        const size_t sizeInBytes = traits->SizeInBytes();
        void* const data = Allocate_AssumeNotInitialized_(sizeInBytes);

        traits->Construct(data);
        Assert(traits->IsDefaultValue(data));
    }
}
//----------------------------------------------------------------------------
FAny::~FAny()  {
    if (Likely(Valid()))
        Reset_AssumeInitialized_();
}
//----------------------------------------------------------------------------
FAny& FAny::Reset(ENativeType type) {
    return Reset(MakeTraits(type));
}
//----------------------------------------------------------------------------
FAny& FAny::Reset(const ITypeTraits& traits) {
    // reset to default value if already holding the same traits
    if (Traits().PTraits == &traits) {
        traits.ResetToDefaultValue(Data());
        return (*this);
    }

    // special case when asking for FAny: we don't wrap FAny in FAny, instead it wraps itself
    if (Unlikely(MakeTraits<FAny>().PTraits == &traits)) {
        Reset(); // see Data() / Traits()
        return (*this);
    }

    const size_t newSize = traits.SizeInBytes();

    void* data = nullptr;

    // check if we can reuse current external block, if any
    if (Valid() &&
        not IsFittingInSitu_() &&
        not Any_FitInSitu_(newSize) &&
        newSize <= _allocatorBlock.SizeInBytes) {
        Assert(Meta::IsAlignedPow2(traits.Alignment(), _allocatorBlock.Data));

        Traits()->Destroy(_allocatorBlock.Data);

        data = _allocatorBlock.Data;
    }
    else {
        if (Valid())
            Reset_AssumeInitialized_();

        data = Allocate_AssumeNotInitialized_(traits.SizeInBytes());
    }

    Assert(data);
    SetTraits_(traits);
    traits.Construct(data);
    return (*this);
}
//----------------------------------------------------------------------------
bool FAny::Equals(const FAny& other) const {
    const PTypeTraits traits = Traits();

    if (traits != other.Traits())
        return false;

    if (Valid())
        return traits->Equals(Data(), other.Data());

    return (not other.Valid());
}
//----------------------------------------------------------------------------
hash_t FAny::HashValue() const {
    return (Valid()
        ? Traits()->HashValue(Data())
        : hash_t(static_cast<size_t>(-1)) );
}
//----------------------------------------------------------------------------
void FAny::Swap(FAny& other) {
    const PTypeTraits traits = Traits();

    if (traits != other.Traits())
        std::swap(*this, other);
    else if (traits)
        traits->Swap(Data(), other.Data());
}
//----------------------------------------------------------------------------
void FAny::SetTraits_(const ITypeTraits& traits) NOEXCEPT {
    _traitsWFlags.Set(&traits);
}
//----------------------------------------------------------------------------
void FAny::AssignCopy_(const void* src, const ITypeTraits& traits) {
    Assert(src);

    if (Traits().PTraits == &traits) {
        traits.Copy(src, Data());
        return;
    }

    Reset(traits);

    traits.ConstructCopy(Data(), src);
    Assert(traits.Equals(Data(), src));
}
//----------------------------------------------------------------------------
void FAny::AssignMove_(void* src, const ITypeTraits& traits) NOEXCEPT {
    Assert(src);

    if (Traits().PTraits == &traits) {
        traits.Move(src, Data());
        return;
    }

    Reset(traits);
    traits.ConstructMove(Data(), src);
}
//----------------------------------------------------------------------------
void FAny::AssignMoveDestroy_(void* src, const ITypeTraits& traits) NOEXCEPT {
    Assert(src);

    if (Traits().PTraits == &traits) {
        traits.Move(src, Data());
        traits.Destroy(src);
        return;
    }

    Reset(traits);
    traits.ConstructMoveDestroy(Data(), src);
}
//----------------------------------------------------------------------------
void FAny::AssignCopy_AssumeNotInitialized_(const void* src, const ITypeTraits& traits) {
    Assert(src);

    SetTraits_(traits);

    const size_t sz = traits.SizeInBytes();
    void* const data = Allocate_AssumeNotInitialized_(sz);

    traits.ConstructCopy(data, src);
    Assert(traits.Equals(data, src));
}
//----------------------------------------------------------------------------
void FAny::AssignMove_AssumeNotInitialized_(void* src, const ITypeTraits& traits) NOEXCEPT {
    Assert(src);

    SetTraits_(traits);

    const size_t sz = traits.SizeInBytes();
    void* const data = Allocate_AssumeNotInitialized_(sz);

    traits.ConstructMove(data, src);
}
//----------------------------------------------------------------------------
void FAny::AssignMoveDestroy_AssumeNotInitialized_(void* src, const ITypeTraits& traits) NOEXCEPT {
    Assert(src);

    SetTraits_(traits);

    const size_t sz = traits.SizeInBytes();
    void* const data = Allocate_AssumeNotInitialized_(sz);

    traits.ConstructMoveDestroy(data, src);
}
//----------------------------------------------------------------------------
void* FAny::Allocate_AssumeNotInitialized_(const size_t sizeInBytes) {
    Assert(sizeInBytes);

    if (Likely(Any_FitInSitu_(sizeInBytes))) {
        SetFittingInSitu_(true);

        return (&_inSitu);
    }

    SetFittingInSitu_(false);

    _allocatorBlock = TRACKING_MALLOC_FOR_NEW(Any, sizeInBytes);
    return _allocatorBlock.Data;
}
//----------------------------------------------------------------------------
void FAny::Reset_AssumeInitialized_() {
    Assert(Valid());

    Traits()->Destroy(Data());

    if (not IsFittingInSitu_()) {
        Assert_NoAssume(_allocatorBlock.Data);

        TRACKING_FREE_FOR_DELETE(Any, _allocatorBlock);

        _allocatorBlock = Default;
    }

    _traitsWFlags = Default; // don't want to call the dtor

    ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(&_inSitu, GInSituSize));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void AssignCopy(FAny* dst, const void* src, const ITypeTraits& traits) {
    dst->AssignCopy(src, traits);
}
//----------------------------------------------------------------------------
void AssignMove(FAny* dst, void* src, const ITypeTraits& traits) NOEXCEPT {
    dst->AssignMove(src, traits);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
