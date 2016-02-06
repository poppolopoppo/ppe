#include "stdafx.h"

#include "VertexDeclaration.h"

#include "Core/IO/Format.h"
#include "Core/IO/Stream.h"
#include "Core/Memory/AlignedStorage.h"
#include "Core/Memory/MemoryView.h"

#include "Device/DeviceEncapsulator.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(sizeof(AbstractVertexSubPart) == sizeof(TypedVertexSubPart<VertexSubPartFormat::Byte  >) );
STATIC_ASSERT(sizeof(AbstractVertexSubPart) == sizeof(TypedVertexSubPart<VertexSubPartFormat::Short >) );
STATIC_ASSERT(sizeof(AbstractVertexSubPart) == sizeof(TypedVertexSubPart<VertexSubPartFormat::Float3>) );
STATIC_ASSERT(sizeof(AbstractVertexSubPart) == sizeof(TypedVertexSubPart<VertexSubPartFormat::Half4 >) );
//----------------------------------------------------------------------------
StringSlice VertexSubPartSemanticToCStr(VertexSubPartSemantic semantic) {
    switch (semantic)
    {
    case Core::Graphics::VertexSubPartSemantic::Position:
        return MakeStringSlice("Position");
    case Core::Graphics::VertexSubPartSemantic::TexCoord:
        return MakeStringSlice("TexCoord");
    case Core::Graphics::VertexSubPartSemantic::Color:
        return MakeStringSlice("Color");
    case Core::Graphics::VertexSubPartSemantic::Normal:
        return MakeStringSlice("Normal");
    case Core::Graphics::VertexSubPartSemantic::Tangent:
        return MakeStringSlice("Tangent");
    case Core::Graphics::VertexSubPartSemantic::Binormal:
        return MakeStringSlice("Binormal");
    }
    AssertNotImplemented();
    return StringSlice();
}
//----------------------------------------------------------------------------
StringSlice VertexSubPartFormatToCStr(VertexSubPartFormat format) {
    switch (format)
    {
    case Core::Graphics::VertexSubPartFormat::Float:
        return MakeStringSlice("Float");
    case Core::Graphics::VertexSubPartFormat::Float2:
        return MakeStringSlice("Float2");
    case Core::Graphics::VertexSubPartFormat::Float3:
        return MakeStringSlice("Float3");
    case Core::Graphics::VertexSubPartFormat::Float4:
        return MakeStringSlice("Float4");
    case Core::Graphics::VertexSubPartFormat::Byte:
        return MakeStringSlice("Byte");
    case Core::Graphics::VertexSubPartFormat::Byte2:
        return MakeStringSlice("Byte2");
    case Core::Graphics::VertexSubPartFormat::Byte4:
        return MakeStringSlice("Byte4");
    case Core::Graphics::VertexSubPartFormat::UByte:
        return MakeStringSlice("UByte");
    case Core::Graphics::VertexSubPartFormat::UByte2:
        return MakeStringSlice("UByte2");
    case Core::Graphics::VertexSubPartFormat::UByte4:
        return MakeStringSlice("UByte4");
    case Core::Graphics::VertexSubPartFormat::Short:
        return MakeStringSlice("Short");
    case Core::Graphics::VertexSubPartFormat::Short2:
        return MakeStringSlice("Short2");
    case Core::Graphics::VertexSubPartFormat::Short4:
        return MakeStringSlice("Short4");
    case Core::Graphics::VertexSubPartFormat::UShort:
        return MakeStringSlice("UShort");
    case Core::Graphics::VertexSubPartFormat::UShort2:
        return MakeStringSlice("UShort2");
    case Core::Graphics::VertexSubPartFormat::UShort4:
        return MakeStringSlice("UShort4");
    case Core::Graphics::VertexSubPartFormat::Word:
        return MakeStringSlice("Word");
    case Core::Graphics::VertexSubPartFormat::Word2:
        return MakeStringSlice("Word2");
    case Core::Graphics::VertexSubPartFormat::Word3:
        return MakeStringSlice("Word3");
    case Core::Graphics::VertexSubPartFormat::Word4:
        return MakeStringSlice("Word4");
    case Core::Graphics::VertexSubPartFormat::UWord:
        return MakeStringSlice("UWord");
    case Core::Graphics::VertexSubPartFormat::UWord2:
        return MakeStringSlice("UWord2");
    case Core::Graphics::VertexSubPartFormat::UWord3:
        return MakeStringSlice("UWord3");
    case Core::Graphics::VertexSubPartFormat::UWord4:
        return MakeStringSlice("UWord4");
    case Core::Graphics::VertexSubPartFormat::Half:
        return MakeStringSlice("Half");
    case Core::Graphics::VertexSubPartFormat::Half2:
        return MakeStringSlice("Half2");
    case Core::Graphics::VertexSubPartFormat::Half4:
        return MakeStringSlice("Half4");
    case Core::Graphics::VertexSubPartFormat::Byte2N:
        return MakeStringSlice("Byte2N");
    case Core::Graphics::VertexSubPartFormat::Byte4N:
        return MakeStringSlice("Byte4N");
    case Core::Graphics::VertexSubPartFormat::UByte2N:
        return MakeStringSlice("UByte2N");
    case Core::Graphics::VertexSubPartFormat::UByte4N:
        return MakeStringSlice("UByte4N");
    case Core::Graphics::VertexSubPartFormat::Short2N:
        return MakeStringSlice("Short2N");
    case Core::Graphics::VertexSubPartFormat::Short4N:
        return MakeStringSlice("Short4N");
    case Core::Graphics::VertexSubPartFormat::UShort2N:
        return MakeStringSlice("UShort2N");
    case Core::Graphics::VertexSubPartFormat::UShort4N:
        return MakeStringSlice("UShort4N");
    case Core::Graphics::VertexSubPartFormat::UX10Y10Z10W2N:
        return MakeStringSlice("UX10Y10Z10W2N");
    }
    AssertNotImplemented();
    return StringSlice();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
