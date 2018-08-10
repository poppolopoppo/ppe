#include "stdafx.h"

#include "DX11ResourceBuffer.h"

#include "DX11DeviceAPIEncapsulator.h"
#include "DX11ResourceHelpers.h"

#include "Device/DeviceAPI.h"
#include "Device/DeviceEncapsulatorException.h"
#include "Device/DeviceResource.h"
#include "Device/PresentationParameters.h"
#include "Device/Texture/SurfaceFormat.h"

#include "Allocator/PoolAllocator-impl.h"
#include "Maths/ScalarVector.h"
#include "Maths/ScalarRectangle.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDX11ResourceBuffer::FDX11ResourceBuffer(
    IDeviceAPIEncapsulator *device,
    const FDeviceResource *resource,
    const FDeviceResourceBuffer *buffer,
    const TMemoryView<const u8>& optionalData)
:   FDeviceAPIDependantResourceBuffer(device, resource, buffer, optionalData) {
    const FDX11DeviceWrapper *wrapper = DX11GetDeviceWrapper(device);

    ::D3D11_BUFFER_DESC bd;
    ::SecureZeroMemory(&bd, sizeof(bd));
    bd.ByteWidth = checked_cast<UINT>(SizeInBytes());
    bd.StructureByteStride = checked_cast<UINT>(Stride());
    bd.BindFlags = DeviceResourceTypeToDX11BindFlag(resource->ResourceType());
    bd.Usage = BufferUsageToDX11Usage(Usage());
    bd.CPUAccessFlags = BufferModeToDX11CPUAccessFlags(Mode());
    bd.MiscFlags = 0;

    if (optionalData.empty()) {
        AssertRelease(Usage() != EBufferUsage::Immutable);

        DX11_THROW_IF_FAILED(device, resource, (
            wrapper->Device()->CreateBuffer(&bd, NULL, _entity.GetAddressOf())
            ));
    }
    else {
        AssertRelease(bd.ByteWidth == optionalData.SizeInBytes());
        Assert(Meta::IsAligned(GRAPHICS_BOUNDARY, optionalData.Pointer()));

        ::D3D11_SUBRESOURCE_DATA initData;
        ::SecureZeroMemory(&initData, sizeof(initData));
        initData.pSysMem = optionalData.Pointer();

        DX11_THROW_IF_FAILED(device, resource, (
            wrapper->Device()->CreateBuffer(&bd, &initData, _entity.GetAddressOf())
            ));
    }

    Assert(_entity);
    DX11SetDeviceResourceNameIFP(_entity.Get(), resource);
}
//----------------------------------------------------------------------------
FDX11ResourceBuffer::~FDX11ResourceBuffer() {
    ReleaseComRef(_entity);
}
//----------------------------------------------------------------------------
void FDX11ResourceBuffer::GetData(IDeviceAPIEncapsulator *device, size_t offset, const TMemoryView<u8>& dst) {
    AssertRelease(Usage() == EBufferUsage::Staging);

    DX11ResourceGetData(device, _entity.Get(), 0, offset, dst, Mode(), Usage());
}
//----------------------------------------------------------------------------
void FDX11ResourceBuffer::SetData(IDeviceAPIEncapsulator *device, size_t offset, const TMemoryView<const u8>& src) {

    DX11ResourceSetData(device, _entity.Get(), 0, offset, src, Mode(), Usage());
}
//----------------------------------------------------------------------------
void FDX11ResourceBuffer::CopyFrom(IDeviceAPIEncapsulator *device, const FDeviceAPIDependantResourceBuffer *psource) {

    const FDX11ResourceBuffer *psourceDX11 = checked_cast<const FDX11ResourceBuffer *>(psource);

    DX11CopyResource(device, _entity.Get(), psourceDX11->Entity());
}
//----------------------------------------------------------------------------
void FDX11ResourceBuffer::CopySubPart(
    IDeviceAPIEncapsulator *device, size_t dstOffset,
    const FDeviceAPIDependantResourceBuffer *psource, size_t srcOffset, size_t length ) {

    const FDX11ResourceBuffer *psourceDX11 = checked_cast<const FDX11ResourceBuffer *>(psource);

    const uint3 dstPos(checked_cast<u32>(dstOffset), 0, 0);
    const FAabb3u srcBox(uint3(checked_cast<u32>(srcOffset), 0, 0),
                        uint3(checked_cast<u32>(srcOffset + length), 0, 1) );

    DX11CopyResourceSubRegion(device, _entity.Get(), 0, dstPos, psourceDX11->Entity(), 0, srcBox);
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Graphics, FDX11ResourceBuffer, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_CPU_ACCESS_FLAG BufferModeToDX11CPUAccessFlags(EBufferMode value) {
    switch (value)
    {
    case PPE::Graphics::EBufferMode::None:
        return static_cast<D3D11_CPU_ACCESS_FLAG>(0);
    case PPE::Graphics::EBufferMode::Read:
        return D3D11_CPU_ACCESS_READ;
    case PPE::Graphics::EBufferMode::Write:
    case PPE::Graphics::EBufferMode::WriteDiscard:
    case PPE::Graphics::EBufferMode::WriteDoNotWait:
        return D3D11_CPU_ACCESS_WRITE;
    case PPE::Graphics::EBufferMode::ReadWrite:
        return static_cast<D3D11_CPU_ACCESS_FLAG>(D3D11_CPU_ACCESS_READ|D3D11_CPU_ACCESS_WRITE);
    default:
        break;
    }
    AssertNotImplemented();
    return static_cast<D3D11_CPU_ACCESS_FLAG>(-1);
}
//----------------------------------------------------------------------------
EBufferMode DX11CPUAccessFlagsToBufferMode(D3D11_CPU_ACCESS_FLAG value) {
    switch (size_t(value))
    {
    case D3D11_CPU_ACCESS_FLAG(0):
        return PPE::Graphics::EBufferMode::None;
    case D3D11_CPU_ACCESS_READ:
        return PPE::Graphics::EBufferMode::Read;
    case D3D11_CPU_ACCESS_WRITE:
        return PPE::Graphics::EBufferMode::Write;
    case D3D11_CPU_ACCESS_READ|D3D11_CPU_ACCESS_WRITE:
        return PPE::Graphics::EBufferMode::ReadWrite;
    }
    AssertNotImplemented();
    return static_cast<Graphics::EBufferMode>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
::D3D11_USAGE BufferUsageToDX11Usage(EBufferUsage value) {
    switch (value)
    {
    case PPE::Graphics::EBufferUsage::Default:
        return D3D11_USAGE_DEFAULT;
    case PPE::Graphics::EBufferUsage::Immutable:
        return D3D11_USAGE_IMMUTABLE;
    case PPE::Graphics::EBufferUsage::Dynamic:
        return D3D11_USAGE_DYNAMIC;
    case PPE::Graphics::EBufferUsage::Staging:
        return D3D11_USAGE_STAGING;
    }
    AssertNotImplemented();
    return static_cast<::D3D11_USAGE>(-1);
}
//----------------------------------------------------------------------------
EBufferUsage DX11UsageToBufferUsage(D3D11_USAGE value) {
    switch (value)
    {
    case D3D11_USAGE_DEFAULT:
        return PPE::Graphics::EBufferUsage::Default;
    case D3D11_USAGE_IMMUTABLE:
        return PPE::Graphics::EBufferUsage::Immutable;
    case D3D11_USAGE_DYNAMIC:
        return PPE::Graphics::EBufferUsage::Dynamic;
    case D3D11_USAGE_STAGING:
        return PPE::Graphics::EBufferUsage::Staging;
    }
    AssertNotImplemented();
    return static_cast<Graphics::EBufferUsage>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
::D3D11_BIND_FLAG DeviceResourceTypeToDX11BindFlag(EDeviceResourceType value) {
    switch (value)
    {
    case PPE::Graphics::EDeviceResourceType::Constants:
        return D3D11_BIND_CONSTANT_BUFFER;
    case PPE::Graphics::EDeviceResourceType::Indices:
        return D3D11_BIND_INDEX_BUFFER;
    case PPE::Graphics::EDeviceResourceType::FRenderTarget:
        return D3D11_BIND_RENDER_TARGET;
    case PPE::Graphics::EDeviceResourceType::FShaderProgram:
        return D3D11_BIND_SHADER_RESOURCE;
    case PPE::Graphics::EDeviceResourceType::Vertices:
        return D3D11_BIND_VERTEX_BUFFER;

    default:
        AssertNotImplemented();
        PPE_THROW_IT(FDeviceEncapsulatorException("DX11: unsupported resource type", nullptr));
    }
}
//----------------------------------------------------------------------------
EDeviceResourceType DX11BindFlagToDeviceResourceType(::D3D11_BIND_FLAG value) {
    switch (value)
    {
    case D3D11_BIND_CONSTANT_BUFFER:
        return PPE::Graphics::EDeviceResourceType::Constants;
    case D3D11_BIND_INDEX_BUFFER:
        return PPE::Graphics::EDeviceResourceType::Indices;
    case D3D11_BIND_RENDER_TARGET:
        return PPE::Graphics::EDeviceResourceType::FRenderTarget;
    case D3D11_BIND_SHADER_RESOURCE:
        return PPE::Graphics::EDeviceResourceType::FShaderProgram;
    case D3D11_BIND_VERTEX_BUFFER:
        return PPE::Graphics::EDeviceResourceType::Vertices;
    default:
        AssertNotImplemented();
    }
    return static_cast<Graphics::EDeviceResourceType>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
