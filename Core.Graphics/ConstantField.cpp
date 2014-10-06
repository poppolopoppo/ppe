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
    case Core::Graphics::ConstantFieldType::Float4x4:
        return sizeof(float)*4*4;
    }
    AssertNotImplemented();
    return 0;
}
//----------------------------------------------------------------------------
const char *ConstantFieldTypeToCStr(ConstantFieldType value) {
    switch (value)
    {
    case Core::Graphics::ConstantFieldType::Int:
        return "Int";
    case Core::Graphics::ConstantFieldType::Int2:
        return "Int2";
    case Core::Graphics::ConstantFieldType::Int3:
        return "Int3";
    case Core::Graphics::ConstantFieldType::Int4:
        return "Int4";
    case Core::Graphics::ConstantFieldType::UInt:
        return "UInt";
    case Core::Graphics::ConstantFieldType::UInt2:
        return "UInt2";
    case Core::Graphics::ConstantFieldType::UInt3:
        return "UInt3";
    case Core::Graphics::ConstantFieldType::UInt4:
        return "UInt4";
    case Core::Graphics::ConstantFieldType::Float:
        return "Float";
    case Core::Graphics::ConstantFieldType::Float2:
        return "Float2";
    case Core::Graphics::ConstantFieldType::Float3:
        return "Float3";
    case Core::Graphics::ConstantFieldType::Float4:
        return "Float4";
    case Core::Graphics::ConstantFieldType::Float3x3:
        return "Float3x3";
    case Core::Graphics::ConstantFieldType::Float4x3:
        return "Float4x3";
    case Core::Graphics::ConstantFieldType::Float4x4:
        return "Float4x4";
    }
    AssertNotImplemented();
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
