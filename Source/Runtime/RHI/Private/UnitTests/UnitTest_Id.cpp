// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "RHI_fwd.h"

#if USE_PPE_RHIDEBUG

#include "RHIApi.h"

#include "Diagnostic/Logger.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NO_INLINE static void Test_Id1_() {
    constexpr FVertexID id0{ "sdlmkfdslmkflmdskfsdm" };
    constexpr FVertexID id1{ "sdlmkfdslmkflmdskfsdm" };
    constexpr FVertexID id2{ "eoirpeiropezskesdl" };

    AssertRelease(id0.HashValue == id1.HashValue);
    AssertRelease(id1.HashValue != id2.HashValue);

    FString str;
    str += "sdlmkfdslmk";
    str += "flmdskfsdm";
    FVertexID id3{ str };
    AssertRelease(id3.HashValue == id0.HashValue);
}
//----------------------------------------------------------------------------
NO_INLINE static void Test_Id2_() {
    using ID1 = details::TNamedId<100, false>;
    using ID2 = details::TNamedId<100, true>;

    ID1 a{"test"};
    ID2 b{"test"};
    AssertRelease(a == b);
}
//----------------------------------------------------------------------------
NO_INLINE static void Test_Id3_() {
    FVertexBufferID a;
    FVertexBufferID b = Default;
    FVertexBufferID c = ""_vertexbuffer;
    AssertRelease(a == b);
    AssertRelease(a == c);
}
//----------------------------------------------------------------------------
void UnitTest_Id() {
    Test_Id1_();
    Test_Id2_();
    Test_Id3_();

    LOG(RHI, Info, L"UnitTest_Id [PASSED]");
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!USE_PPE_RHIDEBUG
