#include "stdafx.h"

#include "ShaderProfile.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const char *ShaderProfileTypeToCStr(ShaderProfileType profile) {
    switch (profile)
    {
    case Core::Graphics::ShaderProfileType::ShaderModel5:
        return "ShaderModel5";
    case Core::Graphics::ShaderProfileType::ShaderModel4_1:
        return "ShaderModel4_1";
    case Core::Graphics::ShaderProfileType::ShaderModel4:
        return "ShaderModel4";
    case Core::Graphics::ShaderProfileType::ShaderModel3:
        return "ShaderModel3";
    }
    AssertNotImplemented();
    return nullptr;
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
const char *ShaderProgramTypeToCStr(ShaderProgramType program) {
    switch (program)
    {
    case Core::Graphics::ShaderProgramType::Vertex:
        return "Vertex";
    case Core::Graphics::ShaderProgramType::Hull:
        return "Hull";
    case Core::Graphics::ShaderProgramType::Domain:
        return "Domain";
    case Core::Graphics::ShaderProgramType::Pixel:
        return "Pixel";
    case Core::Graphics::ShaderProgramType::Geometry:
        return "Geometry";
    case Core::Graphics::ShaderProgramType::Compute:
        return "Compute";
    default:
        AssertNotImplemented();
    }
    return nullptr;
}
//----------------------------------------------------------------------------
const char *ShaderProgramTypeToEntryPoint(ShaderProgramType program) {
    switch (program)
    {
    case Core::Graphics::ShaderProgramType::Vertex:
        return "vmain";
    case Core::Graphics::ShaderProgramType::Hull:
        return "hmain";
    case Core::Graphics::ShaderProgramType::Domain:
        return "dmain";
    case Core::Graphics::ShaderProgramType::Pixel:
        return "pmain";
    case Core::Graphics::ShaderProgramType::Geometry:
        return "gmain";
    case Core::Graphics::ShaderProgramType::Compute:
        return "cmain";
    default:
        AssertNotImplemented();
    }
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core