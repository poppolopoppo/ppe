#include "stdafx.h"

#include "ShaderProfile.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
StringSlice ShaderProfileTypeToCStr(ShaderProfileType profile) {
    switch (profile)
    {
    case Core::Graphics::ShaderProfileType::ShaderModel5:
        return MakeStringSlice("ShaderModel5");
    case Core::Graphics::ShaderProfileType::ShaderModel4_1:
        return MakeStringSlice("ShaderModel4_1");
    case Core::Graphics::ShaderProfileType::ShaderModel4:
        return MakeStringSlice("ShaderModel4");
    case Core::Graphics::ShaderProfileType::ShaderModel3:
        return MakeStringSlice("ShaderModel3");
    }
    AssertNotImplemented();
    return StringSlice();
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
StringSlice ShaderProgramTypeToCStr(ShaderProgramType program) {
    switch (program)
    {
    case Core::Graphics::ShaderProgramType::Vertex:
        return MakeStringSlice("Vertex");
    case Core::Graphics::ShaderProgramType::Hull:
        return MakeStringSlice("Hull");
    case Core::Graphics::ShaderProgramType::Domain:
        return MakeStringSlice("Domain");
    case Core::Graphics::ShaderProgramType::Pixel:
        return MakeStringSlice("Pixel");
    case Core::Graphics::ShaderProgramType::Geometry:
        return MakeStringSlice("Geometry");
    case Core::Graphics::ShaderProgramType::Compute:
        return MakeStringSlice("Compute");
    default:
        AssertNotImplemented();
    }
    return StringSlice();
}
//----------------------------------------------------------------------------
StringSlice ShaderProgramTypeToEntryPoint(ShaderProgramType program) {
    switch (program)
    {
    case Core::Graphics::ShaderProgramType::Vertex:
        return MakeStringSlice("vmain");
    case Core::Graphics::ShaderProgramType::Hull:
        return MakeStringSlice("hmain");
    case Core::Graphics::ShaderProgramType::Domain:
        return MakeStringSlice("dmain");
    case Core::Graphics::ShaderProgramType::Pixel:
        return MakeStringSlice("pmain");
    case Core::Graphics::ShaderProgramType::Geometry:
        return MakeStringSlice("gmain");
    case Core::Graphics::ShaderProgramType::Compute:
        return MakeStringSlice("cmain");
    default:
        AssertNotImplemented();
    }
    return StringSlice();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core