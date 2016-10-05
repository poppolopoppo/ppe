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
static bool ExportFieldData_(void *dst, const void *src, EValueType type) {
    switch (type)
    {
    // bool
    case EValueType::Bool:
        *reinterpret_cast<bool *>(dst) = *reinterpret_cast<const bool *>(src);
        break;

    // int & int vectors
    case EValueType::Word:
        *reinterpret_cast<i32 *>(dst) = *reinterpret_cast<const i32 *>(src);
        break;
    case EValueType::Word2:
        *reinterpret_cast<i322 *>(dst) = *reinterpret_cast<const i322 *>(src);
        break;
    case EValueType::Word3:
        *reinterpret_cast<i323 *>(dst) = *reinterpret_cast<const i323 *>(src);
        break;
    case EValueType::Word4:
        *reinterpret_cast<i324 *>(dst) = *reinterpret_cast<const i324 *>(src);
        break;

    // unsigned & unsigned vectors
    case EValueType::UWord:
        *reinterpret_cast<u32 *>(dst) = *reinterpret_cast<const u32 *>(src);
        break;
    case EValueType::UWord2:
        *reinterpret_cast<u322 *>(dst) = *reinterpret_cast<const u322 *>(src);
        break;
    case EValueType::UWord3:
        *reinterpret_cast<u323 *>(dst) = *reinterpret_cast<const u323 *>(src);
        break;
    case EValueType::UWord4:
        *reinterpret_cast<u324 *>(dst) = *reinterpret_cast<const u324 *>(src);
        break;

    // float & float vectors
    case EValueType::Float:
        *reinterpret_cast<float *>(dst) = *reinterpret_cast<const float *>(src);
        break;
    case EValueType::Float2:
        *reinterpret_cast<float2 *>(dst) = *reinterpret_cast<const float2 *>(src);
        break;
    case EValueType::Float3:
        *reinterpret_cast<float3 *>(dst) = *reinterpret_cast<const float3 *>(src);
        break;
    case EValueType::Float4:
        *reinterpret_cast<float4 *>(dst) = *reinterpret_cast<const float4 *>(src);
        break;

    // float matrices (need to be transposed for DX11)
    case EValueType::Float3x3:
        *reinterpret_cast<float3x3 *>(dst) = reinterpret_cast<const float3x3 *>(src)->Transpose();
        break;
    case EValueType::Float4x3:
        *reinterpret_cast<float3x4 *>(dst) = reinterpret_cast<const float4x3 *>(src)->Transpose();
        break;
    case EValueType::Float4x4:
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
FDX11ConstantWriter::FDX11ConstantWriter(IDeviceAPIEncapsulator *device)
:   FDeviceAPIDependantConstantWriter(device) {}
//----------------------------------------------------------------------------
FDX11ConstantWriter::~FDX11ConstantWriter() {}
//----------------------------------------------------------------------------
void FDX11ConstantWriter::SetData(
    IDeviceAPIEncapsulator *device,
    const FConstantBuffer *resource,
    const TMemoryView<const u8>& rawData,
    const TMemoryView<u8>& output) const {
    Assert(device);
    Assert(resource);
    Assert(resource->Layout());
    Assert(rawData.size() == resource->Layout()->SizeInBytes());

    for (const FValueBlock::TField& field : resource->Layout()->Fields()) {
        const size_t offset = field.Offset();

        const void *src = rawData.CutStartingAt(offset).data();
        void *const dst = output.CutStartingAt(offset).data();

        if (!ExportFieldData_(dst, src, field.Type())) {
            AssertNotImplemented();
            CORE_THROW_IT(FDeviceEncapsulatorException("DX11: failed to write constant buffer field", device, resource));
        }
    }
}
//----------------------------------------------------------------------------
void FDX11ConstantWriter::SetData(
    IDeviceAPIEncapsulator *device,
    const FConstantBuffer *resource,
    const TMemoryView<const void *>& fieldsData,
    const TMemoryView<u8>& output) const {
    Assert(device);
    Assert(resource);
    Assert(resource->Layout());

    const TMemoryView<const FValueBlock::TField> fields = resource->Layout()->Fields();
    Assert(fields.size());
    Assert(fields.size() == fieldsData.size());

    const size_t n = fields.size();
    for (size_t i = 0; i < n; ++i) {
        const FValueBlock::TField& field = fields[i];

        const void *src = fieldsData[i];
        void *const dst = &output.at(field.Offset());

        if (!ExportFieldData_(dst, src, field.Type())) {
            AssertNotImplemented();
            CORE_THROW_IT(FDeviceEncapsulatorException("DX11: failed to write constant buffer field", device, resource));
        }
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
