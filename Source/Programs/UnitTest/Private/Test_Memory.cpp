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
    PPE_LOG_CHECKVOID(Test_Memory, notOwnedImmutable.IsValid());
    PPE_LOG_CHECKVOID(Test_Memory, not notOwnedImmutable.IsOwned());
    PPE_LOG_CHECKVOID(Test_Memory, notOwnedImmutable.IsImmutable());
    PPE_LOG_CHECKVOID(Test_Memory, notOwnedImmutable.IsMaterialized());
    PPE_LOG_CHECKVOID(Test_Memory, notOwnedImmutable.MakeView() == MakeRawConstView(source));

    FSharedBuffer ownedShared = notOwnedImmutable.MakeOwned();
    PPE_LOG_CHECKVOID(Test_Memory, ownedShared.IsValid());
    PPE_LOG_CHECKVOID(Test_Memory, ownedShared.IsOwned());
    PPE_LOG_CHECKVOID(Test_Memory, not ownedShared.IsImmutable());
    PPE_LOG_CHECKVOID(Test_Memory, ownedShared.IsMaterialized());
    PPE_LOG_CHECKVOID(Test_Memory, ownedShared.MakeView() != MakeRawConstView(source));
    PPE_LOG_CHECKVOID(Test_Memory, Memcmp(ownedShared.MakeView(), MakeRawConstView(source)) == 0);

    PPE_LOG_CHECKVOID(Test_Memory, notOwnedImmutable.IsValid());
    PPE_LOG_CHECKVOID(Test_Memory, not notOwnedImmutable.IsOwned());
    PPE_LOG_CHECKVOID(Test_Memory, notOwnedImmutable.IsImmutable());
    PPE_LOG_CHECKVOID(Test_Memory, notOwnedImmutable.IsMaterialized());
    PPE_LOG_CHECKVOID(Test_Memory, notOwnedImmutable.MakeView() == MakeRawConstView(source));

    FWeakSharedBuffer ownedWeak = ownedShared;
    {
        const FSharedBuffer ownedPinned = ownedWeak.Pin();
        PPE_LOG_CHECKVOID(Test_Memory, ownedPinned.IsValid());
        PPE_LOG_CHECKVOID(Test_Memory, ownedPinned == ownedShared);
        PPE_LOG_CHECKVOID(Test_Memory, ownedPinned.IsOwned());
        PPE_LOG_CHECKVOID(Test_Memory, not ownedPinned.IsImmutable());
        PPE_LOG_CHECKVOID(Test_Memory, ownedPinned.IsMaterialized());

        PPE_LOG_CHECKVOID(Test_Memory, notOwnedImmutable.IsValid());
        PPE_LOG_CHECKVOID(Test_Memory, not notOwnedImmutable.IsOwned());
        PPE_LOG_CHECKVOID(Test_Memory, notOwnedImmutable.IsImmutable());
        PPE_LOG_CHECKVOID(Test_Memory, notOwnedImmutable.IsMaterialized());
        PPE_LOG_CHECKVOID(Test_Memory, notOwnedImmutable.MakeView() == MakeRawConstView(source));
    }

    FUniqueBuffer ownedUnique = ownedShared.MoveToUnique();
    PPE_LOG_CHECKVOID(Test_Memory, ownedUnique.IsValid());
    PPE_LOG_CHECKVOID(Test_Memory, ownedUnique.IsOwned());
    PPE_LOG_CHECKVOID(Test_Memory, not ownedUnique.IsImmutable());
    PPE_LOG_CHECKVOID(Test_Memory, ownedUnique.IsMaterialized());
    PPE_LOG_CHECKVOID(Test_Memory, MakeRawConstView(ownedUnique.MakeView()) != MakeRawConstView(source));
    PPE_LOG_CHECKVOID(Test_Memory, Memcmp(MakeRawConstView(ownedUnique.MakeView()), MakeRawConstView(source)) == 0);

    PPE_LOG_CHECKVOID(Test_Memory, not ownedShared.IsValid());
    PPE_LOG_CHECKVOID(Test_Memory, ownedShared.IsImmutable());
    PPE_LOG_CHECKVOID(Test_Memory, ownedShared.IsMaterialized());

    PPE_LOG_CHECKVOID(Test_Memory, ownedWeak.Pin() == ownedUnique);

    PPE_LOG_CHECKVOID(Test_Memory, notOwnedImmutable.IsValid());
    PPE_LOG_CHECKVOID(Test_Memory, not notOwnedImmutable.IsOwned());
    PPE_LOG_CHECKVOID(Test_Memory, notOwnedImmutable.IsImmutable());
    PPE_LOG_CHECKVOID(Test_Memory, notOwnedImmutable.IsMaterialized());
    PPE_LOG_CHECKVOID(Test_Memory, notOwnedImmutable.MakeView() == MakeRawConstView(source));

    ownedUnique.Reset();

    PPE_LOG_CHECKVOID(Test_Memory, not ownedWeak.Pin().IsValid());

    PPE_LOG_CHECKVOID(Test_Memory, not ownedUnique.IsValid());
    PPE_LOG_CHECKVOID(Test_Memory, ownedUnique.IsOwned());
    PPE_LOG_CHECKVOID(Test_Memory, ownedUnique.IsImmutable());
    PPE_LOG_CHECKVOID(Test_Memory, ownedUnique.IsMaterialized());
    PPE_LOG_CHECKVOID(Test_Memory, ownedUnique.MakeView() == Default);
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
