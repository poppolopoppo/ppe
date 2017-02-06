#include "stdafx.h"

#include "ShaderProfile.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView ShaderProfileTypeToCStr(EShaderProfileType profile) {
    switch (profile)
    {
    case Core::Graphics::EShaderProfileType::ShaderModel5:
        return MakeStringView("ShaderModel5");
    case Core::Graphics::EShaderProfileType::ShaderModel4_1:
        return MakeStringView("ShaderModel4_1");
    case Core::Graphics::EShaderProfileType::ShaderModel4:
        return MakeStringView("ShaderModel4");
    case Core::Graphics::EShaderProfileType::ShaderModel3:
        return MakeStringView("ShaderModel3");
    }
    AssertNotImplemented();
    return FStringView();
}
//----------------------------------------------------------------------------
namespace {
    static constexpr EShaderProgramType gShaderProgramTypes[size_t(EShaderProgramType::__Count)] = {
        EShaderProgramType::Vertex,
        EShaderProgramType::Hull,
        EShaderProgramType::Domain,
        EShaderProgramType::Pixel,
        EShaderProgramType::Geometry,
        EShaderProgramType::Compute,
    };
}
TMemoryView<const EShaderProgramType> EachShaderProgramType() {
    return MakeView(gShaderProgramTypes);
}
//----------------------------------------------------------------------------
FStringView ShaderProgramTypeToCStr(EShaderProgramType program) {
    switch (program)
    {
    case Core::Graphics::EShaderProgramType::Vertex:
        return MakeStringView("Vertex");
    case Core::Graphics::EShaderProgramType::Hull:
        return MakeStringView("Hull");
    case Core::Graphics::EShaderProgramType::Domain:
        return MakeStringView("Domain");
    case Core::Graphics::EShaderProgramType::Pixel:
        return MakeStringView("Pixel");
    case Core::Graphics::EShaderProgramType::Geometry:
        return MakeStringView("Geometry");
    case Core::Graphics::EShaderProgramType::Compute:
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
    case Core::Graphics::EShaderProgramType::Vertex:
        return MakeStringView("vmain");
    case Core::Graphics::EShaderProgramType::Hull:
        return MakeStringView("hmain");
    case Core::Graphics::EShaderProgramType::Domain:
        return MakeStringView("dmain");
    case Core::Graphics::EShaderProgramType::Pixel:
        return MakeStringView("pmain");
    case Core::Graphics::EShaderProgramType::Geometry:
        return MakeStringView("gmain");
    case Core::Graphics::EShaderProgramType::Compute:
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
} //!namespace Core
