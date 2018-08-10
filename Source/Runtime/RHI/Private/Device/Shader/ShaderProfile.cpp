#include "stdafx.h"

#include "ShaderProfile.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView ShaderProfileTypeToCStr(EShaderProfileType profile) {
    switch (profile)
    {
    case PPE::Graphics::EShaderProfileType::ShaderModel5:
        return MakeStringView("ShaderModel5");
    case PPE::Graphics::EShaderProfileType::ShaderModel4_1:
        return MakeStringView("ShaderModel4_1");
    case PPE::Graphics::EShaderProfileType::ShaderModel4:
        return MakeStringView("ShaderModel4");
    case PPE::Graphics::EShaderProfileType::ShaderModel3:
        return MakeStringView("ShaderModel3");
    }
    AssertNotImplemented();
    return FStringView();
}
//----------------------------------------------------------------------------
namespace {
    static constexpr EShaderProgramType GShaderProgramTypes[size_t(EShaderProgramType::__Count)] = {
        EShaderProgramType::Vertex,
        EShaderProgramType::Hull,
        EShaderProgramType::Domain,
        EShaderProgramType::Pixel,
        EShaderProgramType::Geometry,
        EShaderProgramType::Compute,
    };
}
TMemoryView<const EShaderProgramType> EachShaderProgramType() {
    return MakeView(GShaderProgramTypes);
}
//----------------------------------------------------------------------------
FStringView ShaderProgramTypeToCStr(EShaderProgramType program) {
    switch (program)
    {
    case PPE::Graphics::EShaderProgramType::Vertex:
        return MakeStringView("Vertex");
    case PPE::Graphics::EShaderProgramType::Hull:
        return MakeStringView("Hull");
    case PPE::Graphics::EShaderProgramType::Domain:
        return MakeStringView("Domain");
    case PPE::Graphics::EShaderProgramType::Pixel:
        return MakeStringView("Pixel");
    case PPE::Graphics::EShaderProgramType::Geometry:
        return MakeStringView("Geometry");
    case PPE::Graphics::EShaderProgramType::Compute:
        return MakeStringView("Compute");
    default:
        AssertNotImplemented();
    }
    return FStringView();
}
//----------------------------------------------------------------------------
FStringView ShaderProgramTypeToEntryPoint(EShaderProgramType program) {
    switch (program)
    {
    case PPE::Graphics::EShaderProgramType::Vertex:
        return MakeStringView("vmain");
    case PPE::Graphics::EShaderProgramType::Hull:
        return MakeStringView("hmain");
    case PPE::Graphics::EShaderProgramType::Domain:
        return MakeStringView("dmain");
    case PPE::Graphics::EShaderProgramType::Pixel:
        return MakeStringView("pmain");
    case PPE::Graphics::EShaderProgramType::Geometry:
        return MakeStringView("gmain");
    case PPE::Graphics::EShaderProgramType::Compute:
        return MakeStringView("cmain");
    default:
        AssertNotImplemented();
    }
    return FStringView();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
