#include "stdafx.h"

#include "Any.h"

#include "Core/IO/FileSystem.h"
#include "Core/IO/String.h"
#include "Core/Maths/ScalarMatrix.h"
#include "Core/Maths/ScalarVector.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// checks for padding
STATIC_ASSERT(sizeof(FAny) == FAny::GSmallBufferSize + sizeof(intptr_t));
//----------------------------------------------------------------------------
// checks that 80% of wrapped types are fitting in small buffer
namespace {
constexpr size_t GNumSupportedTypes = 0
#define DECL_RTTI_NATIVETYPE_SUPPORTED(_Name, T, _TypeId) + 1
FOREACH_RTTI_NATIVETYPES(DECL_RTTI_NATIVETYPE_SUPPORTED)
#undef DECL_RTTI_NATIVETYPE_SUPPORTED
    ;
constexpr size_t GNumSupportedTypesFittingInSmallBuffer = 0
#define DECL_RTTI_NATIVETYPE_SUPPORTED(_Name, T, _TypeId) + (sizeof(T) <= FAny::GSmallBufferSize ? 1 : 0)
FOREACH_RTTI_NATIVETYPES(DECL_RTTI_NATIVETYPE_SUPPORTED)
#undef DECL_RTTI_NATIVETYPE_SUPPORTED
    ;
STATIC_ASSERT(GNumSupportedTypesFittingInSmallBuffer > 0.8f * GNumSupportedTypes);
} //!namespace
//----------------------------------------------------------------------------
FAny::FAny() NOEXCEPT {
    ONLY_IF_ASSERT(::memset(std::addressof(_smallBuffer), 0xDD, sizeof(_smallBuffer)));
}
//----------------------------------------------------------------------------
FAny::FAny(const PTypeTraits& traits) NOEXCEPT {
    FAtom self(Allocate_(PTypeTraits(traits)));
    _traits->Create(self);
    Assert(_traits->IsDefaultValue(self));
}
//----------------------------------------------------------------------------
void* FAny::Data() {
    if (Likely(_traits))
        return (_traits->SizeInBytes() <= GSmallBufferSize ? std::addressof(_smallBuffer) : _externalStorage);
    else
        return nullptr;
}
//----------------------------------------------------------------------------
void FAny::Reset() {
    if (Likely(_traits)) {
        _traits->Destroy(InnerAtom());

        if (_traits->SizeInBytes() > GSmallBufferSize)
            _traits->Deallocate(_externalStorage);

        _traits.Destroy();

        ONLY_IF_ASSERT(::memset(std::addressof(_smallBuffer), 0xDD, sizeof(_smallBuffer)));
    }
    Assert((void*)CODE3264(0xDDDDDDDDul, 0xDDDDDDDDDDDDDDDDull));
}
//----------------------------------------------------------------------------
FAtom FAny::Reset(const PTypeTraits& traits) {
    Reset();

    if (traits == MakeTraits<FAny>()) {
        return RTTI::MakeAtom(this);
    }
    else {
        FAtom self(Allocate_(PTypeTraits(traits)));
        _traits->Create(self);
        Assert(_traits->IsDefaultValue(self));
        return InnerAtom();
    }
}
//----------------------------------------------------------------------------
void FAny::AssignCopy(const FAtom& atom) {
    Assert(atom);

    if (atom.Traits() == _traits) {
        _traits->Copy(atom, InnerAtom());
    }
    else {
        Reset();
        FAtom self(Allocate_(PTypeTraits(atom.Traits())));
        _traits->CreateCopy(self, atom);
    }
}
//----------------------------------------------------------------------------
void FAny::AssignMove(const FAtom& atom) {
    Assert(atom);

    if (atom.Traits() == _traits) {
        _traits->Move(atom, InnerAtom());
    }
    else {
        Reset();
        FAtom self(Allocate_(PTypeTraits(atom.Traits())));
        _traits->CreateMove(self, atom);
    }
}
//----------------------------------------------------------------------------
bool FAny::Equals(const FAny& other) const {
    return ((_traits == other._traits)
        ? (not _traits || _traits->Equals(InnerAtom(), other.InnerAtom()) )
        : false );
}
//----------------------------------------------------------------------------
hash_t FAny::HashValue() const {
    return ((_traits)
        ? _traits->HashValue(InnerAtom())
        : hash_t(0) );
}
//----------------------------------------------------------------------------
FAtom FAny::Allocate_(PTypeTraits&& traits) {
    Assert(traits);
    Assert(MakeTraits<FAny>() != traits); // oh boy, stop this meta infinite loop
    Assert(not _traits);

    _traits = std::move(traits);

    if (Likely(_traits->SizeInBytes() <= GSmallBufferSize))
        return FAtom(std::addressof(_smallBuffer), _traits);
    else
        return FAtom(_externalStorage = _traits->Allocate(), _traits);
}
//----------------------------------------------------------------------------
void FAny::CopyFrom_(const FAny& other) {
    if (_traits == other._traits) {
        if (_traits)
            _traits->Copy(other.InnerAtom(), InnerAtom());
        else
            NOOP(); // both invalid nothing to do
    }
    else {
        Reset();
        if (other._traits)
            other._traits->CreateCopy(Allocate_(PTypeTraits(other._traits)), other.InnerAtom());
    }
    Assert(Equals(other));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core

