#include "stdafx.h"

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
constexpr size_t GNumSupportedTypes = size_t(0)
#define DECL_RTTI_NATIVETYPE_SUPPORTED(_Name, T, _TypeId) + size_t(1)
FOREACH_RTTI_NATIVETYPES(DECL_RTTI_NATIVETYPE_SUPPORTED)
#undef DECL_RTTI_NATIVETYPE_SUPPORTED
    ;
constexpr size_t GNumSupportedTypesFittingInSitu = size_t(0)
#define DECL_RTTI_NATIVETYPE_SUPPORTED(_Name, T, _TypeId) + size_t(sizeof(T) <= FAny::GInSituSize ? 1 : 0)
FOREACH_RTTI_NATIVETYPES(DECL_RTTI_NATIVETYPE_SUPPORTED)
#undef DECL_RTTI_NATIVETYPE_SUPPORTED
    ;
// checks that all types can be wrapped in except for FAny
STATIC_ASSERT(GNumSupportedTypesFittingInSitu == GNumSupportedTypes - size_t(1)/* FAny */);
STATIC_ASSERT(sizeof(FAny) > sizeof(FAny::GInSituSize));
//----------------------------------------------------------------------------
static FORCE_INLINE bool Any_FitInSitu_(size_t sizeInBytes) {
#if USE_PPE_RTTI_ANY_INSITU
    return (sizeInBytes <= FAny::GInSituSize);
#else
    NOOP(sizeInBytes);
    return false;
#endif
}
//----------------------------------------------------------------------------
static const ITypeTraits& Any_Traits_() {
    ONE_TIME_INITIALIZE(const PTypeTraits, GInstance, MakeTraits<FAny>());
    return (*GInstance);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FAny::FAny(const FAny& other)
:   _traits(Meta::NoInit) {

    if (other._traits) {
        const size_t sizeInBytes = other._traits->SizeInBytes();
        AssignCopy_AssumeNotInitialized_(other.Data_(sizeInBytes), *other._traits, sizeInBytes);
    }
    else {
        _traits = PTypeTraits();
        ONLY_IF_ASSERT(FPlatformMemory::Memset(&_inSitu, 0xDD, GInSituSize));
    }

    Assert(Equals(other));
}
//----------------------------------------------------------------------------
FAny& FAny::operator =(const FAny& other) {
    if (other._traits) {
        const size_t sizeInBytes = other._traits->SizeInBytes();
        AssignCopy_(other.Data_(sizeInBytes), *other._traits, sizeInBytes);
    }
    else if (_traits) {
        Reset_AssumeInitialized_(_traits->SizeInBytes());
    }

    Assert(Equals(other));
    return (*this);
}
//----------------------------------------------------------------------------
FAny::FAny(FAny&& rvalue) NOEXCEPT
:   _traits(Meta::NoInit) {

    if (rvalue._traits) {
        const size_t sizeInBytes = rvalue._traits->SizeInBytes();

        if (Any_FitInSitu_(sizeInBytes)) {
            AssignMoveDestroy_AssumeNotInitialized_(&rvalue._inSitu, *rvalue._traits, sizeInBytes);
        }
        else {
            _traits = rvalue._traits;
            _externalBlock = rvalue._externalBlock; // steel the allocation
        }

        rvalue._traits = PTypeTraits();
        ONLY_IF_ASSERT(FPlatformMemory::Memset(&rvalue._inSitu, 0xDD, GInSituSize));

        Assert(_traits);
    }
    else {
        _traits = PTypeTraits();
        ONLY_IF_ASSERT(FPlatformMemory::Memset(&_inSitu, 0xDD, GInSituSize));
    }
}
//----------------------------------------------------------------------------
FAny& FAny::operator =(FAny&& rvalue) NOEXCEPT {
    if (rvalue._traits) {
        const size_t sizeInBytes = rvalue._traits->SizeInBytes();

        if (Any_FitInSitu_(sizeInBytes)) {
            AssignMoveDestroy_(&rvalue._inSitu, *rvalue._traits, sizeInBytes);
        }
        else {
            if (_traits) // release our own allocation in necessary
                Reset_AssumeInitialized_(_traits->SizeInBytes());

            _traits = rvalue._traits;
            _externalBlock = rvalue._externalBlock; // steel the allocation
        }

        rvalue._traits = PTypeTraits();
        ONLY_IF_ASSERT(FPlatformMemory::Memset(&rvalue._inSitu, 0xDD, GInSituSize));
    }
    else if (_traits) {
        Reset_AssumeInitialized_(_traits->SizeInBytes());
    }

    return (*this);
}
//----------------------------------------------------------------------------
FAny::FAny(ENativeType nativeType)
:   FAny(MakeTraits(nativeType))
{}
//----------------------------------------------------------------------------
FAny::FAny(const PTypeTraits& traits)
:   _traits(Meta::NoInit) {
    Assert(traits);
    Assert(Any_Traits_() != *traits);

    const size_t sizeInBytes = traits->SizeInBytes();
    void* const data = Allocate_AssumeNotInitialized_(sizeInBytes);

    _traits = traits;
    _traits->Construct(data);

    Assert(_traits->IsDefaultValue(data));
}
//----------------------------------------------------------------------------
FAny::FAny() NOEXCEPT {
    ONLY_IF_ASSERT(FPlatformMemory::Memset(&_inSitu, 0xDD, GInSituSize));
}
//----------------------------------------------------------------------------
FAny::~FAny()  {
    if (_traits)
        Reset_AssumeInitialized_(_traits->SizeInBytes());
}
//----------------------------------------------------------------------------
FAny& FAny::Reset(const PTypeTraits& traits) {
    Assert(traits);
    Assert(Any_Traits_() != *traits);

    if (_traits == traits) {
        Assert(_traits);

        _traits->ResetToDefaultValue(Data());

        return (*this);
    }

    const size_t newSize = traits->SizeInBytes();

    if (_traits) {
        const size_t oldSize = _traits->SizeInBytes();

        if (not Any_FitInSitu_(newSize) &&
            not Any_FitInSitu_(oldSize) &&
            newSize <= _externalBlock.SizeInBytes ) {

            _traits->Destroy(_externalBlock.Ptr);
            _traits = traits;
            _traits->Construct(_externalBlock.Ptr);

            return (*this);
        }
        else {
            Reset_AssumeInitialized_(oldSize);
        }
    }

    void* const data = Allocate_AssumeNotInitialized_(newSize);

    _traits = traits;
    _traits->Construct(data);

    Assert(_traits->IsDefaultValue(data));
    return (*this); // can be implicit casted as FAtom for convenience
}
//----------------------------------------------------------------------------
bool FAny::Equals(const FAny& other) const {
    if (_traits != other._traits)
        return false;
    else if (not _traits)
        return true;
    else {
        const size_t sizeInBytes = _traits->SizeInBytes();
        return (Any_FitInSitu_(sizeInBytes)
            ? _traits->Equals(&_inSitu, &other._inSitu)
            : _traits->Equals(_externalBlock.Ptr, other._externalBlock.Ptr) );
    }
}
//----------------------------------------------------------------------------
hash_t FAny::HashValue() const {
    return (_traits
        ? _traits->HashValue(Data())
        : hash_t(size_t(-1)) );
}
//----------------------------------------------------------------------------
void FAny::Swap(FAny& other) {
    if (_traits != other._traits) {
        FAny tmp(std::move(*this));
        *this = std::move(other);
        other = std::move(tmp);
    }
    else if (_traits) {
        const size_t sizeInBytes = _traits->SizeInBytes();

        if (Any_FitInSitu_(sizeInBytes))
            _traits->Swap(&_inSitu, &other._inSitu);
        else
            _traits->Swap(&_externalBlock, &other._externalBlock);
    }
}
//----------------------------------------------------------------------------
void* FAny::Data_(const size_t sizeInBytes) const {
    Assert(_traits);
    Assert(_traits->SizeInBytes() == sizeInBytes);

    return (void*)(Any_FitInSitu_(sizeInBytes) ? &_inSitu : _externalBlock.Ptr);
}
//----------------------------------------------------------------------------
void FAny::AssignCopy_(const void* src, const ITypeTraits& traits, const size_t sizeInBytes) {
    Assert(src);
    Assert(traits.SizeInBytes() == sizeInBytes);

    if (Any_Traits_() == traits) { // avoid wrapping FAny in FAny

        if (static_cast<const FAny*>(src)->Valid()) {
            AssignCopy(static_cast<const FAny*>(src)->InnerAtom());
        }
        else if (_traits) {
            Reset_AssumeInitialized_(_traits->SizeInBytes());
        }

        return;
    }
    else if (_traits) {

        if (traits == *_traits) {
            traits.Copy(src, Data_(sizeInBytes));

            return;
        }

        const size_t oldSize = _traits->SizeInBytes();

        if (not Any_FitInSitu_(sizeInBytes) &&
            not Any_FitInSitu_(oldSize) &&
            sizeInBytes <= _externalBlock.SizeInBytes ) {

            _traits->Destroy(_externalBlock.Ptr);
            _traits.CreateRawCopy_AssumeNotInitialized(traits);
            _traits->ConstructCopy(_externalBlock.Ptr, src);

            return;
        }

        Reset_AssumeInitialized_(oldSize);
    }
    Assert(not _traits);

    void* const data = Allocate_AssumeNotInitialized_(sizeInBytes);

    _traits.CreateRawCopy_AssumeNotInitialized(traits);
    _traits->ConstructCopy(data, src);

    Assert(traits.Equals(data, src));
}
//----------------------------------------------------------------------------
void FAny::AssignMove_(void* src, const ITypeTraits& traits, const size_t sizeInBytes) {
    Assert(src);
    Assert(traits.SizeInBytes() == sizeInBytes);

    if (Any_Traits_() == traits) { // avoid wrapping FAny in FAny

        if (static_cast<FAny*>(src)->Valid()) {
            AssignMove(static_cast<FAny*>(src)->InnerAtom());
        }
        else if (_traits) {
            Reset_AssumeInitialized_(_traits->SizeInBytes());
        }

        return;
    }
    else if (_traits) {
        if (traits == *_traits) {
            traits.Move(src, Data_(sizeInBytes));

            return;
        }

        Reset_AssumeInitialized_(_traits->SizeInBytes());
    }
    Assert(not _traits);

    void* const data = Allocate_AssumeNotInitialized_(sizeInBytes);

    _traits.CreateRawCopy_AssumeNotInitialized(traits);
    _traits->ConstructMove(data, src);
}
//----------------------------------------------------------------------------
void FAny::AssignMoveDestroy_(void* src, const ITypeTraits& traits, const size_t sizeInBytes) {
    Assert(src);
    Assert(traits.SizeInBytes() == sizeInBytes);
    Assert(Any_Traits_() != traits);

    if (_traits) {
        if (traits == *_traits) {
            traits.Move(src, Data_(sizeInBytes));
            traits.Destroy(src);

            return;
        }

        Reset_AssumeInitialized_(_traits->SizeInBytes());
    }
    Assert(not _traits);

    void* const data = Allocate_AssumeNotInitialized_(sizeInBytes);

    _traits.CreateRawCopy_AssumeNotInitialized(traits);
    _traits->ConstructMoveDestroy(data, src);
}
//----------------------------------------------------------------------------
void FAny::AssignCopy_AssumeNotInitialized_(const void* src, const ITypeTraits& traits, const size_t sizeInBytes) {
    Assert(src);
    Assert(traits.SizeInBytes() == sizeInBytes);
    Assert(Any_Traits_() != traits);

    void* const data = Allocate_AssumeNotInitialized_(sizeInBytes);

    _traits.CreateRawCopy_AssumeNotInitialized(traits);
    _traits->ConstructCopy(data, src);

    Assert(traits.Equals(data, src));
}
//----------------------------------------------------------------------------
void FAny::AssignMove_AssumeNotInitialized_(void* src, const ITypeTraits& traits, const size_t sizeInBytes) {
    Assert(src);
    Assert(traits.SizeInBytes() == sizeInBytes);
    Assert(Any_Traits_() != traits);

    void* const data = Allocate_AssumeNotInitialized_(sizeInBytes);

    _traits.CreateRawCopy_AssumeNotInitialized(traits);
    _traits->ConstructMove(data, src);
}
//----------------------------------------------------------------------------
void FAny::AssignMoveDestroy_AssumeNotInitialized_(void* src, const ITypeTraits& traits, const size_t sizeInBytes) {
    Assert(src);
    Assert(traits.SizeInBytes() == sizeInBytes);
    Assert(Any_Traits_() != traits);

    void* const data = Allocate_AssumeNotInitialized_(sizeInBytes);

    _traits.CreateRawCopy_AssumeNotInitialized(traits);
    _traits->ConstructMoveDestroy(data, src);
}
//----------------------------------------------------------------------------
void* FAny::Allocate_AssumeNotInitialized_(const size_t sizeInBytes) {
    Assert(sizeInBytes);

    if (Any_FitInSitu_(sizeInBytes)) {
        return (&_inSitu);
    }
    else {
        _externalBlock.SizeInBytes = malloc_snap_size(sizeInBytes);
        _externalBlock.Ptr = PPE::malloc(_externalBlock.SizeInBytes);

#if USE_PPE_MEMORYDOMAINS
        MEMORYDOMAIN_TRACKING_DATA(Any).Allocate(_externalBlock.SizeInBytes, _externalBlock.SizeInBytes);
#endif

        return _externalBlock.Ptr;
    }
}
//----------------------------------------------------------------------------FPlatformMemory::Memset
void FAny::Reset_AssumeInitialized_(const size_t sizeInBytes) {
    Assert(_traits);
    Assert(_traits->SizeInBytes() == sizeInBytes);

    if (Any_FitInSitu_(sizeInBytes)) {
        _traits->Destroy(&_inSitu);
    }
    else {
        Assert(sizeInBytes <= _externalBlock.SizeInBytes);

        _traits->Destroy(_externalBlock.Ptr);

#if USE_PPE_MEMORYDOMAINS
        MEMORYDOMAIN_TRACKING_DATA(Any).Deallocate(_externalBlock.SizeInBytes, _externalBlock.SizeInBytes);
#endif

        PPE::free(_externalBlock.Ptr);
    }

    _traits = PTypeTraits(); // don't want to call the dtor
    ONLY_IF_ASSERT(FPlatformMemory::Memset(&_inSitu, 0xDD, GInSituSize));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void AssignCopy(FAny* dst, const void* src, const ITypeTraits& traits) {
    dst->AssignCopy(src, traits);
}
//----------------------------------------------------------------------------
void AssignMove(FAny* dst, void* src, const ITypeTraits& traits) {
    dst->AssignMove(src, traits);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE

