// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Diagnostic/Logger.h"

#include "Container/RawStorage.h"
#include "IO/String.h"
#include "Memory/SharedBuffer.h"

namespace PPE {
namespace Test {
LOG_CATEGORY(, Test_Memory)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static NO_INLINE void Test_SharedBuffer_() {
    PPE_LOG(Test_Memory, Emphasis, "testing FSharedBuffer");

    FString dynamicString = "wow, such long text it is sure to be allocated";
    FStringView source = dynamicString;

    FSharedBuffer notOwnedImmutable = FSharedBuffer::MakeView(MakeRawConstView(source));
    AssertRelease_NoAssume(notOwnedImmutable.IsValid());
    AssertRelease_NoAssume(not notOwnedImmutable.IsOwned());
    AssertRelease_NoAssume(notOwnedImmutable.IsImmutable());
    AssertRelease_NoAssume(notOwnedImmutable.IsMaterialized());
    AssertRelease_NoAssume(notOwnedImmutable.MakeView() == MakeRawConstView(source));

    FSharedBuffer ownedShared = notOwnedImmutable.MakeOwned();
    AssertRelease_NoAssume(ownedShared.IsValid());
    AssertRelease_NoAssume(ownedShared.IsOwned());
    AssertRelease_NoAssume(not ownedShared.IsImmutable());
    AssertRelease_NoAssume(ownedShared.IsMaterialized());
    AssertRelease_NoAssume(ownedShared.MakeView() != MakeRawConstView(source));
    AssertRelease_NoAssume(Memcmp(ownedShared.MakeView(), MakeRawConstView(source)) == 0);

    AssertRelease_NoAssume(notOwnedImmutable.IsValid());
    AssertRelease_NoAssume(not notOwnedImmutable.IsOwned());
    AssertRelease_NoAssume(notOwnedImmutable.IsImmutable());
    AssertRelease_NoAssume(notOwnedImmutable.IsMaterialized());
    AssertRelease_NoAssume(notOwnedImmutable.MakeView() == MakeRawConstView(source));

    FWeakSharedBuffer ownedWeak = ownedShared;
    {
        const FSharedBuffer ownedPinned = ownedWeak.Pin();
        AssertRelease_NoAssume(ownedPinned.IsValid());
        AssertRelease_NoAssume(ownedPinned == ownedShared);
        AssertRelease_NoAssume(ownedPinned.IsOwned());
        AssertRelease_NoAssume(not ownedPinned.IsImmutable());
        AssertRelease_NoAssume(ownedPinned.IsMaterialized());

        AssertRelease_NoAssume(notOwnedImmutable.IsValid());
        AssertRelease_NoAssume(not notOwnedImmutable.IsOwned());
        AssertRelease_NoAssume(notOwnedImmutable.IsImmutable());
        AssertRelease_NoAssume(notOwnedImmutable.IsMaterialized());
        AssertRelease_NoAssume(notOwnedImmutable.MakeView() == MakeRawConstView(source));
    }

    FUniqueBuffer ownedUnique = ownedShared.MoveToUnique();
    AssertRelease_NoAssume(ownedUnique.IsValid());
    AssertRelease_NoAssume(ownedUnique.IsOwned());
    AssertRelease_NoAssume(not ownedUnique.IsImmutable());
    AssertRelease_NoAssume(ownedUnique.IsMaterialized());
    AssertRelease_NoAssume(MakeRawConstView(ownedUnique.MakeView()) != MakeRawConstView(source));
    AssertRelease_NoAssume(Memcmp(MakeRawConstView(ownedUnique.MakeView()), MakeRawConstView(source)) == 0);

    AssertRelease_NoAssume(not ownedShared.IsValid());
    AssertRelease_NoAssume(ownedShared.IsImmutable());
    AssertRelease_NoAssume(ownedShared.IsMaterialized());

    AssertRelease_NoAssume(ownedWeak.Pin() == ownedUnique);

    AssertRelease_NoAssume(notOwnedImmutable.IsValid());
    AssertRelease_NoAssume(not notOwnedImmutable.IsOwned());
    AssertRelease_NoAssume(notOwnedImmutable.IsImmutable());
    AssertRelease_NoAssume(notOwnedImmutable.IsMaterialized());
    AssertRelease_NoAssume(notOwnedImmutable.MakeView() == MakeRawConstView(source));

    ownedUnique.Reset();

    AssertRelease_NoAssume(not ownedWeak.Pin().IsValid());

    AssertRelease_NoAssume(not ownedUnique.IsValid());
    AssertRelease_NoAssume(ownedUnique.IsOwned());
    AssertRelease_NoAssume(ownedUnique.IsImmutable());
    AssertRelease_NoAssume(ownedUnique.IsMaterialized());
    AssertRelease_NoAssume(ownedUnique.MakeView() == Default);
}
//----------------------------------------------------------------------------
void Test_Memory() {
    PPE_DEBUG_NAMEDSCOPE("Test_Memory");

    Test_SharedBuffer_();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
} //!namespace Test
