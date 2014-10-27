#include "stdafx.h"

#include "DX11ResourceBuffer.h"

#include "DX11DeviceEncapsulator.h"

#include "Device/DeviceAPIEncapsulator.h"
#include "Device/DeviceEncapsulatorException.h"
#include "Device/DeviceResource.h"
#include "Device/PresentationParameters.h"
#include "Device/Texture/SurfaceFormat.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Graphics {
namespace DX11 {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ResourceBuffer::ResourceBuffer( IDeviceAPIEncapsulator *device,
                                Graphics::DeviceResourceBuffer *owner,
                                const DeviceResource *resource,
                                const MemoryView<const u8>& optionalData)
:   DeviceAPIDependantResourceBuffer(device, owner, resource, optionalData) {
    const DeviceWrapper *wrapper = DX11DeviceWrapper(device);

    ::D3D11_BUFFER_DESC bd;
    ::SecureZeroMemory(&bd, sizeof(bd));
    bd.ByteWidth = checked_cast<UINT>(owner->SizeInBytes());
    bd.StructureByteStride = checked_cast<UINT>(owner->Stride());
    bd.BindFlags = DeviceResourceTypeToDX11BindFlag(resource->ResourceType());
    bd.Usage = BufferUsageToDX11Usage(owner->Usage());
    bd.CPUAccessFlags = BufferModeToDX11CPUAccessFlags(owner->Mode());
    bd.MiscFlags = 0;

    if (optionalData.empty()) {
        Assert(owner->Usage() != BufferUsage::Immutable);

        DX11_THROW_IF_FAILED(device, resource, (
            wrapper->Device()->CreateBuffer(&bd, NULL, _entity.GetAddressOf())
            ));
    }
    else {
        AssertRelease(bd.ByteWidth == optionalData.SizeInBytes());
        Assert(IS_ALIGNED(16, optionalData.Pointer()));

        ::D3D11_SUBRESOURCE_DATA initData;
        ::SecureZeroMemory(&initData, sizeof(initData));
        initData.pSysMem = optionalData.Pointer();

        DX11_THROW_IF_FAILED(device, resource, (
            wrapper->Device()->CreateBuffer(&bd, &initData, _entity.GetAddressOf())
            ));
    }

    Assert(_entity);
    DX11SetDeviceResourceNameIFP(_entity, resource);
}
//----------------------------------------------------------------------------
ResourceBuffer::~ResourceBuffer() {
    ReleaseComRef(_entity);
}
//----------------------------------------------------------------------------
void ResourceBuffer::GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) {
    AssertRelease(Owner()->Usage() == BufferUsage::Staging);

    const DeviceWrapper *wrapper = DX11DeviceWrapper(device);

    if (!DX11MapRead(wrapper->ImmediateContext(), _entity.Get(), offset, dst, stride, count))
        throw DeviceEncapsulatorException("DX11: failed to map resource buffer for reading", device);
}
//----------------------------------------------------------------------------
void ResourceBuffer::SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) {
    Assert(IS_ALIGNED(16, src));
    Assert(_entity);

    const BufferUsage usage = Owner()->Usage();
    const DeviceWrapper *wrapper = DX11DeviceWrapper(device);

    switch (usage)
    {
    case Core::Graphics::BufferUsage::Default:
        AssertRelease(stride * count == Owner()->SizeInBytes()); // <=> 0 == offset
        if (!DX11UpdateResource(wrapper->ImmediateContext(), _entity.Get(), src, stride, count))
            throw DeviceEncapsulatorException("DX11: failed to update resource buffer", device);
        break;

    case Core::Graphics::BufferUsage::Dynamic:
    case Core::Graphics::BufferUsage::Staging:
        if (!DX11MapWrite(wrapper->ImmediateContext(), _entity.Get(), offset, src, stride, count, BufferUsage::Dynamic == usage))
            throw DeviceEncapsulatorException("DX11: failed to map resource buffer for writing", device);
        break;

    case Core::Graphics::BufferUsage::Immutable:
        throw DeviceEncapsulatorException("DX11: immutable buffer can't be muted", device, Resource());

    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(ResourceBuffer, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool DX11UpdateResource(ID3D11DeviceContext *deviceContext, ::ID3D11Resource *resource, const void *src, size_t stride, size_t count) {
    Assert(deviceContext);
    Assert(resource);
    Assert(IS_ALIGNED(16, src));

    deviceContext->UpdateSubresource(resource, 0, NULL, src, 0, 0);
    return true;
}
//----------------------------------------------------------------------------
bool DX11MapRead(ID3D11DeviceContext *deviceContext, ::ID3D11Resource *resource, size_t offset, void *const dst, size_t stride, size_t count) {
    Assert(deviceContext);
    Assert(resource);
    Assert(IS_ALIGNED(16, dst));

    ::D3D11_MAPPED_SUBRESOURCE mappedResource;
    const ::HRESULT result = deviceContext->Map(resource, 0, D3D11_MAP_READ, 0, &mappedResource);
    if (FAILED(result))
        return false;

    memcpy(dst, reinterpret_cast<const u8 *>(mappedResource.pData) + offset, stride * count);

    deviceContext->Unmap(resource, 0);
    return true;
}
//----------------------------------------------------------------------------
bool DX11MapWrite(ID3D11DeviceContext *deviceContext, ::ID3D11Resource *resource, size_t offset, const void *src, size_t stride, size_t count, bool discard) {
    Assert(deviceContext);
    Assert(resource);
    Assert(IS_ALIGNED(16, src));

    const ::D3D11_MAP mapType = (discard)
        ? ::D3D11_MAP_WRITE_DISCARD
        : ::D3D11_MAP_WRITE;

    ::D3D11_MAPPED_SUBRESOURCE mappedResource;
    const ::HRESULT result = deviceContext->Map(resource, 0, mapType, 0, &mappedResource);
    if (FAILED(result))
        return false;

    memcpy(reinterpret_cast<u8 *>(mappedResource.pData) + offset, src, stride * count);

    deviceContext->Unmap(resource, 0);
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_CPU_ACCESS_FLAG BufferModeToDX11CPUAccessFlags(Graphics::BufferMode value) {
    switch (value)
    {
    case Core::Graphics::BufferMode::None:
        return static_cast<D3D11_CPU_ACCESS_FLAG>(0);
    case Core::Graphics::BufferMode::Read:
        return D3D11_CPU_ACCESS_READ;
    case Core::Graphics::BufferMode::Write:
        return D3D11_CPU_ACCESS_WRITE;
    case Core::Graphics::BufferMode::ReadWrite:
        return static_cast<D3D11_CPU_ACCESS_FLAG>(D3D11_CPU_ACCESS_READ|D3D11_CPU_ACCESS_WRITE);
    }
    AssertNotImplemented();
    return static_cast<D3D11_CPU_ACCESS_FLAG>(-1);
}
//----------------------------------------------------------------------------
Graphics::BufferMode DX11CPUAccessFlagsToBufferMode(D3D11_CPU_ACCESS_FLAG value) {
    switch (size_t(value))
    {
    case D3D11_CPU_ACCESS_FLAG(0):
        return Core::Graphics::BufferMode::None;
    case D3D11_CPU_ACCESS_READ:
        return Core::Graphics::BufferMode::Read;
    case D3D11_CPU_ACCESS_WRITE:
        return Core::Graphics::BufferMode::Write;
    case D3D11_CPU_ACCESS_READ|D3D11_CPU_ACCESS_WRITE:
        return Core::Graphics::BufferMode::ReadWrite;
    }
    AssertNotImplemented();
    return static_cast<Graphics::BufferMode>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_USAGE BufferUsageToDX11Usage(Graphics::BufferUsage value) {
    switch (value)
    {
    case Core::Graphics::BufferUsage::Default:
        return D3D11_USAGE_DEFAULT;
    case Core::Graphics::BufferUsage::Immutable:
        return D3D11_USAGE_IMMUTABLE;
    case Core::Graphics::BufferUsage::Dynamic:
        return D3D11_USAGE_DYNAMIC;
    case Core::Graphics::BufferUsage::Staging:
        return D3D11_USAGE_STAGING;
    }
    AssertNotImplemented();
    return static_cast<D3D11_USAGE>(-1);
}
//----------------------------------------------------------------------------
Graphics::BufferUsage DX11UsageToBufferUsage(D3D11_USAGE value) {
    switch (value)
    {
    case D3D11_USAGE_DEFAULT:
        return Core::Graphics::BufferUsage::Default;
    case D3D11_USAGE_IMMUTABLE:
        return Core::Graphics::BufferUsage::Immutable;
    case D3D11_USAGE_DYNAMIC:
        return Core::Graphics::BufferUsage::Dynamic;
    case D3D11_USAGE_STAGING:
        return Core::Graphics::BufferUsage::Staging;
    }
    AssertNotImplemented();
    return static_cast<Graphics::BufferUsage>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_BIND_FLAG DeviceResourceTypeToDX11BindFlag(Graphics::DeviceResourceType value) {
    switch (value)
    {
    case Core::Graphics::DeviceResourceType::Constants:
        return D3D11_BIND_CONSTANT_BUFFER;
    case Core::Graphics::DeviceResourceType::Indices:
        return D3D11_BIND_INDEX_BUFFER;
    case Core::Graphics::DeviceResourceType::RenderTarget:
        return D3D11_BIND_RENDER_TARGET;
    case Core::Graphics::DeviceResourceType::ShaderProgram:
        return D3D11_BIND_SHADER_RESOURCE;
    case Core::Graphics::DeviceResourceType::Vertices:
        return D3D11_BIND_VERTEX_BUFFER;

    case Core::Graphics::DeviceResourceType::State:
    case Core::Graphics::DeviceResourceType::Texture:
    case Core::Graphics::DeviceResourceType::VertexDeclaration:
    default:
        AssertNotImplemented();
        throw DeviceEncapsulatorException("DX11: unkwown resource type", nullptr);
    }
}
//----------------------------------------------------------------------------
Graphics::DeviceResourceType DX11BindFlagToDeviceResourceType(D3D11_BIND_FLAG value) {
    switch (value)
    {
    case D3D11_BIND_CONSTANT_BUFFER:
        return Core::Graphics::DeviceResourceType::Constants;
    case D3D11_BIND_INDEX_BUFFER:
        return Core::Graphics::DeviceResourceType::Indices;
    case D3D11_BIND_RENDER_TARGET:
        return Core::Graphics::DeviceResourceType::RenderTarget;
    case D3D11_BIND_SHADER_RESOURCE:
        return Core::Graphics::DeviceResourceType::ShaderProgram;
    case D3D11_BIND_VERTEX_BUFFER:
        return Core::Graphics::DeviceResourceType::Vertices;
    default:
        AssertNotImplemented();
    }
    return static_cast<Graphics::DeviceResourceType>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
