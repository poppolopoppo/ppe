#include "stdafx.h"

#include "DX11ConstantWriter.h"

#include "DirectX11/DX11DeviceAPIEncapsulator.h"

#include "Device/DeviceAPI.h"
#include "Device/DeviceEncapsulatorException.h"
#include "Device/Shader/ConstantBufferLayout.h"

#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/ScalarMatrix.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool ExportFieldData_(void *dst, const void *src, ValueType type) {
    switch (type)
    {
    // bool
    case ValueType::Bool:
        *reinterpret_cast<bool *>(dst) = *reinterpret_cast<const bool *>(src);
        break;

    // int & int vectors
    case ValueType::Word:
        *reinterpret_cast<i32 *>(dst) = *reinterpret_cast<const i32 *>(src);
        break;
    case ValueType::Word2:
        *reinterpret_cast<i322 *>(dst) = *reinterpret_cast<const i322 *>(src);
        break;
    case ValueType::Word3:
        *reinterpret_cast<i323 *>(dst) = *reinterpret_cast<const i323 *>(src);
        break;
    case ValueType::Word4:
        *reinterpret_cast<i324 *>(dst) = *reinterpret_cast<const i324 *>(src);
        break;

    // unsigned & unsigned vectors
    case ValueType::UWord:
        *reinterpret_cast<u32 *>(dst) = *reinterpret_cast<const u32 *>(src);
        break;
    case ValueType::UWord2:
        *reinterpret_cast<u322 *>(dst) = *reinterpret_cast<const u322 *>(src);
        break;
    case ValueType::UWord3:
        *reinterpret_cast<u323 *>(dst) = *reinterpret_cast<const u323 *>(src);
        break;
    case ValueType::UWord4:
        *reinterpret_cast<u324 *>(dst) = *reinterpret_cast<const u324 *>(src);
        break;

    // float & float vectors
    case ValueType::Float:
        *reinterpret_cast<float *>(dst) = *reinterpret_cast<const float *>(src);
        break;
    case ValueType::Float2:
        *reinterpret_cast<float2 *>(dst) = *reinterpret_cast<const float2 *>(src);
        break;
    case ValueType::Float3:
        *reinterpret_cast<float3 *>(dst) = *reinterpret_cast<const float3 *>(src);
        break;
    case ValueType::Float4:
        *reinterpret_cast<float4 *>(dst) = *reinterpret_cast<const float4 *>(src);
        break;

    // float matrices (need to be transposed for DX11)
    case ValueType::Float3x3:
        *reinterpret_cast<float3x3 *>(dst) = reinterpret_cast<const float3x3 *>(src)->Transpose();
        break;
    case ValueType::Float4x3:
        *reinterpret_cast<float3x4 *>(dst) = reinterpret_cast<const float4x3 *>(src)->Transpose();
        break;
    case ValueType::Float4x4:
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
DX11ConstantWriter::DX11ConstantWriter(IDeviceAPIEncapsulator *device)
:   DeviceAPIDependantConstantWriter(device) {}
//----------------------------------------------------------------------------
DX11ConstantWriter::~DX11ConstantWriter() {}
//----------------------------------------------------------------------------
void DX11ConstantWriter::SetData(
    IDeviceAPIEncapsulator *device,
    const ConstantBuffer *resource,
    const MemoryView<const u8>& rawData,
    const MemoryView<u8>& output) const {
    Assert(device);
    Assert(resource);
    Assert(resource->Layout());
    Assert(rawData.size() == resource->Layout()->SizeInBytes());

    for (const ValueBlock::Field& field : resource->Layout()->Fields()) {
        const size_t offset = field.Offset();

        const void *src = rawData.CutStartingAt(offset).data();
        void *const dst = output.CutStartingAt(offset).data();

        if (!ExportFieldData_(dst, src, field.Type())) {
            AssertNotImplemented();
            throw DeviceEncapsulatorException("DX11: failed to write constant buffer field", device, resource);
        }
    }
}
//----------------------------------------------------------------------------
void DX11ConstantWriter::SetData(
    IDeviceAPIEncapsulator *device,
    const ConstantBuffer *resource,
    const MemoryView<const void *>& fieldsData,
    const MemoryView<u8>& output) const {
    Assert(device);
    Assert(resource);
    Assert(resource->Layout());

    const MemoryView<const ValueBlock::Field> fields = resource->Layout()->Fields();
    Assert(fields.size());
    Assert(fields.size() == fieldsData.size());

    const size_t n = fields.size();
    for (size_t i = 0; i < n; ++i) {
        const ValueBlock::Field& field = fields[i];

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
} //!namespace Graphics
} //!namespace Core
