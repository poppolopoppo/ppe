#include "stdafx.h"

#include "ConstantField.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t ConstantFieldTypeSizeInBytes(ConstantFieldType value) {
    switch (value)
    {
    case Core::Graphics::ConstantFieldType::Bool:
        return sizeof(bool);
    case Core::Graphics::ConstantFieldType::Int:
        return sizeof(i32);
    case Core::Graphics::ConstantFieldType::Int2:
        return sizeof(i32)*2;
    case Core::Graphics::ConstantFieldType::Int3:
        return sizeof(i32)*3;
    case Core::Graphics::ConstantFieldType::Int4:
        return sizeof(i32)*4;
    case Core::Graphics::ConstantFieldType::UInt:
        return sizeof(u32);
    case Core::Graphics::ConstantFieldType::UInt2:
        return sizeof(u32)*2;
    case Core::Graphics::ConstantFieldType::UInt3:
        return sizeof(u32)*3;
    case Core::Graphics::ConstantFieldType::UInt4:
        return sizeof(u32)*4;
    case Core::Graphics::ConstantFieldType::Float:
        return sizeof(float);
    case Core::Graphics::ConstantFieldType::Float2:
        return sizeof(float)*2;
    case Core::Graphics::ConstantFieldType::Float3:
        return sizeof(float)*3;
    case Core::Graphics::ConstantFieldType::Float4:
        return sizeof(float)*4;
    case Core::Graphics::ConstantFieldType::Float3x3:
        return sizeof(float)*3*3;
    case Core::Graphics::ConstantFieldType::Float4x3:
        return sizeof(float)*4*3;
    case Core::Graphics::ConstantFieldType::Float3x4:
        return sizeof(float)*4*3;
    case Core::Graphics::ConstantFieldType::Float4x4:
        return sizeof(float)*4*4;
    case Core::Graphics::ConstantFieldType::Unknown:
        break;
    }
    AssertNotImplemented();
    return 0;
}
//----------------------------------------------------------------------------
StringSlice ConstantFieldTypeToCStr(ConstantFieldType value) {
    switch (value)
    {
    case Core::Graphics::ConstantFieldType::Bool:
        return MakeStringSlice("Bool");
    case Core::Graphics::ConstantFieldType::Int:
        return MakeStringSlice("Int");
    case Core::Graphics::ConstantFieldType::Int2:
        return MakeStringSlice("Int2");
    case Core::Graphics::ConstantFieldType::Int3:
        return MakeStringSlice("Int3");
    case Core::Graphics::ConstantFieldType::Int4:
        return MakeStringSlice("Int4");
    case Core::Graphics::ConstantFieldType::UInt:
        return MakeStringSlice("UInt");
    case Core::Graphics::ConstantFieldType::UInt2:
        return MakeStringSlice("UInt2");
    case Core::Graphics::ConstantFieldType::UInt3:
        return MakeStringSlice("UInt3");
    case Core::Graphics::ConstantFieldType::UInt4:
        return MakeStringSlice("UInt4");
    case Core::Graphics::ConstantFieldType::Float:
        return MakeStringSlice("Float");
    case Core::Graphics::ConstantFieldType::Float2:
        return MakeStringSlice("Float2");
    case Core::Graphics::ConstantFieldType::Float3:
        return MakeStringSlice("Float3");
    case Core::Graphics::ConstantFieldType::Float4:
        return MakeStringSlice("Float4");
    case Core::Graphics::ConstantFieldType::Float3x3:
        return MakeStringSlice("Float3x3");
    case Core::Graphics::ConstantFieldType::Float4x3:
        return MakeStringSlice("Float4x3");
    case Core::Graphics::ConstantFieldType::Float3x4:
        return MakeStringSlice("Float4x3");
    case Core::Graphics::ConstantFieldType::Float4x4:
        return MakeStringSlice("Float4x4");
    case Core::Graphics::ConstantFieldType::Unknown:
        return MakeStringSlice("Unknown");
    }
    AssertNotImplemented();
    return StringSlice();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
