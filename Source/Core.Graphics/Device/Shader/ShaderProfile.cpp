#include "stdafx.h"

#include "ShaderProfile.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
StringView ShaderProfileTypeToCStr(ShaderProfileType profile) {
    switch (profile)
    {
    case Core::Graphics::ShaderProfileType::ShaderModel5:
        return MakeStringView("ShaderModel5");
    case Core::Graphics::ShaderProfileType::ShaderModel4_1:
        return MakeStringView("ShaderModel4_1");
    case Core::Graphics::ShaderProfileType::ShaderModel4:
        return MakeStringView("ShaderModel4");
    case Core::Graphics::ShaderProfileType::ShaderModel3:
        return MakeStringView("ShaderModel3");
    }
    AssertNotImplemented();
    return StringView();
}
//----------------------------------------------------------------------------
namespace {
    static const ShaderProgramType gShaderProgramTypes[size_t(ShaderProgramType::__Count)] = {
        ShaderProgramType::Vertex,
        ShaderProgramType::Hull,
        ShaderProgramType::Domain,
        ShaderProgramType::Pixel,
        ShaderProgramType::Geometry,
        ShaderProgramType::Compute,
    };
}
MemoryView<const ShaderProgramType> EachShaderProgramType() {
    return MakeView(gShaderProgramTypes);
}
//----------------------------------------------------------------------------
StringView ShaderProgramTypeToCStr(ShaderProgramType program) {
    switch (program)
    {
    case Core::Graphics::ShaderProgramType::Vertex:
        return MakeStringView("Vertex");
    case Core::Graphics::ShaderProgramType::Hull:
        return MakeStringView("Hull");
    case Core::Graphics::ShaderProgramType::Domain:
        return MakeStringView("Domain");
    case Core::Graphics::ShaderProgramType::Pixel:
        return MakeStringView("Pixel");
    case Core::Graphics::ShaderProgramType::Geometry:
        return MakeStringView("Geometry");
    case Core::Graphics::ShaderProgramType::Compute:
        return MakeStringView("Compute");
    default:
        AssertNotImplemented();
    }
    return StringView();
}
//----------------------------------------------------------------------------
StringView ShaderProgramTypeToEntryPoint(ShaderProgramType program) {
    switch (program)
    {
    case Core::Graphics::ShaderProgramType::Vertex:
        return MakeStringView("vmain");
    case Core::Graphics::ShaderProgramType::Hull:
        return MakeStringView("hmain");
    case Core::Graphics::ShaderProgramType::Domain:
        return MakeStringView("dmain");
    case Core::Graphics::ShaderProgramType::Pixel:
        return MakeStringView("pmain");
    case Core::Graphics::ShaderProgramType::Geometry:
        return MakeStringView("gmain");
    case Core::Graphics::ShaderProgramType::Compute:
        return MakeStringView("cmain");
    default:
        AssertNotImplemented();
    }
    return StringView();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
