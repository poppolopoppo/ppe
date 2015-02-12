#include "stdafx.h"

#include "DX11ConstantWriter.h"

#include "DirectX11/DX11DeviceEncapsulator.h"

#include "Device/DeviceAPIEncapsulator.h"
#include "Device/Shader/ConstantBufferLayout.h"
#include "Device/Shader/ConstantField.h"

#include "Core/Maths/Geometry/ScalarVector.h"
#include "Core/Maths/Transform/ScalarMatrix.h"

namespace Core {
namespace Graphics {
namespace DX11 {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool ExportFieldData_(void *dst, const void *src, ConstantFieldType type) {
    switch (type)
    {
    // int & int vectors
    case Core::Graphics::ConstantFieldType::Int:
        *reinterpret_cast<i32 *>(dst) = *reinterpret_cast<const i32 *>(src);
        break;
    case Core::Graphics::ConstantFieldType::Int2:
        *reinterpret_cast<i322 *>(dst) = *reinterpret_cast<const i322 *>(src);
        break;
    case Core::Graphics::ConstantFieldType::Int3:
        *reinterpret_cast<i323 *>(dst) = *reinterpret_cast<const i323 *>(src);
        break;
    case Core::Graphics::ConstantFieldType::Int4:
        *reinterpret_cast<i324 *>(dst) = *reinterpret_cast<const i324 *>(src);
        break;

    // unsigned & unsigned vectors
    case Core::Graphics::ConstantFieldType::UInt:
        *reinterpret_cast<u32 *>(dst) = *reinterpret_cast<const u32 *>(src);
        break;
    case Core::Graphics::ConstantFieldType::UInt2:
        *reinterpret_cast<u322 *>(dst) = *reinterpret_cast<const u322 *>(src);
        break;
    case Core::Graphics::ConstantFieldType::UInt3:
        *reinterpret_cast<u323 *>(dst) = *reinterpret_cast<const u323 *>(src);
        break;
    case Core::Graphics::ConstantFieldType::UInt4:
        *reinterpret_cast<u324 *>(dst) = *reinterpret_cast<const u324 *>(src);
        break;

    // float & float vectors
    case Core::Graphics::ConstantFieldType::Float:
        *reinterpret_cast<float *>(dst) = *reinterpret_cast<const float *>(src);
        break;
    case Core::Graphics::ConstantFieldType::Float2:
        *reinterpret_cast<float2 *>(dst) = *reinterpret_cast<const float2 *>(src);
        break;
    case Core::Graphics::ConstantFieldType::Float3:
        *reinterpret_cast<float3 *>(dst) = *reinterpret_cast<const float3 *>(src);
        break;
    case Core::Graphics::ConstantFieldType::Float4:
        *reinterpret_cast<float4 *>(dst) = *reinterpret_cast<const float4 *>(src);
        break;

    // float matrices (need to be transposed for DX11)
    case Core::Graphics::ConstantFieldType::Float3x3:
        *reinterpret_cast<float3x3 *>(dst) = reinterpret_cast<const float3x3 *>(src)->Transpose();
        break;
    case Core::Graphics::ConstantFieldType::Float4x3:
        *reinterpret_cast<float3x4 *>(dst) = reinterpret_cast<const float4x3 *>(src)->Transpose();
        break;
    case Core::Graphics::ConstantFieldType::Float4x4:
        *reinterpret_cast<float4x4 *>(dst) = reinterpret_cast<const float4x4 *>(src)->Transpose();
        break;

    default:
        return false;
    }

    return true;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ConstantWriter::ConstantWriter(IDeviceAPIEncapsulator *device)
:   DeviceAPIDependantConstantWriter(device) {}
//----------------------------------------------------------------------------
ConstantWriter::~ConstantWriter() {}
//----------------------------------------------------------------------------
void ConstantWriter::SetData(
    IDeviceAPIEncapsulator *device,
    const ConstantBuffer *resource,
    const MemoryView<const u8>& rawData,
    const MemoryView<u8>& output) const {
    Assert(device);
    Assert(resource);

    const MemoryView<const ConstantField> fields = resource->Layout()->Fields();
    Assert(fields.size());
    Assert(rawData.size() == resource->Layout()->SizeInBytes());

    const size_t n = fields.size();
    for (size_t i = 0; i < n; ++i) {
        const ConstantField& field = fields[i];

        const size_t offset = field.Offset();

        const void *src = &rawData.at(offset);
        void *const dst = &output.at(offset);

        if (!ExportFieldData_(dst, src, field.Type())) {
            AssertNotImplemented();
            throw DeviceEncapsulatorException("DX11: failed to write constant buffer field", device, resource);
        }
    }
}
//----------------------------------------------------------------------------
void ConstantWriter::SetData(
    IDeviceAPIEncapsulator *device,
    const ConstantBuffer *resource,
    const MemoryView<const void *>& fieldsData,
    const MemoryView<u8>& output) const {
    Assert(device);
    Assert(resource);

    const MemoryView<const ConstantField> fields = resource->Layout()->Fields();
    Assert(fields.size());
    Assert(fields.size() == fieldsData.size());

    const size_t n = fields.size();
    for (size_t i = 0; i < n; ++i) {
        const ConstantField& field = fields[i];

        const void *src = fieldsData[i];
        void *const dst = &output.at(field.Offset());

        if (!ExportFieldData_(dst, src, field.Type())) {
            AssertNotImplemented();
            throw DeviceEncapsulatorException("DX11: failed to write constant buffer field", device, resource);
        }
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
