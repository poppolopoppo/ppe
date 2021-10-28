#include "stdafx.h"

#include "RHI_fwd.h"

#if USE_PPE_RHIDEBUG

#include "RHIApi.h"

#include "Diagnostic/Logger.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
inline bool TestVertexInput_(const FVertexInputState& state, const FVertexID& id, u32 requiredIndex) {
    const auto it = state.Vertices.find(id);
    if (state.Vertices.end() == it)
        return false;

    return (it->second.Index == requiredIndex);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NO_INLINE static void Test_VertexInput1_() {
    struct FVertex1 {
        float3 Position;
        short2n TexCoord;
        FRgba8u Color;
    };

    const TFixedSizeStack<FVertexAttribute, 16> attribs{{
        {FVertexID("Position"), 2, EVertexFormat::Float3},
        {FVertexID("TexCoord"), 0, EVertexFormat::Float2},
        {FVertexID("Color"), 1, EVertexFormat::Float4},
    }};

    FVertexInputState vertexInput;
    vertexInput.Bind(FVertexBufferID{}, sizeof(FVertex1));

    vertexInput.Add(FVertexID("Position"), &FVertex1::Position);
    vertexInput.Add(FVertexID("TexCoord"), &FVertex1::TexCoord);
    vertexInput.Add(FVertexID("Color"), &FVertex1::Color);

    VerifyRelease( vertexInput.CopyAttributes(attribs) );

    AssertRelease( TestVertexInput_(vertexInput, FVertexID("Position"), 2) );
    AssertRelease( TestVertexInput_(vertexInput, FVertexID("TexCoord"), 0) );
    AssertRelease( TestVertexInput_(vertexInput, FVertexID("Color"), 1) );
}
//----------------------------------------------------------------------------
NO_INLINE static void Test_VertexInput2_() {
    struct FVertex1 {
        float3 Position;
        short2n TexCoord;
        FRgba8u Color;
    };

    const TFixedSizeStack<FVertexAttribute, 16> attribs{{
        {FVertexID("Position1"), 2, EVertexFormat::Float3},
        {FVertexID("TexCoord1"), 0, EVertexFormat::Float2},
        {FVertexID("Color1"), 1, EVertexFormat::Float4},
        {FVertexID("Position2"), 5, EVertexFormat::Float3},
        {FVertexID("TexCoord2"), 4, EVertexFormat::Float2},
        {FVertexID("Color2"), 3, EVertexFormat::Float4},
    }};

    FVertexInputState vertexInput;
    vertexInput.Bind(FVertexBufferID("frame1"), sizeof(FVertex1));
    vertexInput.Bind(FVertexBufferID("frame2"), sizeof(FVertex1));

    vertexInput.Add(FVertexID("Position1"), &FVertex1::Position, FVertexBufferID("frame1"));
    vertexInput.Add(FVertexID("TexCoord1"), &FVertex1::TexCoord, FVertexBufferID("frame1"));
    vertexInput.Add(FVertexID("Color1"), &FVertex1::Color, FVertexBufferID("frame1"));

    vertexInput.Add(FVertexID("Position2"), &FVertex1::Position, FVertexBufferID("frame2"));
    vertexInput.Add(FVertexID("TexCoord2"), &FVertex1::TexCoord, FVertexBufferID("frame2"));
    vertexInput.Add(FVertexID("Color2"), &FVertex1::Color, FVertexBufferID("frame2"));

    VerifyRelease( vertexInput.CopyAttributes(attribs) );

    AssertRelease( TestVertexInput_(vertexInput, FVertexID("Position1"), 2) );
    AssertRelease( TestVertexInput_(vertexInput, FVertexID("TexCoord1"), 0) );
    AssertRelease( TestVertexInput_(vertexInput, FVertexID("Color1"), 1) );

    AssertRelease( TestVertexInput_(vertexInput, FVertexID("Position2"), 5) );
    AssertRelease( TestVertexInput_(vertexInput, FVertexID("TexCoord2"), 4) );
    AssertRelease( TestVertexInput_(vertexInput, FVertexID("Color2"), 3) );
}
//----------------------------------------------------------------------------
void UnitTest_VertexInput() {
    Test_VertexInput1_();
    Test_VertexInput2_();

    LOG(RHI, Info, L"UnitTest_VertexInput [PASSED]");
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!USE_PPE_RHIDEBUG
