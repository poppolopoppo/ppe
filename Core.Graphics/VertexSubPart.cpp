#include "stdafx.h"

#include "VertexDeclaration.h"

#include "Core/AlignedStorage.h"
#include "Core/MemoryView.h"
#include "Core/Format.h"
#include "Core/Stream.h"

#include "DeviceEncapsulator.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const char *VertexSubPartSemanticToCStr(VertexSubPartSemantic semantic) {
    switch (semantic)
    {
    case Core::Graphics::VertexSubPartSemantic::Position:
        return "Position";
    case Core::Graphics::VertexSubPartSemantic::TexCoord:
        return "TexCoord";
    case Core::Graphics::VertexSubPartSemantic::Color:
        return "Color";
    case Core::Graphics::VertexSubPartSemantic::Normal:
        return "Normal";
    case Core::Graphics::VertexSubPartSemantic::Tangent:
        return "Tangent";
    case Core::Graphics::VertexSubPartSemantic::Binormal:
        return "Binormal";
    }
    AssertNotImplemented();
    return nullptr;
}
//----------------------------------------------------------------------------
const char *VertexSubPartFormatToCStr(VertexSubPartFormat format) {
    switch (format)
    {
    case Core::Graphics::VertexSubPartFormat::Float:
        return "Float";
    case Core::Graphics::VertexSubPartFormat::Float2:
        return "Float2";
    case Core::Graphics::VertexSubPartFormat::Float3:
        return "Float3";
    case Core::Graphics::VertexSubPartFormat::Float4:
        return "Float4";
    case Core::Graphics::VertexSubPartFormat::Byte:
        return "Byte";
    case Core::Graphics::VertexSubPartFormat::Byte2:
        return "Byte2";
    case Core::Graphics::VertexSubPartFormat::Byte4:
        return "Byte4";
    case Core::Graphics::VertexSubPartFormat::UByte:
        return "UByte";
    case Core::Graphics::VertexSubPartFormat::UByte2:
        return "UByte2";
    case Core::Graphics::VertexSubPartFormat::UByte4:
        return "UByte4";
    case Core::Graphics::VertexSubPartFormat::Short:
        return "Short";
    case Core::Graphics::VertexSubPartFormat::Short2:
        return "Short2";
    case Core::Graphics::VertexSubPartFormat::Short4:
        return "Short4";
    case Core::Graphics::VertexSubPartFormat::UShort:
        return "UShort";
    case Core::Graphics::VertexSubPartFormat::UShort2:
        return "UShort2";
    case Core::Graphics::VertexSubPartFormat::UShort4:
        return "UShort4";
    case Core::Graphics::VertexSubPartFormat::Word:
        return "Word";
    case Core::Graphics::VertexSubPartFormat::Word2:
        return "Word2";
    case Core::Graphics::VertexSubPartFormat::Word3:
        return "Word3";
    case Core::Graphics::VertexSubPartFormat::Word4:
        return "Word4";
    case Core::Graphics::VertexSubPartFormat::UWord:
        return "UWord";
    case Core::Graphics::VertexSubPartFormat::UWord2:
        return "UWord2";
    case Core::Graphics::VertexSubPartFormat::UWord3:
        return "UWord3";
    case Core::Graphics::VertexSubPartFormat::UWord4:
        return "UWord4";
    case Core::Graphics::VertexSubPartFormat::Half:
        return "Half";
    case Core::Graphics::VertexSubPartFormat::Half2:
        return "Half2";
    case Core::Graphics::VertexSubPartFormat::Half4:
        return "Half4";
    case Core::Graphics::VertexSubPartFormat::Byte2N:
        return "Byte2N";
    case Core::Graphics::VertexSubPartFormat::Byte4N:
        return "Byte4N";
    case Core::Graphics::VertexSubPartFormat::UByte2N:
        return "UByte2N";
    case Core::Graphics::VertexSubPartFormat::UByte4N:
        return "UByte4N";
    case Core::Graphics::VertexSubPartFormat::Short2N:
        return "Short2N";
    case Core::Graphics::VertexSubPartFormat::Short4N:
        return "Short4N";
    case Core::Graphics::VertexSubPartFormat::UShort2N:
        return "UShort2N";
    case Core::Graphics::VertexSubPartFormat::UShort4N:
        return "UShort4N";
    case Core::Graphics::VertexSubPartFormat::UX10Y10Z10W2N:
        return "UX10Y10Z10W2N";
    }
    AssertNotImplemented();
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
